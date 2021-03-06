#ifndef TOOLS_SUPPORT_RENDEROPERATION_H
#define TOOLS_SUPPORT_RENDEROPERATION_H

#include "signal/operationwrapper.h"
#include "tfr/transform.h"

#include "shared_state.h"

namespace Tools {
namespace Support {

/**
 * @brief The RenderOperationDesc class should keep the filter operation used by a
 * render target and notify that render target about processing events.
 */
class RenderOperationDesc : public Signal::OperationDescWrapper
{
public:
    class RenderTarget
    {
    public:
        typedef shared_state<RenderTarget> ptr;

        virtual ~RenderTarget() {}

        /**
         * @brief refreshSamples is called when samples are about to be recomputed.
         * @param I the samples that will be recomputed.
         */
        virtual void refreshSamples(const Signal::Intervals& I) = 0;

        /**
         * @brief processedData is called whenever new samples have been processed.
         * @param input extent of the buffer that was sent to process.
         * @param output extent of the buffer that is returned from process.
         */
        virtual void processedData(const Signal::Interval& input, const Signal::Interval& output) = 0;
    };


    RenderOperationDesc(Signal::OperationDesc::ptr embed, RenderTarget::ptr render_target);

    // Signal::OperationDesc
    Signal::Operation::ptr      createOperation( Signal::ComputingEngine* engine ) const;
    Signal::Interval            affectedInterval( const Signal::Interval& I ) const;


    Tfr::TransformDesc::ptr     transform_desc() const;
    void                        transform_desc(Tfr::TransformDesc::ptr);

private:
    class Operation : public Signal::Operation
    {
    public:
        Operation(Signal::Operation::ptr wrapped, RenderTarget::ptr render_target);

        Signal::pBuffer process(Signal::pBuffer b);

    private:
        Operation::ptr wrapped_;
        RenderTarget::ptr render_target_;
    };

    RenderTarget::ptr render_target_;

public:
    static void test();
};

} // namespace Support
} // namespace Tools

#endif // TOOLS_SUPPORT_RENDEROPERATION_H
