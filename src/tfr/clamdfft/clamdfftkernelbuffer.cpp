#include "clamdfftkernelbuffer.h"

#include "TaskTimer.h"


CLAMDFFTKernelBuffer::CLAMDFFTKernelBuffer()
{

}


CLAMDFFTKernelBuffer::~CLAMDFFTKernelBuffer()
{
    // Don't try to clean up, let ClAmdFft take care of it.
	/* 
    for (PlanMap::iterator i=kernels.begin(); i!=kernels.end(); ++i)
	{
        clAmdFftDestroyPlan(&(i->second));
	}
	*/
}


clAmdFftPlanHandle CLAMDFFTKernelBuffer::getPlan(OpenCLContext* c, unsigned int n, clAmdFftStatus& error)
{
    if (kernels.find(n) != kernels.end())
    {
        error = CLFFT_SUCCESS;
        return kernels[n];
	}



    //clFFT_Dim3 ndim = { n, 1, 1 };
    //clFFT_Plan plan = clFFT_CreatePlan(c, ndim, clFFT_1D, clFFT_InterleavedComplexFormat, &error);
    size_t clLengths[] = { n, 1, 1 };
    clAmdFftPlanHandle plan;

#ifdef COPY_EXISTING_PLAN
    if (n != 1024)
    {
        TaskTimer tt("Copying and modifying a 1024 OpenCL AMD FFT compute plan for n=%u", n);
        error = clAmdFftCopyPlan(&plan, c->getContext(), kernels[1024]);
        error = clAmdFftSetPlanLength(plan, CLFFT_1D, clLengths);
    }
    else
#endif
    {
        TaskTimer tt("Creating an OpenCL AMD FFT compute plan for n=%u", n);
        error = clAmdFftCreateDefaultPlan(&plan, c->getContext(), CLFFT_1D, clLengths);
        //Default: Batch Size 1, single precision, scaling 1.0 forward, 1.0 / P backward,
        //inplace, complex interleaved input and output, strides same for output and input.
    //The defaults, explicitly - it is not recommended by AMD to rely on defaults.
    //One of the calls below crash the program, however, so commented out for now.
/*
	error = clAmdFftSetPlanBatchSize(plan, 1);
    error = clAmdFftSetPlanPrecision(plan, CLFFT_SINGLE); //CLFFT_SINGLE_FAST not yet supported
    error = clAmdFftSetPlanScale(plan, CLFFT_FORWARD, 1);
    error = clAmdFftSetPlanScale(plan, CLFFT_BACKWARD, 1.0f/n);
    error = clAmdFftSetResultLocation(plan, CLFFT_INPLACE);
    error = clAmdFftSetLayout(plan, CLFFT_COMPLEX_INTERLEAVED, CLFFT_COMPLEX_INTERLEAVED);
    error = clAmdFftSetPlanInStride(plan, CLFFT_1D, clLengths);
    error = clAmdFftSetPlanOutStride(plan, CLFFT_1D, clLengths);
*/
    }

    if (error == CLFFT_SUCCESS)
        kernels[n] = plan;

    //error = clAmdFftBakePlan(plan, 1, &c->getCommandQueue(), NULL, NULL);

	return plan;
}