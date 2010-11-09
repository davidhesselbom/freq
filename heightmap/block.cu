#include <stdio.h>
#include "heightmap/block.cu.h"

#include "resample.cu.h"


class ConverterPhase
{
public:
    __device__ float operator()( float2 v, uint2 const& /*dataPosition*/ )
    {
        return atan2(v.y, v.x);
    }
};

class WeightInterpolation
{
public:
    __device__ float operator()( float2 v, uint2 const& /*dataPosition*/ )
    {
        return atan2(v.y, v.x);
    }
};

void blockResampleChunk( cudaPitchedPtrType<float2> input,
                 cudaPitchedPtrType<float> output,
                 uint2 validInputs,
                 float4 inputRegion,
                 float4 outputRegion)
{
    elemSize3_t sz_input = input.getNumberOfElements();
    elemSize3_t sz_output = output.getNumberOfElements();

    uint4 validInputs4 = make_uint4( validInputs.x, 0, validInputs.y, sz_input.y );
    uint2 validOutputs = make_uint2( sz_output.x, sz_output.y );

    bool tittafas = false;
    if (!tittafas)
    {
        resample2d<float2, float, ConverterAmplitude, AssignOperator<float> >(
                input,
                output,
                validInputs4,
                validOutputs,
                inputRegion,
                outputRegion
        );
    } else {
        resample2d<float2, float, ConverterPhase, AssignOperator<float> >(
                    input,
                    output,
                    validInputs4,
                    validOutputs,
                    inputRegion,
                    outputRegion
            );
    }
}


__global__ void kernel_merge(
                cudaPitchedPtrType<float> inBlock,
                cudaPitchedPtrType<float> outBlock,
                float resample_width,
                float resample_height,
                float in_offset,
                float out_offset,
                float in_valid_samples)
{
    elemSize3_t writePos;
    if( !outBlock.unwrapCudaGrid( writePos ))
        return;

    float val = 0;
    unsigned n = 0;

    if (writePos.x>=out_offset)
    {
        for (float x = 0; x < resample_width; x++)
        {
            float s = in_offset + x + resample_width*(writePos.x-out_offset);
            if ( s >= in_offset + in_valid_samples + .25f*resample_width )
                x=resample_width;
            else for (float y = 0; y < resample_height; y++)
            {
                float t = y + resample_height*writePos.y;

                elemSize3_t readPos = make_elemSize3_t( s, t, 0 );
                if ( inBlock.valid(readPos) ) {
                    val += inBlock.elem(readPos);

                    //outBlock.e( writePos ) = val;
                    //return;

                    n ++;
                }
            }
        }
    }

    if (0<n) {
        val/=n;
        outBlock.elem( writePos ) = val;
    }
}

extern "C"
void blockMerge( cudaPitchedPtrType<float> inBlock,
                 cudaPitchedPtrType<float> outBlock,
                 float in_sample_rate,
                 float out_sample_rate,
                 float in_frequency_resolution,
                 float out_frequency_resolution,
                 float in_offset,
                 float out_offset,
                 float in_valid_samples,
                 unsigned cuda_stream)
{
    dim3 grid, block;
    unsigned block_size = 128;

    outBlock.wrapCudaGrid2D( block_size, grid, block );

    float resample_width = in_sample_rate/out_sample_rate;
    float resample_height = in_frequency_resolution/out_frequency_resolution;

    kernel_merge<<<grid, block, cuda_stream>>>(
        inBlock, outBlock,
        resample_width,
        resample_height,
        in_offset, out_offset, in_valid_samples );
}

texture<float2, 1, cudaReadModeElementType> chunkTexture;

