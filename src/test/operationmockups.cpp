#include "operationmockups.h"

namespace Test {


Signal::pBuffer TransparentOperation::
        process(Signal::pBuffer b)
{
    return b;
}


Signal::Interval TransparentOperation::
        requiredInterval( Signal::Interval& I )
{
    return I;
}


Signal::Operation::Ptr TransparentOperationDesc::
        createOperation(Signal::ComputingEngine*) const
{
    return Signal::Operation::Ptr( new TransparentOperation );
}


Signal::OperationDesc::Ptr TransparentOperationDesc::
        copy() const
{
    return OperationDesc::Ptr( new TransparentOperationDesc );
}


QString TransparentOperationDesc::
        toString() const
{
    return "TransparentOperationDesc";
}


bool TransparentOperationDesc::
        operator==(const OperationDesc& d) const
{
    return 0 != dynamic_cast<const TransparentOperationDesc*>(&d);
}

} // namespace Test
