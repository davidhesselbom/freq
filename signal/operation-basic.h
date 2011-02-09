#ifndef SIGNALOPERATIONBASIC_H
#define SIGNALOPERATIONBASIC_H

#include "signal/operation.h"

namespace Signal {

class OperationRemoveSection: public Operation
{
public:
    OperationRemoveSection( pOperation source, Interval section );

    virtual pBuffer read( const Interval& I );
    virtual IntervalType number_of_samples();

    virtual Intervals affected_samples();
    virtual Intervals translate_interval(Intervals I);
    virtual Intervals translate_interval_inverse(Intervals I);

private:

    Interval section_;
};

/**
  Has no effect as long as source()->number_of_samples <= section.first.
  */
class OperationInsertSilence: public Operation
{
public:
    OperationInsertSilence( pOperation source, Interval section );

    virtual pBuffer read( const Interval& I );
    virtual IntervalType number_of_samples();

    virtual Intervals affected_samples();
    virtual Intervals translate_interval(Intervals I);
    virtual Intervals translate_interval_inverse(Intervals I);
private:
    Interval section_;
};

class OperationSuperposition: public Operation
{
public:
    OperationSuperposition( pOperation source, pOperation source2 );

    virtual pBuffer read( const Interval& I );

    virtual pOperation source2() const { return _source2; }

private:
    pOperation _source2;
};

} // namespace Signal

#endif // SIGNALOPERATIONBASIC_H