#ifndef MATLABCONTROLLER_H
#define MATLABCONTROLLER_H

#include <signal/operation.h>

#include <QObject>

namespace Sawe { class Project; }
namespace Signal { class Worker; }
namespace Adapters { class MatlabOperation; }

namespace Tools
{
    class RenderView;

    class MatlabController: public QObject {
        Q_OBJECT
    public:
        MatlabController( Sawe::Project* project, RenderView* render_view );
        ~MatlabController();

    private slots:
        void receiveMatlabOperation();
        void receiveMatlabFilter();
        void tryHeadAsMatlabOperation();

    private:
        // Model
        // Model that is controlled, this controller doesn't have a view
        // and shares control of the worker with many others
        Sawe::Project* project_;

        RenderView* render_view_;

        // GUI
        // The fact that this controller doesn't have a view doesn't mean
        // it can't have widgets. QToolButton widgets are created by adding
        // their corresponding action to a ToolBar.
        // TODO Could also add this functionality to a menu.
        void setupGui(Sawe::Project* project);
        void prepareLogView( Adapters::MatlabOperation*m );
    };
} // namespace Tools
#endif // MATLABCONTROLLER_H
