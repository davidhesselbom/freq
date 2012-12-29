#include "filter.h"
#include "signal/buffersource.h"
#include "tfr/chunk.h"
#include "tfr/transform.h"

#include "demangle.h"

#include <boost/format.hpp>

#include <QMutexLocker>

//#define TIME_Filter
#define TIME_Filter if(0)

//#define TIME_FilterReturn
#define TIME_FilterReturn if(0)


using namespace Signal;
using namespace boost;


namespace Tfr {

//////////// Filter
Filter::
        Filter( pOperation source )
            :
            Operation( source )
{}


Filter::
        Filter(Filter& f)
    :
      Operation(f)
{
    transform(f.transform ());
}


Signal::pBuffer Filter::
        read(  const Signal::Interval& I )
{
    TIME_Filter TaskTimer tt("%s Filter::read( %s )", vartype(*this).c_str(),
                             I.toString().c_str());

    QMutexLocker l(&_transform_mutex);
    pTransform t = _transform;
    l.unlock ();

    Signal::Interval required = requiredInterval(I, t);

    // If no samples would be non-zero, return zeros
    if (!(required - zeroed_samples_recursive()))
        return zeros(required);

    pBuffer b = source()->readFixedLength( required );
    if (this != affecting_source(required))
        return b;

    pBuffer r;
    for (unsigned c=0; c<b->number_of_channels (); ++c)
    {
        ChunkAndInverse ci;
        ci.channel = c;
        ci.t = t;
        ci.inverse = b->getChannel (c);

        ci.chunk = (*t)( ci.inverse );

        #ifdef _DEBUG
            Interval cii = ci.chunk->getInterval().spanned ( ci.chunk->getCoveredInterval () );

            EXCEPTION_ASSERT( cii & I );
        #endif

        if (applyFilter( ci ))
            ci.inverse = t->inverse (ci.chunk);

        if (!r)
            r.reset ( new Buffer(ci.inverse->getInterval (), ci.inverse->sample_rate (), b->number_of_channels ()));

        #ifdef _DEBUG
            Interval invinterval = ci.inverse->getInterval ();
            Interval i(I.first, I.first+1);
            if (!( i & invinterval ))
            {
                Signal::Interval required2 = requiredInterval(I, t);
                Interval cgi2 = ci.chunk->getInterval ();
                ci.inverse = b->getChannel (c);
                ci.chunk = (*t)( ci.inverse );
                ci.inverse = t->inverse (ci.chunk);
                EXCEPTION_ASSERT( i & invinterval );
            }
        #endif

        *r->getChannel (c) |= *ci.inverse;
    }

    return r;
}


Operation* Filter::
        affecting_source( const Interval& I )
{
    return Operation::affecting_source( I );
}


unsigned Filter::
        prev_good_size( unsigned current_valid_samples_per_chunk )
{
    return transform()->transformParams()->prev_good_size( current_valid_samples_per_chunk, sample_rate() );
}


unsigned Filter::
        next_good_size( unsigned current_valid_samples_per_chunk )
{
    return transform()->transformParams()->next_good_size( current_valid_samples_per_chunk, sample_rate() );
}


bool ChunkFilter::
        applyFilter( ChunkAndInverse& chunk )
{
    return (*this)( *chunk.chunk );
}


Tfr::pTransform Filter::
        transform()
{
    QMutexLocker l(&_transform_mutex);
    return _transform;
}


void Filter::
        transform( Tfr::pTransform t )
{
    QMutexLocker l(&_transform_mutex);

    if (_transform)
    {
        EXCEPTION_ASSERTX(typeid(*_transform) == typeid(*t), str(format("'transform' must be an instance of %s, was %s") % vartype(*_transform) % vartype(*t)));
    }

    if (_transform == t )
        return;

    _transform = t;

    l.unlock ();

    invalidate_samples( getInterval() );
}

} // namespace Tfr
