#include "clamdfftkernelbuffer.h"

#include "TaskTimer.h"


CLAMDFFTKernelBuffer::CLAMDFFTKernelBuffer()
{

}


CLAMDFFTKernelBuffer::~CLAMDFFTKernelBuffer()
{
    for (PlanMap::iterator i=kernels.begin(); i!=kernels.end(); ++i)
	{
        clAmdFftDestroyPlan(&(i->second));
	}
}


clAmdFftPlanHandle CLAMDFFTKernelBuffer::getPlan(OpenCLContext* c, unsigned int n, clAmdFftStatus& error)
{
    if (kernels.find(n) != kernels.end())
    {
        error = CLFFT_SUCCESS;
        return kernels[n];
	}

    TaskTimer tt("Creating an OpenCL AMD FFT compute plan for n=%u", n);

    //clFFT_Dim3 ndim = { n, 1, 1 };
    //clFFT_Plan plan = clFFT_CreatePlan(c, ndim, clFFT_1D, clFFT_InterleavedComplexFormat, &error);
    size_t clLengths[] = { n, 1, 1 };
    clAmdFftPlanHandle plan;
    //Default: Batch Size 1, single precision, scaling 1.0 forward, 1.0 / P backward,
    //inplace, complex interleaved input and output, strides same for output and input.
    error = clAmdFftCreateDefaultPlan(&plan, c->getContext(), CLFFT_1D, clLengths);

    if (error == CLFFT_SUCCESS)
        kernels[n] = plan;

    error = clAmdFftBakePlan(plan, 1, &c->getCommandQueue(), NULL, NULL);

	return plan;
}
