#include "rectanglecontroller.h"
#include "rectanglemodel.h"

// Sonic AWE
#include "tools/selectioncontroller.h"
#include "filters/rectangle.h"
#include "sawe/project.h"
#include "ui_mainwindow.h"
#include "ui/mainwindow.h"
#include "tools/renderview.h"

// gpumisc
#include <TaskTimer.h>

// Qt
#include <QMouseEvent>

using namespace Tools;

namespace Tools { namespace Selections
{
    RectangleController::
            RectangleController(
                    RectangleView* view,
                    SelectionController* selection_controller
            )
        :   view_( view ),
            selection_button_( Qt::LeftButton ),
            selection_controller_( selection_controller )
    {
        setupGui();

        setAttribute( Qt::WA_DontShowOnScreen, true );
        setEnabled( false );
    }


    RectangleController::
            ~RectangleController()
    {
        TaskTimer(__FUNCTION__).suppressTiming();
    }


    void RectangleController::
            setupGui()
    {
        Ui::SaweMainWindow* main = selection_controller_->model()->project()->mainWindow();
        Ui::MainWindow* ui = main->getItems();

        // Connect enabled/disable actions,
        // 'enableRectangleSelection' sets/unsets this as current tool when
        // the action is checked/unchecked.
        connect(ui->actionRectangleSelection, SIGNAL(toggled(bool)), SLOT(enableRectangleSelection(bool)));
        connect(ui->actionFrequencySelection, SIGNAL(toggled(bool)), SLOT(enableFrequencySelection(bool)));
        connect(ui->actionTimeSelection, SIGNAL(toggled(bool)), SLOT(enableTimeSelection(bool)));
        connect(this, SIGNAL(enabledChanged(bool)), ui->actionRectangleSelection, SLOT(setChecked(bool)));
        connect(this, SIGNAL(enabledChanged(bool)), ui->actionFrequencySelection, SLOT(setChecked(bool)));
        connect(this, SIGNAL(enabledChanged(bool)), ui->actionTimeSelection, SLOT(setChecked(bool)));

        // Paint the ellipse when render view paints
        connect(selection_controller_->render_view(), SIGNAL(painting()), view_, SLOT(draw()));
        // Close this widget before the OpenGL context is destroyed to perform
        // proper cleanup of resources
        connect(selection_controller_->render_view(), SIGNAL(destroying()), SLOT(close()));

        // Add the action as a combo box item in selection controller
        selection_controller_->addComboBoxAction( ui->actionRectangleSelection ) ;
        selection_controller_->addComboBoxAction( ui->actionTimeSelection ) ;
        selection_controller_->addComboBoxAction( ui->actionFrequencySelection ) ;

        one_action_at_a_time_.reset( new Ui::ComboBoxAction() );
        one_action_at_a_time_->decheckable( false );
        one_action_at_a_time_->addActionItem( ui->actionRectangleSelection );
        one_action_at_a_time_->addActionItem( ui->actionTimeSelection );
        one_action_at_a_time_->addActionItem( ui->actionFrequencySelection );
    }


    void RectangleController::
            mousePressEvent ( QMouseEvent * e )
    {
        if (e->button() == selection_button_)
        {
            Tools::RenderView &r = *selection_controller_->render_view();
            r.makeCurrent(); // required for Ui::MouseControl::planePos

            if (false == Ui::MouseControl::planePos(
                    e->x(), height() - 1 - e->y(),
					selectionStart.time, selectionStart.scale, r.model->xscale))
            {
                selectionStart.time = -FLT_MAX;
            }

            model()->validate();
        }

        selection_controller_->render_view()->userinput_update();
    }


    void RectangleController::
            mouseReleaseEvent ( QMouseEvent * e )
    {
        if (e->button() == selection_button_)
        {
            selection_controller_->setCurrentSelection( model()->updateFilter() );
        }

        selection_controller_->render_view()->userinput_update();
    }


    void RectangleController::
            mouseMoveEvent ( QMouseEvent * e )
    {
        if (e->buttons().testFlag( selection_button_ ))
        {
            Tools::RenderView &r = *selection_controller_->render_view();
            r.makeCurrent(); // required for Ui::MouseControl::planePos

            Heightmap::Position p;
            if (Ui::MouseControl::planePos(
                    e->x(), height() - 1 - e->y(),
					p.time, p.scale, r.model->xscale))
            {
                if (-FLT_MAX == selectionStart.time) // TODO test
                {
                    selectionStart = p;
                }

                model()->a = selectionStart;
                model()->b = p;
            }

            model()->validate();
        }

        selection_controller_->render_view()->userinput_update();
    }


    void RectangleController::
            changeEvent ( QEvent * event )
    {
        if (event->type() & QEvent::ParentChange)
        {
            view_->visible = 0!=parent();
        }

        if (event->type() & QEvent::EnabledChange)
        {
            view_->enabled = isEnabled();

            if (!isEnabled())
                emit enabledChanged(isEnabled());
        }
    }


    void RectangleController::
            enableSelectionType(const RectangleModel::RectangleType type, const bool active)
    {
        if (active)
        {
            selection_controller_->setCurrentTool( this, active );
            selection_controller_->setCurrentSelection( model()->updateFilter() );
            model()->type = type;
        }
        else if (model()->type == type)
        {
            //selection_controller_->setCurrentTool( this, active );
        }
    }


    void RectangleController::
            enableRectangleSelection(bool active)
    {
        enableSelectionType(RectangleModel::RectangleType_RectangleSelection, active);
    }


    void RectangleController::
            enableTimeSelection(bool active)
    {
        enableSelectionType(RectangleModel::RectangleType_TimeSelection, active);
    }


    void RectangleController::
            enableFrequencySelection(bool active)
    {
        enableSelectionType(RectangleModel::RectangleType_FrequencySelection, active);
    }

}} // namespace Tools::Selections
