#include "sinksource.h"

#ifndef SAWE_NO_MUTEX
#include <QMutexLocker>
#endif
#include <sstream>
#include <neat_math.h>

#include <boost/foreach.hpp>

static const bool D = false;

using namespace std;

namespace Signal {

SinkSource::
        SinkSource()
{
}


SinkSource::
        SinkSource( const SinkSource& b)
            :
        Sink(b),
        _cache( b._cache )
{
}


SinkSource& SinkSource::
        operator=( const SinkSource& b)
{
    _cache = b._cache;
    _invalid_samples = b._invalid_samples;
    return *this;
}


void SinkSource::
        put( pBuffer b )
{
    /* CANONICAL {
        QMutexLocker l(&_mutex);

        // Simply remove previous overlapping buffer, don't bother merging.
        foreach( pBuffer& s, _cache) {
        {
            if (!s)
                s = b;
            else if (s->sample_offset < b->sample_offset+b->number_of_samples() && s->sample_offset + s->number_of_samples() > b->sample_offset)
            {
                s = b;
                b.reset(); // If copied more than once, set others to 0
            }
        }
    }

    _cache.push_back( b );
    */

    merge(b);
}


void SinkSource::
        putExpectedSamples( pBuffer buffer, const Intervals& expected )
{
    BufferSource bs( buffer );

    Intervals I = expected & buffer->getInterval();
    BOOST_FOREACH( const Interval& i, I )
    {
        pBuffer s = bs.readFixedLength( i );
        put( s );
    }
}


void SinkSource::
        selfmerge()
{
	{
#ifndef SAWE_NO_MUTEX
		QMutexLocker l(&_cache_mutex);
#endif

		if (_cache.empty())
			return;
	}

    //TaskTimer tt("SinkSource::selfmerge");
    //samplesDesc().print("selfmerged start");
    //tt.info("_cache.size()=%u", _cache.size());

    Intervals sid = samplesDesc();
	std::vector<pBuffer> new_cache;

    BOOST_FOREACH( Interval i, sid )
	{
        for (unsigned L=0; i.first < i.last; i.first+=L)
		{
            L = lpo2s( i.count() + 1);

            if (L < (1<<13))
                L = i.count();

            new_cache.push_back(readFixedLength( Interval( i.first, i.first + L) ));
		}
	}

	{
#ifndef SAWE_NO_MUTEX
		QMutexLocker l(&_cache_mutex);
#endif
	    _cache = new_cache;
	}

    //samplesDesc().print("selfmerged finished");
    //tt.info("_cache.size()=%u", _cache.size());
}


void SinkSource::
        merge( pBuffer bp )
{
    bp->release_extra_resources();

    const Buffer& b = *bp;
    float FS = sample_rate();

    std::stringstream ss;
    pBuffer n;

	{
#ifndef SAWE_NO_MUTEX
    QMutexLocker cache_locker(&_cache_mutex);
#endif

    if (!_cache.empty())
        BOOST_ASSERT(_cache.front()->sample_rate == b.sample_rate);

    // REMOVE caches that become outdated by this new buffer 'b'
    for ( std::vector<pBuffer>::iterator itr = _cache.begin(); itr!=_cache.end(); )
    {
        pBuffer s = *itr;

        Intervals toKeep = s->getInterval();
        toKeep -= b.getInterval();

        Intervals toRemove = s->getInterval();
        toRemove &= b.getInterval();

        if (toRemove)
        {
            if(D) ss << " -" << s->getInterval().toString();

            // '_cache' is a vector but itr is most often the last element of the vector
            // thus making this operation inexpensive.
            itr = _cache.erase(itr); // Note: 'pBuffer s' stores a copy for the scope of the for-loop

            BOOST_FOREACH( Interval i, toKeep )
            {
                if(D) ss << " +" << i.toString();

                pBuffer n( new Buffer( i.first, i.count(), FS));
                *n |= *s;
                itr = _cache.insert(itr, n );
                itr++; // Move past inserted element
            }
        } else {
            itr++;
        }
    }

    n = pBuffer( new Buffer( b.sample_offset, b.number_of_samples(), b.sample_rate));
    *n |= b;
    _cache.push_back( n );
    } //cache_locker.unlock(); // finished working with _cache, samplesDesc() below needs the lock

    if(D) if (!ss.str().empty())
    {
        ss << " +" << n->getInterval().toString();
        TaskTimer("M:%s", ss.str().c_str()).suppressTiming();
    }

    _invalid_samples -= b.getInterval();

    selfmerge();
    //samplesDesc().print("SinkSource received samples");
    //_expected_samples.print("SinkSource expected samples");
}


void SinkSource::
        clear()
{
#ifndef SAWE_NO_MUTEX
    QMutexLocker l(&_cache_mutex);
#endif
    _cache.clear();
    _invalid_samples = Intervals();
}


pBuffer SinkSource::
        read( const Interval& I )
{
    Interval not_found = I;

    {
#ifndef SAWE_NO_MUTEX
        QMutexLocker l(&_cache_mutex);
#endif

        BOOST_FOREACH( const pBuffer& s, _cache) {
            if (s->sample_offset <= I.first && s->sample_offset + s->number_of_samples() > I.first )
            {
                if(D) TaskTimer("%s: sinksource [%u, %u] got [%u, %u]",
                             __FUNCTION__,
                             I.first,
                             I.last,
                             s->getInterval().first,
                             s->getInterval().last).suppressTiming();
                cudaExtent sz = s->waveform_data()->getNumberOfElements();

                return s;
            }
            if (s->sample_offset > I.first && s->sample_offset<not_found.last)
                not_found.last = s->sample_offset;
        }
    }

    // Return silence
    return zeros(not_found);
}


float SinkSource::
        sample_rate()
{
#ifndef SAWE_NO_MUTEX
    QMutexLocker l(&_cache_mutex);
#endif

    if (_cache.empty())
        return 44100;

    return _cache.front()->sample_rate;
}


long unsigned SinkSource::
        number_of_samples()
{
    return samplesDesc().coveredInterval().count();
}


pBuffer SinkSource::
        first_buffer()
{
#ifndef SAWE_NO_MUTEX
    QMutexLocker l(&_cache_mutex);
#endif
    BOOST_ASSERT( !_cache.empty() );

    if (_cache.empty())
        return pBuffer();
    return _cache.front();
}


bool SinkSource::empty()
{
#ifndef SAWE_NO_MUTEX
    QMutexLocker l(&_cache_mutex);
#endif
    return _cache.empty();
}


Intervals SinkSource::
        samplesDesc()
{
#ifndef SAWE_NO_MUTEX
    QMutexLocker l(&_cache_mutex);
#endif

    Intervals sid;

    BOOST_FOREACH( const pBuffer& s, _cache) {
        sid |= s->getInterval();
    }

    return sid;
}

} // namespace Signal