#ifndef JUSTMISC_LAZY_H
#define JUSTMISC_LAZY_H

namespace JustMisc {

/**
 * @brief The lazy class should provide lazy initialization of a value that is
 * move constructible (not necessarily default constructible).
 */
template<class T>
class lazy
{
public:
    lazy() {}
    lazy(T&& t) : t(t) {}
    lazy(lazy&&b) : t(b.t) {b.t=0;}
    lazy(lazy&b)=delete;
    lazy& operator=(lazy&b)=delete;
    lazy& operator=(T&& v) { delete t; t = new T(std::move(v)); return *this; }
    ~lazy() { delete t; }

    operator T() const { return *t; }
    T* operator ->() { return t; }
private:
    T* t = 0;
};

} // namespace JustMisc

#endif // JUSTMISC_LAZY_H
