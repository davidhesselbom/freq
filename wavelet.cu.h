#ifndef WAVELET_CU_H
#define WAVELET_CU_H

const char* wtGetError();
void        wtCompute( float2* in_waveform_ft, float2* out_wavelet_ft, unsigned sampleRate, float minHz, float maxHz, cudaExtent numElem, cudaStream_t stream=0 );
void        wtInverse( float2* in_wavelet, float* out_inverse_waveform, cudaExtent numElem, uint4 area, cudaStream_t stream=0 );
void        wtClamp( float2* in_wt, cudaExtent in_numElem, size_t in_offset, size_t last_sample, float2* out_clamped_wt, cudaExtent out_numElem, cudaStream_t stream=0  );
void        removeDisc( float2* wavelet, cudaExtent numElem, uint4 area );

#endif // WAVELET_CU_H
