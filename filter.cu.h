#ifndef FILTER_CU_H
#define FILTER_CU_H

#include <cuda_runtime.h>

void        removeDisc( float2* wavelet, cudaExtent numElem, float4 area );
void        removeRect( float2* wavelet, cudaExtent numElem, float4 area );
void        moveFilter( cudaPitchedPtrType<float2> chunk, float df, float min_hz, float max_hz, float sample_rate );

#endif // FILTER_CU_H
