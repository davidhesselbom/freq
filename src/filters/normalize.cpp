#include "normalize.h"
#include "normalizekernel.h"
#include "cpumemorystorage.h"
#include "signal/operation-basic.h"
#include "stringprintf.h"

using namespace Signal;

namespace Filters {

Normalize::
        Normalize( unsigned normalizationRadius, pOperation source )
            :
            Operation(source),
            normalizationRadius(normalizationRadius)
{
}


Normalize::
        Normalize()
            :
            Operation(pOperation())
{}


std::string Normalize::
        name()
{
    return printfstring("Rolling normalization with radius %g s", normalizationRadius/sample_rate());
}


void Normalize::
        invalidate_samples(const Intervals& I)
{
    Operation::invalidate_samples(I.enlarge(normalizationRadius));
}


pBuffer Normalize::
        read( const Interval& I )
{
    Interval J = Intervals(I).enlarge(normalizationRadius).spannedInterval();
    pBuffer b = source()->readFixedLength(J);

    for (unsigned c=0; c<b->number_of_channels (); ++c)
       normalizedata( b->getChannel (c)->waveform_data(), normalizationRadius );

    // normalizedata moved the data (and made the last 2*normalizationRadius elements invalid)
    Signal::IntervalType sample_offset = J.first + normalizationRadius;

    pBuffer r( new Buffer(I.first, I.count(), b->sample_rate(), b->number_of_channels ()));
    for (unsigned c=0; c<b->number_of_channels (); ++c)
    {
        MonoBuffer mb(sample_offset, b->getChannel (c)->waveform_data (), b->sample_rate ());
        *r->getChannel (c) |= mb;
    }

    return r;
}


} // namespace Filters
