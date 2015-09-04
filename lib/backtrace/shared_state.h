/**
 * Include: shared_state.h, shared_state_mutex.h, shared_timed_mutex_polyfill.h
 * Library: C++11 only
 *
 * The shared_state class is a smart pointer that guarantees thread-safe access
 * to shared states in a multithreaded environment, with
 *
 *   - compile-time errors on write attempts from shared read-only locks and,
 *   - run-time exceptions on lock timeouts
 *
 * shared_state can be extended with type traits to get, for instance,
 *
 *   - backtraces on deadlocks from all participating threads,
 *   - warnings on locks that are held too long.
 *
 *
 * In a nutshell
 * -------------
 *
 *        shared_state<A> a {new A};
 *        ...
 *        a->foo ();         // Mutually exclusive write access
 *
 *
 * Using shared_state
 * ------------------
 * Use shared_state to ensure thread-safe access to otherwise unprotected data
 * of class MyType by wrapping all 'MyType* p = new MyType' calls as:
 *
 *        shared_state<MyType> p {new MyType};
 *
 *
 * There are a couple of ways to access the data in 'p'. Call "p.write()" to
 * enter a critical section for read and write access. The critical section is
 * thread-safe and exception-safe through a mutex lock and RAII. p.write() can
 * be used either implicitly in a single function call:
 *
 *        p->...
 *
 * Equivalent to:
 *
 *        p.write ()->...
 *
 * Or to enter a critical section to perform a transaction over multiple method
 * calls:
 *
 *        {
 *          auto w = p.write ();
 *          w->...
 *          w->...
 *        }
 *
 * Enter a critical section only if the lock is readily available:
 *
 *        if (auto w = p.try_write ())
 *        {
 *          w->...
 *          w->...
 *        }
 *
 * Like-wise 'p.read()' or 'p.try_read()' creates a critical section with
 * shared read-only access. An excepion is thrown if a lock couldn't be
 * obtained within a given timeout. The timeout is set, or disabled, by
 * shared_state_traits<MyType>:
 *
 *        try {
 *          p.write ()->...
 *        } catch (lock_failed) {
 *          ...
 *        }
 *
 * You can also discard the thread safety and get unprotected access to a
 * mutable state:
 *
 *        {
 *          MyType* m = p.raw ();
 *          m->...
 *          m->...
 *        }
 *
 * For more complete examples (that actually compile) see
 * shared_state_test::test () in shared_state.cpp.
 *
 *
 * Performance and overhead
 * ------------------------
 * shared_state should cause an overhead of less than 0.1 microseconds in a
 * 'release' build when using 'try_write' or 'try_read'.
 *
 * shared_state should fail within 0.1 microseconds in a 'release' build when
 * using 'try_write' or 'try_read' on a busy lock.
 *
 * shared_state should cause an overhead of less than 0.3 microseconds in a
 * 'release' build when using 'write' or 'read'.
 *
 *
 * Configuring timeouts and extending functionality
 * ------------------------------------------------
 * It is possible to use different timeouts, or disable timeouts, for different
 * types. Create a template specialization of shared_state_traits to override
 * the defaults. Alternatively you can also create an internal class called
 * shared_state_traits within your type. See 'shared_state_traits_default' for
 * more details.
 *
 *
 * Author: johan.b.gustafsson@gmail.com
 */

#ifndef SHARED_STATE_H
#define SHARED_STATE_H

template<class T>
class shared_state;

#include "shared_state_mutex.h"

class lock_failed: public virtual std::exception {};


struct shared_state_traits_default {
    /**
     If 'timeout >= 0' read_ptr/write_ptr will try to lock until the timeout
     has passed and then throw a lock_failed exception. If 'timeout < 0'
     they will block indefinitely until the lock becomes available.

     Define SHARED_STATE_NO_TIMEOUT to disable timeouts altogether, all lock
     attempts will then block indefinitely until the lock becomes available.

     timeout() must be reentrant, i.e thread-safe without the support of
     shared_state.
     */
    double timeout () { return 0.100; }