/**
 kernel_merge_chunk has one thread for each output element
*/
__global__ void kernel_merge_chunk_old(
                cudaPitchedPtrType<float2> inChunk,
                cudaPitchedPtrType<float> outBlock,
                float resample_width,
                float resample_height,
                float in_offset,
                float out_offset,
                unsigned in_count )
{
    elemSize3_t writePos;
    if( !outBlock.unwrapCudaGrid( writePos ))
        return;

    float val = 0;
    float n = 0;

    if (writePos.x>=out_offset)
    {
        // TODO xs should depend on hz
        float ff = writePos.y/((float)outBlock.getNumberOfElements().y - 1);

        float xs = 2.f/(ff*ff);//resample_width/10;
        if (1>xs) xs=1;
        for (float x = 0; x < resample_width; x+=xs)
        {
            float s = in_offset + x + resample_width*(writePos.x-out_offset);

            if ( s > in_offset + in_count + .25f*resample_width)
                x=resample_width; // abort for x loop, faster than "break;"
            else for (float y = 0; y < resample_height; y++)
            {
                //float y = 0;
                float t = y + resample_height*writePos.y;

                elemSize3_t readPos = make_elemSize3_t( s, t, 0 );
                inChunk.clamp(readPos);
                if ( inChunk.valid(readPos) ) {
                    float ff = t/(float)inChunk.getNumberOfElements().y;
                    float if0 = 40.f/(2 + 35*ff*ff*ff);

                    //float2 c = inChunk.elem(readPos);
                    float2 c = tex1Dfetch(chunkTexture, inChunk.eOffs(readPos));
                    val = max(val, if0*sqrt(if0*(c.x*c.x + c.y*c.y)));

 //outBlock.e( writePos ) = 4*val;
 //return;
/*
  TODO use command line argument "yscale"
                        case Yscale_Linear:
                            v[2][df] = amplitude;
                            break;
                        case Yscale_ExpLinear:
                            v[2][df] = amplitude * exp(.001*fi);
                            break;
                        case Yscale_LogLinear:
                            v[2][df] = amplitude;
                            v[2][df] = log(1+fabsf(v[2][df]))*(v[2][df]>0?1:-1);
                            break;
                        case Yscale_LogExpLinear:
                            v[2][df] = amplitude * exp(.001*fi);
                            v[2][df] = log(1+fabsf(v[2][df]))*(v[2][df]>0?1:-1);
                            */

                    n++;
                }
            }
        }
    }
/*
    __syncthreads();
*/
    if (0<n) {
        //val/=n;
        outBlock.e( writePos ) = val;
    }
}

#define BLOCK_SIZE 64 // manally adjusted to increase performance

struct read_params {
    unsigned
        block_first_read,
        block_last_read,
        num_reads,
        thread_first_read,
        thread_last_read,
        start_y,
        end_y;
};

__device__ __host__ read_params computeReadParams( unsigned in_sample_offset, unsigned in_count, float out_sample_offset,
                                                   float in_frequency_offset, float out_frequency_offset,
                                                   float resample_height, float resample_width,
                                                   elemSize3_t writePos, bool firstThreadInBlock )
{
    read_params p;

    float threadFirstWrite = writePos.x;
    float threadLastWrite = writePos.x+1;

    // Here, BLOCK_SIZE is the number of elements that each block is responsible for writing
    float blockFirstWrite = writePos.x/BLOCK_SIZE*BLOCK_SIZE; // integer division
    float blockLastWrite = blockFirstWrite + BLOCK_SIZE;

    // Don't write anything before out_sample_offset, if the entire range is before out_sample_offset the range will be
    // [out_sample_offset, out_sample_offset) which is an empty set. However, [blockFirstWrite, blockLastWrite) might be
    // non-empty even though [myFirstWrite, myLastWrite) is empty.
    if (threadFirstWrite < out_sample_offset) threadFirstWrite = out_sample_offset;
    if (threadLastWrite  < out_sample_offset) threadLastWrite  = out_sample_offset;
    if (blockFirstWrite  < out_sample_offset) blockFirstWrite  = out_sample_offset;
    if (blockLastWrite   < out_sample_offset) blockLastWrite   = out_sample_offset;

    p.thread_first_read = in_sample_offset + (threadFirstWrite - out_sample_offset) * resample_width;
    p.thread_last_read  = in_sample_offset + (threadLastWrite  - out_sample_offset) * resample_width;

    if (p.thread_first_read < in_sample_offset)          p.thread_first_read = in_sample_offset;
    if (p.thread_last_read  > in_sample_offset+in_count) p.thread_last_read  = in_sample_offset+in_count;

    if (firstThreadInBlock)
    {
        p.block_first_read = in_sample_offset + (blockFirstWrite - out_sample_offset) * resample_width;
        p.block_last_read  = in_sample_offset + (blockLastWrite  - out_sample_offset) * resample_width;

        if (p.block_first_read < in_sample_offset)          p.block_first_read = in_sample_offset;
        if (p.block_last_read  > in_sample_offset+in_count) p.block_last_read  = in_sample_offset+in_count;

        // Here, BLOCK_SIZE is the number of elements that a block reads in each read chunk
        p.block_first_read =  p.block_first_read              /BLOCK_SIZE; // integer division
        p.block_last_read  = (p.block_last_read+ BLOCK_SIZE-1)/BLOCK_SIZE;

        if (p.block_first_read<p.block_last_read)
            p.num_reads = p.block_last_read-p.block_first_read;
        else
            p.num_reads = 0;

        p.block_first_read *= BLOCK_SIZE;
        p.block_last_read  *= BLOCK_SIZE;

        p.start_y = resample_height *  writePos.y    + in_frequency_offset;
        p.end_y   = resample_height * (writePos.y+1) + in_frequency_offset;
        if (p.end_y == p.start_y) p.end_y++;
    }


    return p;
}


