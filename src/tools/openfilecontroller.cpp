#include "openfilecontroller.h"

namespace Tools {


OpenfileController* OpenfileController::
        instance()
{
    static OpenfileController ofc;
    return &ofc;
}


OpenfileController::OpenfileController(QObject *parent) :
    QObject(parent)
{
}


void OpenfileController::
        registerOpener(QPointer<OpenfileInterface> file_opener)
{
    file_openers.push_back (file_opener);
}


QList<std::pair<QString,QString> > OpenfileController::
        patterns()
{
    Patterns R;

    foreach(QPointer<OpenfileInterface> file_opener, file_openers) {
        foreach(Patterns::value_type p, file_opener->patterns ()) {
            R.push_back (p);
        }
    }

    return R;
}


Signal::OperationDesc::ptr OpenfileController::
        reopen(QString url, Signal::OperationDesc::ptr prev)
{
    Signal::OperationDesc::ptr o;

    // TODO match url against patterns and try matching patterns first
    // string suffix = QFileInfo(filename.c_str()).completeSuffix().toLower().toStdString();

    foreach(QPointer<OpenfileInterface> file_opener, file_openers) {
        if ((o = file_opener->reopen (url, prev)))
            return o;
    }

    return o;
}


} // namespace Tools


#include "exceptionassert.h"

// Test

namespace Tools {

class DummyFileOperationDesc : public Signal::OperationDesc {
public:
    DummyFileOperationDesc(QString which):which(which) {}
    virtual Signal::Interval requiredInterval( const Signal::Interval& I, Signal::Interval* ) const { return I; }
    virtual Signal::Interval affectedInterval( const Signal::Interval& I ) const { return I; }
    virtual OperationDesc::ptr copy() const { return OperationDesc::ptr(); }
    virtual Signal::Operation::ptr createOperation(Signal::ComputingEngine*) const { return Signal::Operation::ptr(); }
    virtual QString toString() const { return which; }

private:
    QString which;
};

class DummyFileOpener : public OpenfileController::OpenfileInterface {
public:
    DummyFileOpener(QString which):which(which) {}

    Patterns patterns() {
        std::pair<QString,QString> pattern("*.dummy", "Dummy files " + which);
        QList<std::pair<QString,QString> > R;
        R.push_back (pattern);
        return R;
    }

    Signal::OperationDesc::ptr reopen(QString url, Signal::OperationDesc::ptr) {
        if (url == which)
            return Signal::OperationDesc::ptr(new DummyFileOperationDesc(which));
        return Signal::OperationDesc::ptr();
    }

    QString which;
};

void OpenfileController::
        test()
{
    // It should provide a generic interface for opening files.
    {
        QPointer<OpenfileInterface> opener1(new DummyFileOpener("file1"));
        QPointer<OpenfileInterface> opener2(new DummyFileOpener("file2"));
        OpenfileController openfile;

        openfile.registerOpener (opener1);
        openfile.registerOpener (opener2);

        EXCEPTION_ASSERT_EQUALS( openfile.patterns ().first ().first.toStdString (), "*.dummy" );
        EXCEPTION_ASSERT_EQUALS( openfile.patterns ().first ().second.toStdString (), "Dummy files file1" );
        EXCEPTION_ASSERT_EQUALS( openfile.patterns ().last ().second.toStdString (), "Dummy files file2" );

        Signal::OperationDesc::ptr o = openfile.open("blaj");
        EXCEPTION_ASSERT(!o);

        o = openfile.open("file1");
        EXCEPTION_ASSERT(o);
        EXCEPTION_ASSERT(dynamic_cast<DummyFileOperationDesc*>(o.raw ()));
        EXCEPTION_ASSERT_EQUALS(o.read ()->toString().toStdString(), "file1");

        o = openfile.open("file0");
        EXCEPTION_ASSERT(!o);

        o = openfile.open("file2");
        EXCEPTION_ASSERT(o);
        EXCEPTION_ASSERT(dynamic_cast<DummyFileOperationDesc*>(o.raw ()));
        EXCEPTION_ASSERT_EQUALS(o.read ()->toString().toStdString(), "file2");
    }
}

} // namespace Tools