    /**
     If 'timeout_failed' does not throw the attempted read_ptr or write_ptr will
     become a null-pointer.
     */
    template<class T>
    void timeout_failed (T*) {
        throw typename shared_state<T>::lock_failed {};
    }

    /**
     'locked' and 'unlocked' are called right after the mutex for this is
     instance is locked or unlocked, respectively. Regardless if the lock
     is a read-only lock or a read-and-write lock.
     */
    template<class T>
    void locked (T*) {}

    template<class T>
    void unlocked (T*) {}

    /**
     * @brief shared_state_mutex defines which mutex shared_state will use to
     * protect this type. The default is to use a mutex with support for shared
     * read access but exclusive write access. Depending on your usage pattern
     * it might be better to use a simpler type.
     *
     * See shared_state_mutex.h for possible implementations.
     */
    typedef ::shared_state_mutex shared_state_mutex;

    /**
     * @brief enable_implicit_lock defines if the -> operator is enough to
     * obtain a lock. Otherwise, explicit .write() and .read() are needed.
     */
    struct enable_implicit_lock : std::true_type {};
};

template<class C>
struct shared_state_traits: public shared_state_traits_default {};

template<class T>
struct shared_state_details_helper {
    // The first template function "test" is used to detect if
    // "typename C::shared_state_traits" exists.
    // SFINAE skips this declaration if the subtype does not exist and instead
    // lets the second template declare "test".
    template<typename C>
    static typename C::shared_state_traits test(typename C::shared_state_traits*);

    template<typename C> // worst match
    static shared_state_traits<C> test(...);

    // shared_state_details_helper::type represents the return type of the
    // template function "test"
    typedef decltype(test<T>(0)) type;
};

template<typename T>
struct shared_state_details: public shared_state_details_helper<T>::type {
    shared_state_details(T*p) : p(p) {}
    shared_state_details(shared_state_details const&) = delete;
    shared_state_details& operator=(shared_state_details const&) = delete;
    ~shared_state_details() { delete p; }

    typedef typename shared_state_details_helper<T>::type::shared_state_mutex shared_state_mutex;
    mutable shared_state_mutex lock;

    T* const p;
};


// compare with std::enable_if
template <bool, class Tp = void> struct disable_if {};
template <class T> struct disable_if<false, T> {typedef T type;};


template<class T>
class shared_state final
{
private:
    typedef shared_state_details<typename std::remove_const<T>::type> details;

public:
    typedef typename std::remove_const<T>::type element_type;

    class lock_failed: public ::lock_failed {};

    shared_state () {}

    template<class Y,
             class = typename std::enable_if <std::is_convertible<Y*, element_type*>::value>::type>
    explicit shared_state ( Y* p )
    {
        reset(p);
    }

    template<class Y,
             class = typename std::enable_if <std::is_convertible<Y*, element_type*>::value>::type>
    shared_state(const shared_state<Y>& a)
    {
        d = a.d;
    }

    void reset() {
        d.reset ();
    }

    template<class Y,
             class = typename std::enable_if <std::is_convertible<Y*, element_type*>::value>::type>
    void reset( Y* yp ) {
        d.reset (new details(yp));
    }

    class weak_ptr {
    public:
        weak_ptr() {}
        weak_ptr(const shared_state& t) : d(t.d) {}

        shared_state lock() const {
            return shared_state{d.lock ()};
        }

    private:
        std::weak_ptr<details> d;
    };

    /**
     * For examples of usage see void shared_state_test::test ().
     *
     * The purpose of read_ptr is to provide thread-safe access to an a const
     * object for a thread during the lifetime of the read_ptr. This access
     * may be shared by multiple threads that simultaneously use their own
     * read_ptr to access the same object.
     *
     * The accessors without no_lock_failed always returns an accessible
     * instance, never null. If a lock fails a lock_failed exception is thrown.
     *
     * @see void shared_state_test::test ()
     * @see class shared_state
     * @see class write_ptr
     */
    class read_ptr {
    public:
        read_ptr() : l(0), p(0) {}