/**
    kernel_merge_chunk2 has one thread for each output element.
    Threads collaborate in warps to serve eachother. Each block only have one warp.

    @param resample_width is the number of input elements that should be taken into
           account for the output element that each thread is to write.
*/
__global__ void kernel_merge_chunk2(
                cudaPitchedPtrType<float2> inChunk,
                cudaPitchedPtrType<float> outBlock,
                float resample_width,
                float resample_height,
                unsigned in_sample_offset,
                float out_sample_offset,
                float in_frequency_offset,
                float out_frequency_offset,
                unsigned in_count,
                Heightmap::ComplexInfo complexInfo )
{
/**
    Merging like this for resample_width = 3, resample_height=1, in_offset=1, out_offset=0
    (take max of each group of 3)
    chunk: 1|1234575652165761|9682657451433|321
    out:    |  5  7  5  7  9 | 8  7  5  4  | => |57579|8754|

    That is; discard element that begins before outBlock but don't discard fraction of element that end after outBlock.

    If an element would include more than BLOCK_SIZE, the last elements are discarded.
*/
    __shared__ float val[BLOCK_SIZE];

    elemSize3_t threadPos = make_elemSize3_t(
                            blockIdx.x* blockDim.x + threadIdx.x,
                            blockIdx.y* blockDim.y + threadIdx.y,
                            blockIdx.z* blockDim.z + threadIdx.z);

    read_params thread_p = computeReadParams(
            in_sample_offset, in_count, out_sample_offset,
            in_frequency_offset, out_frequency_offset,
            resample_height, resample_width,
            threadPos, 0==threadIdx.x );

    // TODO could use the out_frequency_offset as a float and adjust blocks
    elemSize3_t writePos = threadPos;
    writePos.y += out_frequency_offset;

    __shared__ read_params block_read;
    if (0==threadIdx.x)
        block_read = thread_p;

    float myVal = -1; // Negative value that indicates that no value was fetched

    __syncthreads();

    if (0 != block_read.num_reads)
        for (unsigned y = block_read.start_y;
             y < block_read.end_y && y < inChunk.getNumberOfElements().y;
             y++)
    {
        float ff = y/(float)inChunk.getNumberOfElements().y;
        float if0 = 20.f/(2 + 35*ff*ff*ff);
        if0=if0*if0*if0;
        if0 *= 0.4f;

        for (unsigned i=0; i<block_read.num_reads; i++)
        {
            unsigned base = block_read.block_first_read + i*BLOCK_SIZE;
            elemSize3_t readPos = make_elemSize3_t( base + threadIdx.x, y, 0 );

            bool valid = readPos.x >= in_sample_offset &&
                         readPos.x < in_sample_offset + in_count &&
                         inChunk.valid(readPos);

            // Read from global memory
            float2 c = valid ? inChunk.e(readPos) : make_float2(0,0);

            // TODO use templates instead of switch argument
            switch (complexInfo)
            {
            case Heightmap::ComplexInfo_Amplitude_Non_Weighted:
                val[threadIdx.x] = c.x*c.x + c.y*c.y;
                break;

            case Heightmap::ComplexInfo_Phase:
                val[threadIdx.x] = 0.1f*(((float)M_PI) + atan2(c.y, c.x))*(1.f/(2*(float)M_PI));
                break;

            default:
            // case Heightmap::ComplexInfo_Amplitude_Weighted
                val[threadIdx.x] = if0*(c.x*c.x + c.y*c.y);
                break;
            }

            __syncthreads();

            if (0) { // Each thread minds its own business
                if ( thread_p.thread_last_read  > base &&
                     thread_p.thread_first_read < thread_p.thread_last_read &&
                     thread_p.thread_first_read < base+BLOCK_SIZE )
                {
                    unsigned start = max(thread_p.thread_first_read, base) - base;
                    unsigned end = min(thread_p.thread_last_read, base + BLOCK_SIZE) - base;
    //                myVal = val[start];
                    for (unsigned x = start; x<end; x++)
                        myVal = max(myVal, val[x]);
                }
            } else {
                // Find the maxima in all of val and use that, half of the threads go to sleep at each step
                // This is not as exact as "Each thread minds its own business" but differences are rarely visible.
                if (128==BLOCK_SIZE) {
                    if (threadIdx.x<64) val[threadIdx.x] = max(val[threadIdx.x], val[64 + threadIdx.x]);
                    if (threadIdx.x<32) val[threadIdx.x] = max(val[threadIdx.x], val[32 + threadIdx.x]);
                }
                if (128==BLOCK_SIZE || 32==BLOCK_SIZE) {
                    if (threadIdx.x<16) val[threadIdx.x] = max(val[threadIdx.x], val[16 + threadIdx.x]);
                    if (threadIdx.x<8) val[threadIdx.x] = max(val[threadIdx.x], val[8 + threadIdx.x]);
                    if (threadIdx.x<4) val[threadIdx.x] = max(val[threadIdx.x], val[4 + threadIdx.x]);
                    if (threadIdx.x<2) val[threadIdx.x] = max(val[threadIdx.x], val[2 + threadIdx.x]);
                    if (threadIdx.x<1) val[0] = max(val[0], val[1]);
                }
                if (64==BLOCK_SIZE) {
                    if (threadIdx.x<16) {
                        val[threadIdx.x] = max(max(val[threadIdx.x], val[48 + threadIdx.x]),
                                               max(val[32 + threadIdx.x], val[16 + threadIdx.x]));

                        if (threadIdx.x<4) {
                            val[threadIdx.x] = max(max(val[threadIdx.x], val[4 + threadIdx.x]),
                                                   max(val[8 + threadIdx.x], val[12 + threadIdx.x]));

                            if (threadIdx.x<1) {
                                val[0] = max(max(val[0], val[1]), max(val[2], val[3]));
                    }}}
                }

                __syncthreads();

                if ( thread_p.thread_last_read  > base &&
                     thread_p.thread_first_read < thread_p.thread_last_read &&
                     thread_p.thread_first_read < base+BLOCK_SIZE )
                {
                    myVal = max(myVal, val[0]);
                }
            }
        }
    }

    if (outBlock.valid( writePos ) && 0<=myVal)
    {
        switch (complexInfo)
        {
        case Heightmap::ComplexInfo_Phase:
            // myVal contains phase value
            break;

        default:
        // case Heightmap::ComplexInfo_Amplitude_Non_Weighted:
        // case Heightmap::ComplexInfo_Amplitude_Weighted:
            myVal = sqrtf(myVal);
            break;
        }

        myVal *= .02f;
        //myVal = .1f;
        outBlock.elem( writePos ) = myVal;
    }
}


