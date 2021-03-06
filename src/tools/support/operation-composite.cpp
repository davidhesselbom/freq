#include "operation-composite.h"

#include "signal/operation-basic.h"
#include "filters/move.h"
#include "filters/ellipse.h"
#include "demangle.h"
#include "signal/computingengine.h"

using namespace Signal;

namespace Tools {
    namespace Support {

    // OperationCrop  /////////////////////////////////////////////////////////////////

OperationCrop::
        OperationCrop( const Signal::Interval& section )
    :
      OperationOtherSilent(section)
{

}

OperationCrop::Extent OperationCrop::
        extent() const
{
    Extent x;
    x.interval = section();
    return x;
}

QString OperationCrop::
        toString() const
{
    std::stringstream ss;
    ss << "Crop " << section();
    return ss.str().c_str ();
}

void OperationCrop::
        test()
{
    /**
      Example 1:
      start:  1234567
      OperationCrop( start, 1, 2 );
      result: 23
    */
    {
        Interval I(5,7);
        OperationCrop oc(I);
        EXCEPTION_ASSERT_EQUALS( oc.extent ().interval.get (), I );
    }
}

    // OperationOtherSilent  /////////////////////////////////////////////////////////////////



OperationOtherSilent::Operation::
        Operation(const Interval &section)
    :
      section_(section)
{}


pBuffer OperationOtherSilent::Operation::
        process (pBuffer b)
{
    Signal::Intervals I = b->getInterval ();
    I -= section_;

    foreach (Signal::Interval i, I) {
        Buffer zero(i, b->sample_rate(), b->number_of_channels ());
        *b |= zero;
    }

    return b;
}


OperationOtherSilent ::OperationOtherSilent(const Interval &section)
    :
      section_(section)
{
}


Interval OperationOtherSilent::
        requiredInterval( const Interval& I, Interval* expectedOutput ) const
{
    if (expectedOutput)
        *expectedOutput = I;
    return I;
}


Interval OperationOtherSilent::
        affectedInterval( const Interval& I ) const
{
    return I;
}


OperationDesc::ptr OperationOtherSilent::
        copy() const
{
    return OperationDesc::ptr(new OperationOtherSilent(section_));
}


Signal::Operation::ptr OperationOtherSilent::
        createOperation(ComputingEngine* engine) const
{
    if (0==engine || dynamic_cast<Signal::ComputingCpu*>(engine))
        return Signal::Operation::ptr(new OperationOtherSilent::Operation(section_));
    return Signal::Operation::ptr();
}


void OperationOtherSilent::
        test()
{
    /**
      Example 1:
      start:  1234567
      OperationOtherSilent( start, 1, 2 );
      result: 0230000
    */
    {
        EXCEPTION_ASSERTX(false, "not implemented");
    }
}

    // OperationMove  /////////////////////////////////////////////////////////////////
#if 0 // TODO implement using branching in the Dag

OperationMove::
        OperationMove( pOperation source, const Signal::Interval& section, unsigned newFirstSample )
:   OperationSubOperations( source, "Move" )
{
    reset(section, newFirstSample);
}

void OperationMove::
        reset( const Signal::Interval& section, unsigned newFirstSample )
{
    // Note: difference to OperationMoveMerge is that OperationMove has the silenceTarget step

    Intervals newSection = section;
    if (newFirstSample<section.first)
        newSection >>= (section.first-newFirstSample);
    else
        newSection <<= (newFirstSample-section.first);

    pOperation silenceTarget( new OperationSetSilent(source_sub_operation_, newSection.spannedInterval() ));
    pOperation silence( new OperationSetSilent(silenceTarget, section ));

    pOperation crop( new OperationCrop( source_sub_operation_, section ));
    pOperation moveToNewPos( new OperationInsertSilence( crop, Interval(0, newFirstSample)));

    pOperation addition( new OperationSuperposition (moveToNewPos, silence ));

    DeprecatedOperation::source( addition );
}
#endif

    // OperationMoveMerge  /////////////////////////////////////////////////////////////////
#if 0 // TODO implement using branching in the Dag

OperationMoveMerge::
        OperationMoveMerge( pOperation source, const Signal::Interval& section, unsigned newFirstSample )
:   OperationSubOperations( source, "Move and merge" )
{
    reset(section, newFirstSample);
}

void OperationMoveMerge::
        reset( const Signal::Interval& section, unsigned newFirstSample )
{
    pOperation silence( new OperationSetSilent (source_sub_operation_, section ));

    pOperation crop( new OperationCrop( source_sub_operation_, section ));
    pOperation moveToNewPos( new OperationInsertSilence( crop, Interval(0, newFirstSample)));

    pOperation addition( new OperationSuperposition (moveToNewPos, silence ));

    DeprecatedOperation::source( addition );
}

#endif
    // OperationShift  /////////////////////////////////////////////////////////////////

class OperationShiftOperation: public Signal::Operation
{
public:
    OperationShiftOperation( Signal::IntervalType sampleShift )
        :
          sampleShift_(sampleShift)
    {

    }