        read_ptr(read_ptr&& b)
            :   read_ptr()
        {
            swap(b);
        }

        read_ptr(const read_ptr&) = delete;
        read_ptr& operator=(read_ptr const&) = delete;

        ~read_ptr() {
            unlock ();
        }

#ifdef _DEBUG
        const T* operator-> () const { assert(p); return p; }
        const T& operator* () const { assert(p); return *p; }
#else
        const T* operator-> () const { return p; }
        const T& operator* () const { return *p; }
#endif
        const T* get () const { return p; }
        explicit operator bool() const { return (bool)p; }

        void lock() {
            // l is not locked, but timeout is required to be reentrant
            double timeout = d->timeout();

            // try_lock_shared_for and lock_shared are unnecessarily complex if
            // the lock is available right away
            if (l->try_lock_shared ())
            {
                // Got lock
            }
            else if (timeout < 0)
            {
                l->lock_shared ();
                // Got lock
            }
            else if (l->try_lock_shared_for (shared_state_chrono::duration<double>{timeout}))
            {
                // Got lock
            }
            else
            {
                d->template timeout_failed<T> (d->p);
                // timeout_failed is expected to throw. But if it doesn't,
                // make this behave as a null pointer
                return;
            }

            p = d->p;

            try {
                d->locked (p);
            } catch (...) {
                p = 0;
                l->unlock_shared ();
                throw;
            }
        }

        void unlock() {
            if (p)
            {
                const T* q = p;
                p = 0;
                l->unlock_shared ();
                d->unlocked (q);
            }
        }

        void swap(read_ptr& b) {
            std::swap(l, b.l);
            std::swap(d, b.d);
            std::swap(p, b.p);
        }

    private:
        friend class shared_state;

        explicit read_ptr (const shared_state& vp)
            :   l (&vp.d->lock),
                d (vp.d),
                p (0)
        {
            lock ();
        }

        read_ptr (const shared_state& vp, bool)
            :   l (&vp.d->lock),
                d (vp.d),
                p (0)
        {
            if (l->try_lock_shared ()) {
                p = d->p;
                d->locked (p);
            }
        }

        typename details::shared_state_mutex* l;
        std::shared_ptr<details> d;
        const element_type* p;
    };


    /**
     * For examples of usage see void shared_state_test::test ().
     *
     * The purpose of write_ptr is to provide exclusive access to an object for
     * a single thread during the lifetime of the write_ptr.
     *
     * @see class read_ptr
     */
    class write_ptr {
    public:
        write_ptr() : l(0), p(0) {}

        write_ptr(write_ptr&& b)
            :   write_ptr()
        {
            swap(b);
        }

        write_ptr(const write_ptr&) = delete;
        write_ptr& operator=(write_ptr const&) = delete;

        ~write_ptr() {
            unlock ();
        }

#ifdef _DEBUG
        T* operator-> () const { assert(p); return p; }
        T& operator* () const { assert(p); return *p; }
#else
        T* operator-> () const { return p; }
        T& operator* () const { return *p; }
#endif
        T* get () const { return p; }
        explicit operator bool() const { return (bool)p; }

        // See read_ptr::lock
        void lock() {
            double timeout = d->timeout();

            if (l->try_lock())
            {
            }
            else if (timeout < 0)
            {
                l->lock ();
            }
            else if (l->try_lock_for (shared_state_chrono::duration<double>{timeout}))
            {
            }
            else
            {
                d->timeout_failed(d->p);
                return;
            }

            p = d->p;

            try {
                d->locked (p);
            } catch (...) {
                p = 0;
                l->unlock ();
                throw;
            }
        }