extern "C"
void blockMergeChunk( cudaPitchedPtrType<float2> inChunk,
                 cudaPitchedPtrType<float> outBlock,
                 float in_sample_rate,
                 float out_sample_rate,
                 float in_frequency_resolution,
                 float out_frequency_resolution,
                 unsigned in_sample_offset,
                 float out_sample_offset,
                 float in_frequency_offset,
                 float out_frequency_offset,
                 float out_count,
                 Heightmap::ComplexInfo complexInfo,
                 unsigned cuda_stream)
{
    unsigned block_size;

    const unsigned version = 2;
    switch (version)
    {   case 1: block_size = 128; break;
        case 2: block_size = BLOCK_SIZE; break;
    }

    // For version 2
    dim3 block(block_size,1,1);
    uint3 nElems = outBlock.getNumberOfElements();
    fprintf(stdout, "\noutBlock.getNumberOfElements() = (%u, %u, %u)", nElems.x, nElems.y, nElems.z );
    unsigned in_count = ceil(out_count*in_sample_rate/out_sample_rate);

    // Limit kernel size
    {
        nElems.y -= out_frequency_offset;

        unsigned last_write = ceil(out_sample_offset + out_count);
        nElems.x = min(nElems.x, last_write);
        unsigned unused = out_sample_offset/block_size;
        unused*=block_size;
        printf("\nout_count = %g", out_count);
        printf("\nblock_size = %u", block_size);
        printf("\nout_sample_offset = %g", out_sample_offset);
        printf("\nlast_write = %u", last_write);
        printf("\nunused = %u", unused);
        printf("\n1 Elems.x = %u", nElems.x);
        nElems.x -= unused;
        printf("\n2 Elems.x = %u", nElems.x);
        out_sample_offset -= unused;
        cudaPitchedPtr cpp = outBlock.getCudaPitchedPtr();
        cpp.ptr = &((float*)cpp.ptr)[unused];
        outBlock =  cudaPitchedPtrType<float>(cpp);
    }

    dim3 grid;

    switch (version)
    {   case 1: outBlock.wrapCudaGrid2D(block_size, grid, block); break;
        case 2: grid = make_uint3(
                    int_div_ceil(nElems.x, block.x),
                    int_div_ceil(nElems.y, block.y),
                    int_div_ceil(nElems.z, block.z));
            printf("\ngrid.x = %u", grid.x);
            printf("\nnElems.x = %u", nElems.x);
            printf("\nblock.x = %u", block.x);
            printf("\nint_div_ceil(nElems.x, block.x) = %lu", int_div_ceil(nElems.x, block.x));
            break;
    }

    float resample_width = in_sample_rate/out_sample_rate;
    // TODO motivate this '2'
    float resample_height = (in_frequency_resolution+2)/out_frequency_resolution;

    if (1) {
        elemSize3_t sz_o = outBlock.getNumberOfElements();
        elemSize3_t sz_i = inChunk.getNumberOfElements();
        //fprintf(stdout,"sz_o (%d, %d, %d)\tsz_i (%d, %d, %d)\n", sz_o.x, sz_o.y, sz_o.z, sz_i.x, sz_i.y, sz_i.z );

        fprintf(stdout, "\nint_div_ceil(128,2048) = %lu", int_div_ceil(128,2048));

        fprintf(stdout,"\ngrid (%d, %d, %d)\tblock (%d, %d, %d)\n", grid.x, grid.y, grid.z, block.x, block.y, block.z );
        fprintf(stdout,"(in sr %g, out sr %g, w=%g) (in f %g, out f %g, h=%g)\n\tin o %u, out o %g, in count = %u, out count %g\n",
            in_sample_rate, out_sample_rate, resample_width,
            in_frequency_resolution, out_frequency_resolution, resample_height,
            in_sample_offset, out_sample_offset, in_count, out_count);
        fprintf(stdout,"outBlock(%d,%d,%d) pitch %lu\n",
            outBlock.getNumberOfElements().x,
            outBlock.getNumberOfElements().y,
            outBlock.getNumberOfElements().z,
            outBlock.getCudaPitchedPtr().pitch );
        fprintf(stdout,"inChunk(%d,%d,%d) pitch %lu\n",
            inChunk.getNumberOfElements().x,
            inChunk.getNumberOfElements().y,
            inChunk.getNumberOfElements().z,
            inChunk.getCudaPitchedPtr().pitch );
        fflush(stdout);

    }

    if (0)
    {
        unsigned x_count = grid.x*block.x;
        printf("nElems.x = %u, x_count = %u\n", nElems.x, x_count);

        if(1) for (int x=0; x<x_count; x++)
        {
            elemSize3_t writePos = make_elemSize3_t(x,0,0);
            elemSize3_t myThreadIdx = make_elemSize3_t(0,0,0);

            read_params p = computeReadParams( in_sample_offset, in_count, out_sample_offset,
                                               in_frequency_offset, out_frequency_offset,
                                               resample_height, resample_width,
                                               writePos, 0==myThreadIdx.x );

            printf("%3u [%u, %u](%u) > [%u, %u]\n", x,
                p.block_first_read, p.block_last_read, p.num_reads,
                p.thread_first_read, p.thread_last_read);
        }
    }

    switch (version)
    {
    case 1:
        {
            cudaChannelFormatDesc channelDesc = cudaCreateChannelDesc<float2>();
            cudaBindTexture(0, &chunkTexture, inChunk.ptr(), &channelDesc, inChunk.getCudaPitchedPtr().pitch * inChunk.getCudaPitchedPtr().ysize);

            kernel_merge_chunk_old<<<grid, block, cuda_stream>>>(
                inChunk, outBlock,
                resample_width,
                resample_height,
                in_sample_offset, out_sample_offset, in_count );
            cudaUnbindTexture(&chunkTexture);
            break;
        }
    case 2:
        {
            kernel_merge_chunk2<<<grid, block, cuda_stream>>>(
                inChunk, outBlock,
                resample_width,
                resample_height,
                in_sample_offset, out_sample_offset,
                in_frequency_offset, out_frequency_offset,
                in_count, complexInfo );
            break;
        }
    }

    cudaUnbindTexture(&chunkTexture);
}

