#ifndef SIGNAL_PROCESSING_WORKER_H
#define SIGNAL_PROCESSING_WORKER_H

#include "ischedule.h"
#include "signal/computingengine.h"

#include "shared_state.h"

#include <QThread>
#include <QPointer>

#include <boost/exception/all.hpp>
#include <boost/exception_ptr.hpp>

namespace Signal {
namespace Processing {

/**
 * @brief The Worker class should run tasks as given by the scheduler.
 *
 * It should wait to be dispatched with a wakeup signal if there are no tasks.
 *
 * It should store information about a crashed task (both segfault and
 * std::exception as well as LockFailed) and stop execution.
 *
 * It should not hang if it causes a deadlock.
 * In the sense that Worker.terminate () still works;
 *
 * It should announce when tasks are finished.
 */
class Worker: public QObject
{
    Q_OBJECT
public:
    typedef QPointer<Worker> ptr;

    class TerminatedException: virtual public boost::exception, virtual public std::exception {};

    Worker (Signal::ComputingEngine::ptr computing_eninge, ISchedule::ptr schedule);
    ~Worker ();

    void abort();
    void terminate();
    bool wait(unsigned long time_ms = ULONG_MAX);
    bool isRunning() const;

    // 'if (caught_exception())' will be true if an exception was caught.
    //
    // To examine the exception. Use this pattern:
    //
    //     try {
    //         rethrow_exception(caught_exception ());
    //     } catch ( std::exception& x ) {
    //         x.what();
    //         ... get_error_info<...>(x);
    //         boost::diagnostic_information(x);
    //     }
    std::exception_ptr caught_exception() const;

signals:
    void oneTaskDone();
    void finished(std::exception_ptr, Signal::ComputingEngine::ptr);

public slots:
    void wakeup();

private slots:
    void finished();

private:
    void loop_while_tasks();

    Signal::ComputingEngine::ptr            computing_engine_;
    ISchedule::ptr                          schedule_;

    QThread*                                thread_;
    shared_state<std::exception_ptr>        exception_;
    std::exception_ptr                      terminated_exception_;

public:
    static void test ();
};


} // namespace Processing
} // namespace Signal

#endif // SIGNAL_PROCESSING_WORKER_H