        void unlock() {
            if (p)
            {
                T* q = p;
                p = 0;
                l->unlock ();
                d->unlocked (q);
            }
        }

        void swap(write_ptr& b) {
            std::swap(l, b.l);
            std::swap(d, b.d);
            std::swap(p, b.p);
        }

    private:
        friend class shared_state;

        template<class = typename disable_if <std::is_convertible<const T*, T*>::value>::type>
        explicit write_ptr (const shared_state& vp)
            :   l (&vp.d->lock),
                d (vp.d),
                p (0)
        {
            lock ();
        }

        template<class = typename disable_if <std::is_convertible<const T*, T*>::value>::type>
        write_ptr (const shared_state& vp, bool)
            :   l (&vp.d->lock),
                d (vp.d),
                p (0)
        {
            if (l->try_lock ()) {
                p = d->p;
                d->locked (p);
            }
        }

        typename details::shared_state_mutex* l;
        std::shared_ptr<details> d;
        T* p;
    };


    /**
     * @brief read provides thread safe read-only access.
     */
    read_ptr read() const { return read_ptr(*this); }

    /**
     * @brief write provides thread safe read and write access. Not accessible
     * if T is const.
     */
    write_ptr write() const { return write_ptr(*this); }

    /**
     * @brief try_read obtains the lock only if it is readily available.
     *
     * If the lock was not obtained it doesn't throw any exception, but the
     * accessors return null pointers. This function fails much faster (about
     * 30x faster) than setting timeout=0 and discarding any lock_failed.
     */
    read_ptr try_read() const { return read_ptr(*this, bool()); }

    /**
     * @brief try_write. See try_read.
     */
    write_ptr try_write() const { return write_ptr(*this, bool()); }

    /**
     * @brief mutex returns the mutex object for this instance.
     */
    typename details::shared_state_mutex& mutex() const { return d->lock; }

    /**
     * @brief raw gives direct access to the unprotected state. The client is
     * responsible for using other synchornization mechanisms, consider using
     * read() or write() instead.
     */
    T* raw() const { return d ? d->p : nullptr; }

    /**
     * @brief traits provides unprotected access to the instance of
     * shared_state_traits used for this type.
     * The shared_state is not released as long as the shared_ptr return from
     * traits is alive.
     */
    std::shared_ptr<typename shared_state_details_helper<typename std::remove_const<T>::type>::type> traits() const { return d; }

    explicit operator bool() const { return (bool)d; }
    bool operator== (const shared_state& b) const { return d == b.d; }
    bool operator!= (const shared_state& b) const { return !(*this == b); }
    bool operator < (const shared_state& b) const { return this->d < b.d; }

#ifndef SHARED_STATE_DISABLE_IMPLICIT_LOCK
    static write_ptr lock_type_test(element_type*);
    static read_ptr lock_type_test(element_type const*);
    typedef decltype(lock_type_test((T*)0)) lock_type;

    lock_type operator-> () const {
        typedef typename std::enable_if <details::enable_implicit_lock::value>::type method_is_disabled_for_this_type;
        return lock_type(*this); }
    lock_type get () const {
        typedef typename std::enable_if <details::enable_implicit_lock::value>::type method_is_disabled_for_this_type;
        return lock_type(*this); }
#endif

    bool unique() const { return d.unique (); }

    void swap(shared_state& b) {
        std::swap(d, b.d);
    }

private:
    template<typename Y>
    friend class shared_state;

    shared_state ( std::shared_ptr<details> d ) : d(d) {}

    std::shared_ptr<details> d;
};


template<class T>
inline void swap(typename shared_state<T>::read_ptr& a, typename shared_state<T>::read_ptr& b)
{
    a.swap(b);
}


template<class T>
inline void swap(typename shared_state<T>::write_ptr& a, typename shared_state<T>::write_ptr& b)
{
    a.swap(b);
}


namespace shared_state_test {
    void test ();
}

#endif // SHARED_STATE_H