__global__ void kernel_expand_stft(
                cudaPitchedPtrType<float2> inStft,
                cudaPitchedPtrType<float> outBlock,
                float start,
                float steplogsize,
                float out_offset,
                float out_length )
{
    // Element number
    const unsigned
            y = blockIdx.x*blockDim.x + threadIdx.x;

    unsigned nFrequencies = outBlock.getNumberOfElements().y;
    if( y >= nFrequencies )
        return;

    float ff = y/(float)nFrequencies;
    float hz_out = start*exp(ff*steplogsize);

    float max_stft_hz = 44100.f/2;
    float min_stft_hz = 44100.f/(2*inStft.getNumberOfElements().x);
    float read_f = max(0.f,min(1.f,(hz_out-min_stft_hz)/(max_stft_hz-min_stft_hz)));

    float2 c;

    float p = read_f*inStft.getNumberOfElements().x;
    elemSize3_t readPos = make_elemSize3_t( p, 0, 0 );
    inStft.clamp(readPos);
    c = inStft.elem(readPos);
    float val1 = sqrt(c.x*c.x + c.y*c.y);

    readPos.x++;
    inStft.clamp(readPos);
    c = inStft.elem(readPos);
    float val2 = sqrt(c.x*c.x + c.y*c.y);

    p-=(unsigned)p;
    float val = .02f*(val1*(1-p)+val2*p);
    const float f0 = 2.0f + 35*ff*ff*ff;
    val*=f0;

    elemSize3_t writePos = make_elemSize3_t( 0, y, 0 );
    for (writePos.x=out_offset; writePos.x<out_offset + out_length && writePos.x<outBlock.getNumberOfElements().x;writePos.x++)
    {
        outBlock.e( writePos ) = val;
    }
}


