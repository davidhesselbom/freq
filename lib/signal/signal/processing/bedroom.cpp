#include "bedroom.h"

#include "exceptionassert.h"
#include "expectexception.h"
#include "timer.h"

#include <boost/foreach.hpp>

namespace Signal {
namespace Processing {


Bedroom::Data::
    Data()
    :
        sleepers(new Counter::element_type),
        skip_sleep_marker(new Counter::element_type),
        is_closed(false)
{
}


Bedroom::Bed::
        Bed(shared_state<Data> data)
    :
      data_(data)
{
    data_.write ()->beds.insert(this);
}


Bedroom::Bed::
        ~Bed()
{
    data_.write ()->beds.erase(this);
}


void Bedroom::Bed::
        sleep()
{
    sleep(ULONG_MAX);
}


bool Bedroom::Bed::
        sleep(unsigned long ms_timeout)
{
    auto data = data_.write ();

    if (data->is_closed) {
        BOOST_THROW_EXCEPTION(BedroomClosed() << Backtrace::make ());
    }

    bool r = true;

    // Increment usage count
    Counter b = data->sleepers;

    // Wait in a while-loop to cope with spurious wakeups
    if (ULONG_MAX == ms_timeout)
        while (!skip_sleep_)
            data->work.wait ( data_.mutex() );
    else
        while (r && !skip_sleep_)
            r = std::cv_status::no_timeout == data->work.wait_for (data_.mutex(), std::chrono::milliseconds(ms_timeout));

    skip_sleep_.reset();

    return r;
}


Bedroom::
        Bedroom()
    :
      data_(new Data)
{
}


void Bedroom::
        wakeup()
{
    auto data = data_.write ();
    // no one is going into sleep as long as data_ is locked

    for (Bed* b : data->beds)
        b->skip_sleep_ = data->skip_sleep_marker;

    data->work.notify_all ();
}


void Bedroom::
        close()
{
    data_->is_closed = true;

    wakeup();
}


Bedroom::Bed Bedroom::
        getBed()
{
    return Bed(data_);
}


int Bedroom::
        sleepers()
{
    // Remove 1 to compensate for the instance used by 'this'
    return data_->sleepers.use_count() - 1;
}


} // namespace Processing
} // namespace Signal


#include <QThread>

namespace Signal {
namespace Processing {

class SleepyFaceMock: public QThread {
public:
    SleepyFaceMock(Bedroom::ptr bedroom, int snooze) : bedroom_(bedroom), snooze_(snooze) {}

    void run() {
        Bedroom::Bed bed = bedroom_->getBed();
        do {
            bed.sleep ();
            --snooze_;
        } while(snooze_ > 0);
    }

    int snooze() { return snooze_; }

private:
    Bedroom::ptr bedroom_;
    int snooze_;
};


class SleepingBeautyMock: public QThread {
public:
    SleepingBeautyMock(Bedroom::ptr bedroom, int sleep_ms) : bedroom_(bedroom), sleep_ms_(sleep_ms) {}

    void run() {
        Bedroom::Bed bed = bedroom_->getBed();

        while (!bed.sleep (sleep_ms_))
            ++sleep_count_;
    }

    int sleep_count() { return sleep_count_; }

private:
    Bedroom::ptr bedroom_;
    int sleep_ms_;
    int sleep_count_ = 0;
};


void Bedroom::
        test()
{
    // It should allow different threads to sleep on this object until another thread calls wakeup()
    for (int j=0;j<2; j++) {
        Bedroom::ptr bedroom(new Bedroom);
        int snoozes = 10;
        SleepyFaceMock sleepyface1(bedroom, snoozes);
        SleepyFaceMock sleepyface2(bedroom, snoozes);

        sleepyface1.start ();
        sleepyface2.start ();

        for (int i=snoozes; i>=0; i--) {
            EXCEPTION_ASSERT_EQUALS(sleepyface1.wait (1), i>0?false:true);
            EXCEPTION_ASSERT_EQUALS(sleepyface2.wait (1), i>0?false:true);

            // sleepyface1 and sleepyface2 shoule be sleeping now
            EXCEPTION_ASSERT_EQUALS(bedroom->sleepers(), i>0?2:0);

            // they should have 'i' times left to snooze
            EXCEPTION_ASSERTX(sleepyface1.snooze () == i && sleepyface2.snooze () == i,
                              (boost::format("sleepyface1=%d, sleepyface2=%d, i=%d")
                              % sleepyface1.snooze () % sleepyface2.snooze () % i));

            // should wake up both
            bedroom->wakeup();
        }

        EXCEPTION_ASSERT(sleepyface1.isFinished ());
        EXCEPTION_ASSERT(sleepyface2.isFinished ());
        EXCEPTION_ASSERT_EQUALS(bedroom->sleepers(), 0);
    }

    // It should throw a BedroomClosed exception if someone tries to go to
    // sleep when the bedroom is closed.
    {
        Bedroom b;
        b.close ();
        EXPECT_EXCEPTION(BedroomClosed, b.getBed().sleep ());
    }

    // It should just sleep until the given timeout has elapsed
    {
        Bedroom b;
        Timer t;
        bool woken_up_by_wakeup_call = b.getBed ().sleep (2);

        EXCEPTION_ASSERT_LESS(t.elapsed (), 3e-3);
        EXCEPTION_ASSERT_LESS(2e-3, t.elapsed ());
        EXCEPTION_ASSERT(!woken_up_by_wakeup_call); // timeout
    }

    // It should just sleep until the given timeout has elapsed
    {
        Bedroom::ptr bedroom(new Bedroom);
        SleepingBeautyMock sbm(bedroom, 2);

        sbm.start ();
        EXCEPTION_ASSERT( !sbm.wait (7) );
        bedroom->wakeup();
        EXCEPTION_ASSERT( sbm.wait (2) );

        EXCEPTION_ASSERT(sbm.isFinished ());
        EXCEPTION_ASSERT_EQUALS( sbm.sleep_count (), 3 );
    }
}

} // namespace Processing
} // namespace Signal