    Signal::pBuffer process(Signal::pBuffer b)
    {
        UnsignedF o = b->sample_offset () + (long long)sampleShift_;
        b->set_sample_offset (o);
        return b;
    }

private:
    Signal::IntervalType sampleShift_;
};

OperationShift::
        OperationShift( Signal::IntervalType sampleShift, Signal::Interval extent_interval )
    :
      sampleShift_(sampleShift),
      extent_interval_(extent_interval)
{
}

Signal::Interval OperationShift::
        requiredInterval( const Signal::Interval& I, Signal::Interval* expectedOutput ) const
{
    if (expectedOutput)
        *expectedOutput = I;

    return (Signal::Intervals(I) >>= sampleShift_).spannedInterval ();
}

Signal::Interval OperationShift::
        affectedInterval( const Signal::Interval& I ) const
{
    return (Signal::Intervals(I) <<= sampleShift_).spannedInterval ();
}

Signal::OperationDesc::ptr OperationShift::
        copy() const
{
    return Signal::OperationDesc::ptr(new OperationShift(sampleShift_, extent_interval_));
}

Signal::Operation::ptr OperationShift::
        createOperation(Signal::ComputingEngine* engine) const
{
    if (0==engine || dynamic_cast<Signal::ComputingCpu*>(engine))
        return Signal::Operation::ptr(new OperationShiftOperation(sampleShift_));
    return Signal::Operation::ptr();
}

OperationShift::Extent OperationShift::
        extent() const
{
    OperationShift::Extent x;
    x.interval = extent_interval_;
    return x;
}


#ifdef USE_CUDA
    // OperationMoveSelection  /////////////////////////////////////////////////////////////////

OperationMoveSelection::
        OperationMoveSelection( pOperation source, pOperation selectionFilter, long sampleShift, float freqDelta )
:	OperationSubOperations( source, "OperationMoveSelection" )
{
	reset(selectionFilter, sampleShift, freqDelta );
}


void OperationMoveSelection::
    reset( pOperation selectionFilter, long sampleShift, float freqDelta )
{
    // Take out the samples affected by selectionFilter and move them
    // 'sampleShift' in time and 'freqDelta' in frequency

    pOperation  extract, remove;
    if (Filters::Ellipse* f = dynamic_cast<Filters::Ellipse*>(selectionFilter.get())) {

        // Create filter for extracting selection
        extract.reset( new Filters::Ellipse(*f) );
        dynamic_cast<Filters::Ellipse*>(extract.get())->_save_inside = true;
        extract->source( source() );

        // Create filter for removing selection
        remove.reset( new Filters::Ellipse(*f) );
        dynamic_cast<Filters::Ellipse*>(remove.get())->_save_inside = false;
        remove->source( source() );

	} else {
		throw std::invalid_argument(std::string(__FUNCTION__) + " only supports Tfr::EllipseFilter as selectionFilter");
	}

    pOperation extractAndMove = extract;
    {
        // Create operation for moving extracted selection in time
        if (0!=sampleShift)
        extractAndMove.reset( new OperationShift( extractAndMove, sampleShift ));

        // Create operation for moving extracted selection in frequency
        if (0!=freqDelta)
        {
            pOperation t( new Filters::Move( freqDelta ));
            t->source( extractAndMove );
            extractAndMove = t;
        }

	}

    pOperation mergeSelection( new OperationSuperposition( remove, extractAndMove ));

    Operation::source( mergeSelection );
}
#endif



    // OperationFilterSelection  /////////////////////////////////////////////////////////////////

#if 0 // TODO implement using branching in the Dag because this operation involves a merge of two or more different signals
OperationOnSelection::
        OperationOnSelection( pOperation source, pOperation insideSelection, pOperation outsideSelection, Signal::pOperation operation )
:   OperationSubOperations( source, "OperationOnSelection" )
{
    reset( insideSelection, outsideSelection, operation );
}


std::string OperationOnSelection::
        name()
{
    return (operation_?operation_->name():"(null)") + " in " + (insideSelection_?insideSelection_->name():"(null)");
}


void OperationOnSelection::
        reset( pOperation insideSelection, pOperation outsideSelection, Signal::pOperation operation )
{
    EXCEPTION_ASSERT(insideSelection);
    EXCEPTION_ASSERT(outsideSelection);
    EXCEPTION_ASSERT(operation);

    insideSelection_ = insideSelection;
    operation_ = operation;

    // Take out the samples affected by selectionFilter

    outsideSelection->source( source_sub_operation_ );
    insideSelection->source( source_sub_operation_ );
    operation->source( insideSelection );

    pOperation mergeSelection( new OperationSuperposition( operation, outsideSelection ));

    // Makes reads read from 'mergeSelection'
    DeprecatedOperation::source( mergeSelection );
}
#endif

    } // namespace Support
} // namespace Tools