extern "C"
void expandStft( cudaPitchedPtrType<float2> inStft,
                 cudaPitchedPtrType<float> outBlock,
                 float min_hz,
                 float max_hz,
                 float out_offset,
                 float out_length,
                 unsigned cuda_stream)
{
    dim3 block(256,1,1);
    dim3 grid( int_div_ceil(outBlock.getNumberOfElements().y, block.x), 1, 1);

    if(grid.x>65535) {
        printf("====================\nInvalid argument, number of floats in complex signal must be less than 65535*256.\n===================\n");
        return;
    }

    float start = min_hz/2;
    float steplogsize = log(max_hz)-log(min_hz);

    kernel_expand_stft<<<grid, block, cuda_stream>>>(
        inStft, outBlock,
        start,
        steplogsize,
        out_offset,
        out_length );
}


// TODO optimize this reading/writing pattern
__global__ void kernel_expand_complete_stft(
                cudaPitchedPtrType<float2> inStft,
                cudaPitchedPtrType<float> outBlock,
                float start,
                float steplogsize,
                float out_stft_size,
                float out_offset,
                float in_min_hz,
                float in_max_hz,
                unsigned in_stft_size)
{
    // Element number
    const unsigned
            x = blockIdx.x*blockDim.x + threadIdx.x,
            y = blockIdx.y*blockDim.y + threadIdx.y;

    float val;
    /*if (1 || 0==threadIdx.x)*/ {
        unsigned nFrequencies = outBlock.getNumberOfElements().y-1;

        // abort if this thread would have written outside outBlock
        if( y > nFrequencies )
            return;

        // which frequency should this thread write
        float ff = y/(float)nFrequencies;
        float hz_write = start*exp(ff*steplogsize);

        // which timestep column should this thread write
        float ts_write = x + out_offset;

        // which normalized frequency should we start reading from
        float hz_read_norm = 0.5f * saturate( (hz_write - in_min_hz)/(in_max_hz - in_min_hz) );

        // which timestep column should we start reading from
        float ts_read = ts_write / out_stft_size;

        if ( 0 > ts_read )
            // only happens if 0>out_offse (or if out_stft_size is negative which is an error)
            return;

        // Compute read coordinates
        // q and p measures how bad read_start is an approximation to ts_read
        // and hz_read_norm
        float q = ts_read - 0.5f;
        float p = max(0.f, min( hz_read_norm*in_stft_size + 0.5f, in_stft_size-1.f ));

        unsigned ts_start = 0 > q ? (unsigned)-1 : (unsigned)q;
        unsigned hz_start = (unsigned)p;
        q -= floor(q);
        p -= hz_start;

        // if the next timestep column is required to compute this outBlock
        // pixel don't compute it unless the next timestep column is provided
        if (0 < q && ts_start+1>=inStft.getNumberOfElements().y)
            return;

        // if a previous timestep column is before the first column, use 0
        // instead

        // if the next or previous frequency row is needed, just clamp to the
        // provided range. Not generic but wil have to work for now.

        unsigned hz_secondline = min(hz_start+1, in_stft_size-1);

        float2 c;
        float val1, val2, val3, val4;
        if (ts_start == (unsigned)-1)
        {
            val1 = 0;
            val3 = 0;
        }
        else
        {
            c = inStft.elem(make_elemSize3_t( hz_start, ts_start, 0 ));
            val1 = sqrt(c.x*c.x + c.y*c.y);

            c = inStft.elem(make_elemSize3_t( hz_secondline, ts_start, 0 ));
            val3 = sqrt(c.x*c.x + c.y*c.y);
        }

        c = inStft.elem(make_elemSize3_t( hz_start, ts_start+1, 0 ));
        val2 = sqrt(c.x*c.x + c.y*c.y);

        c = inStft.elem(make_elemSize3_t( hz_secondline, ts_start+1, 0 ));
        val4 = sqrt(c.x*c.x + c.y*c.y);

        // Perform a kind of bicubic interpolation
        p = 3*p*p-2*p*p*p; // p and q are saturated, these equations compute
        q = 3*q*q-2*q*q*q; // an 'S' curve from 0 to 1.
        val = .07f*((val1*(1-q)+val2*q)*(1-p) + (val3*(1-q)+val4*q)*p);

        const float f0 = 2.0f + 35*ff*ff*ff;
//        const float f0 = 15.f;
//        val*=sqrt(f0);
        val*=f0;

        //float if0 = 40.f/(2.0f + 35*ff*ff*ff);
        //float if0 = 40.f/(2.0f + 35.f*ff*ff*ff);
        //if0=if0*if0*if0;
        //val=sqrt(if0*val);

        val*=4;
    }

    val /= in_stft_size;

    elemSize3_t writePos = make_elemSize3_t( x, y, 0 );
    outBlock.e( writePos ) = val;
}


extern "C"
void expandCompleteStft( cudaPitchedPtrType<float2> inStft,
                 cudaPitchedPtrType<float> outBlock,
                 float out_min_hz,
                 float out_max_hz,
                 float out_stft_size,
                 float out_offset,
                 float in_min_hz,
                 float in_max_hz,
                 unsigned in_stft_size,
                 unsigned cuda_stream)
{
    dim3 block(32,1,1);
    dim3 grid( outBlock.getNumberOfElements().x/block.x, outBlock.getNumberOfElements().y, 1);

    if(grid.x>65535 || grid.y>65535 || 0!=(in_stft_size%32)) {
        printf("====================\n"
               "Invalid argument, expandCompleteStft.\n"
               "grid.x=%u || grid.y=%u || in_stft_size=%u\n"
               "===================\n",
               grid.x, grid.y, in_stft_size
               );
        return;
    }

    float start = out_min_hz;
    float steplogsize = log(out_max_hz)-log(out_min_hz);

    kernel_expand_complete_stft<<<grid, block, cuda_stream>>>(
        inStft, outBlock,
        start,
        steplogsize,
        out_stft_size,
        out_offset,
        in_min_hz,
        in_max_hz,
        in_stft_size );
}
