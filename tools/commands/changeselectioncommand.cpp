#include "changeselectioncommand.h"
#include "tools/selectioncontroller.h"

#include "sawe/project.h"

namespace Tools {
namespace Commands {

ChangeSelectionCommand::ChangeSelectionCommand(Tools::SelectionController*p, Signal::pOperation s)
    :
    s(s),
    p(p)
{
}


void ChangeSelectionCommand::
        execute()
{
    // sawe previous state
    prevSelection = p->model()->current_selection_copy();

    // set new selection
    p->setCurrentSelectionCommand( s );
}


void ChangeSelectionCommand::
        undo()
{
    // restore previous state
    p->setCurrentSelectionCommand( prevSelection );
}


std::string ChangeSelectionCommand::
        toString()
{
    return s ? s->name() : "No selection";
}


} // namespace Commands
} // namespace Tools
