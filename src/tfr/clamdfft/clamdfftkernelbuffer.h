#ifndef CLAMDFFTKERNELBUFFER_H
#include "clAmdFft.h"
#include "openclcontext.h"
#include "HasSingleton.h"
#include <utility>
#include <map>

class CLAMDFFTKernelBuffer: public HasSingleton<CLAMDFFTKernelBuffer>
{
public:
    ~CLAMDFFTKernelBuffer();

    clAmdFftPlanHandle getPlan(OpenCLContext *c, unsigned int n, clAmdFftStatus& error);

protected:
    typedef std::map<unsigned int, clAmdFftPlanHandle> PlanMap;
    PlanMap kernels;

private:
    friend class HasSingleton<CLAMDFFTKernelBuffer>;
    CLAMDFFTKernelBuffer(); // CL_DEVICE_TYPE_GPU is NOT required for clAmdFft!
};

#endif //CLFFTKERNELBUFFER_H
