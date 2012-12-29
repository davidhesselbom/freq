#ifndef FFTCLAMDFFT_H
#define FFTCLAMDFFT_H

#include "tfr/fftimplementation.h"
#include <vector>
#include <string>
#include <sstream>
#include <iostream>

typedef size_t clAmdFftPlanHandle;

namespace Tfr {
    class FftClAmdFft: public FftImplementation {
    public:
        FftClAmdFft();
        ~FftClAmdFft();

        void compute( Tfr::ChunkData::Ptr input, Tfr::ChunkData::Ptr output, FftDirection direction );
        void computeR2C( DataStorage<float>::Ptr input, Tfr::ChunkData::Ptr output );
        void computeC2R( Tfr::ChunkData::Ptr input, DataStorage<float>::Ptr output );

        void compute( Tfr::ChunkData::Ptr input, Tfr::ChunkData::Ptr output, DataStorageSize n, FftDirection direction );
        void compute( DataStorage<float>::Ptr inputbuffer, Tfr::ChunkData::Ptr transform_data, DataStorageSize n );
        void inverse( Tfr::ChunkData::Ptr inputdata, DataStorage<float>::Ptr outputdata, DataStorageSize n );

        unsigned sChunkSizeG(unsigned x, unsigned multiple=1);
        unsigned lChunkSizeS(unsigned x, unsigned multiple=1);

        long long kernelExecTime;
		std::vector<std::string> execTimes;
		float bakeTime;
		float getLastBakeTime() { return bakeTime; }

		int getWallExecTime() { return 0; }
		long long getKernelExecTime() { return kernelExecTime; }
		
		void setSize(size_t newSize);
		void setBatchSize(size_t newBatchSize);
		void bake();

		void clearPlans();
		void createPlan(size_t newSize);
		
	    clAmdFftPlanHandle lastPlan;
    };
}

#endif // STFT_CLFFT_H
