#include "timer.h"
#include "trace_perf.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#else
using namespace std::chrono;
#endif

Timer::Timer(bool start)
{
    if (start)
        restart();
}


void Timer::restart()
{
#ifdef _WIN32
    LARGE_INTEGER li;
    QueryPerformanceCounter(&li);
    start_ = li.QuadPart;
#else
    start_ = high_resolution_clock::now ();
#endif
}


double Timer::elapsed() const
{
#ifdef _WIN32
    LARGE_INTEGER li;
    static double PCfreq = 1;
    for(static bool doOnce=true;doOnce;doOnce=false)
    {
        QueryPerformanceFrequency(&li);
        PCfreq = double(li.QuadPart);
    }
    QueryPerformanceCounter(&li);
    return double(li.QuadPart-start_)/PCfreq;
#else
    duration<double> diff = high_resolution_clock::now () - start_;
    return diff.count();
#endif
}


double Timer::elapsedAndRestart()
{
#ifdef _WIN32
    LARGE_INTEGER li;
    static double PCfreq = 1;
    for(static bool doOnce=true;doOnce;doOnce=false)
    {
        QueryPerformanceFrequency(&li);
        PCfreq = double(li.QuadPart);
    }
    QueryPerformanceCounter(&li);
    int64_t now = li.QuadPart;
    double diff = double(now-start_)/PCfreq;
    start_ = now;
    return diff;
#else
    high_resolution_clock::time_point now = high_resolution_clock::now ();
    duration<double> diff = now - start_;
    start_ = now;
    return diff.count ();
#endif
}


void Timer::
        test()
{
    // It should measure duration with a high accuracy
    {
        TRACE_PERF("it should measure short intervals as short");

        trace_perf_.reset ("it should have a low overhead");

        {Timer t;t.elapsed ();}
    }

    // It should have an overhead less than 1 microsecond
    {
        TRACE_PERF("it should have a low overhead 10000");

        for (int i=0;i<10000;i++) {
            Timer t0;
            t0.elapsed ();
        }

        trace_perf_.reset ("it should produce stable measures 10000");

        for (int i=0;i<10000;i++) {
            Timer t0;
            t0.elapsed ();
        }
    }
}
