#ifndef STFTKERNEL_H
#define STFTKERNEL_H

#include "tfr/chunkdata.h"

void        stftNormalizeInverse( DataStorage<float>::Ptr wave, unsigned length );
void        stftNormalizeInverse( Tfr::ChunkData::Ptr inwave, DataStorage<float>::Ptr outwave, unsigned length );
inline void stftDiscardImag( Tfr::ChunkData::Ptr inwavep, DataStorage<float>::Ptr outwavep )
{
    stftNormalizeInverse(inwavep, outwavep, 1);
}

void        stftToComplex( DataStorage<float>::Ptr inwavep, Tfr::ChunkData::Ptr outwavep );

#endif // STFTKERNEL_H
