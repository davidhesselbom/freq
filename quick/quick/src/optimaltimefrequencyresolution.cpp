#include "optimaltimefrequencyresolution.h"
#include "tfr/stftdesc.h"
#include "tfr/waveformrepresentation.h"
#include "log.h"
#include "largememorypool.h"
#include "tfr/cwt.h"
#include "renderviewaxes.h"

//#define LOG_TRANSFORM
#define LOG_TRANSFORM if(0)

OptimalTimeFrequencyResolution::OptimalTimeFrequencyResolution(QQuickItem *parent) :
    QQuickItem(parent)
{
    connect (this, SIGNAL(squircleChanged()), SLOT(onCameraChanged()));
    connect (this, SIGNAL(pausedChanged()), SLOT(onCameraChanged()));
    connect (this, SIGNAL(showAll()), SLOT(onShowAll()));
}


void OptimalTimeFrequencyResolution::
        setSquircle(Squircle* s)
{
    if (squircle_) {
        disconnect (squircle_, SIGNAL(scaleposChanged()), this, SLOT(onCameraChanged()));
        disconnect (squircle_, SIGNAL(timezoomChanged()), this, SLOT(onCameraChanged()));
        disconnect (squircle_, SIGNAL(scalezoomChanged()), this, SLOT(onCameraChanged()));
        disconnect (squircle_, SIGNAL(touchNavigation()), this, SLOT(onCameraChanged()));
    }

    this->squircle_ = s;

    if (!squircle_)
        return;

    connect (squircle_, SIGNAL(scaleposChanged()), this, SLOT(onCameraChanged()));
    connect (squircle_, SIGNAL(timezoomChanged()), this, SLOT(onCameraChanged()));
    connect (squircle_, SIGNAL(scalezoomChanged()), this, SLOT(onCameraChanged()));
    connect (squircle_, SIGNAL(touchNavigation()), this, SLOT(onCameraChanged()));

    connect (squircle_, SIGNAL(chainChanged()), SLOT(onChainChanged()));
    onChainChanged ();
}


void OptimalTimeFrequencyResolution::
        setPaused(bool v)
{
    this->paused_ = v;
    emit pausedChanged ();
}


void OptimalTimeFrequencyResolution::
        onCameraChanged()
{
    if (!squircle_ || paused_)
        return;

    Tools::RenderModel& render_model = *squircle_->renderModel ();
    Tfr::TransformDesc::ptr t = render_model.transform_desc();
    if (!t)
        return; // not initialized

    const auto c = *render_model.camera.read();
    const vectord& q = c.q;

    // match zmin/zmax with TouchNavigation
    auto viewport = render_model.gl_projection.read ()->viewport;
    float aspect = viewport[2]/(float)viewport[3];
    float zmin = std::min(0.5,0.4/(c.zscale/-c.p[2]*aspect));
    zmin = 0; // bah!
    float zmax = 1.0-zmin;
    float zfocus = (q[2]-zmin)/(zmax-zmin);
    if (zmin==zmax) zfocus=0.5;

    auto tm = render_model.tfr_mapping ().read ();
    float ds = 0.01 / c.zscale;
    float fs = tm->targetSampleRate();
    float hz = tm->display_scale().getFrequency(float(zfocus)),
          hz2 = tm->display_scale().getFrequency(float(zfocus < 0.5 ? zfocus + ds : zfocus - ds));
    tm.unlock ();

    // read info about current transform
    float time_res = 1/t->displayedTimeResolution(fs,hz); // samples per 1 time at camera focus
    float s = t->freqAxis(fs).getFrequencyScalarNotClamped(hz);
    float s2 = t->freqAxis(fs).getFrequencyScalarNotClamped(hz2);
    float bin_res = std::fabs (s2-s)/ds; // bins per 1 scale at camera focus

    // compute wanted and current current ratio of time resolution versus frequency resolution
    float ratio = c.zscale/c.xscale; // bins/samples
    float current_ratio = bin_res/time_res;

    Tfr::TransformDesc::ptr newt = t->copy ();
    if (Tfr::StftDesc* stft = dynamic_cast<Tfr::StftDesc*>(newt.get ()))
    {
        int w = stft->chunk_size ();
        float k = std::sqrt(ratio/current_ratio);
        stft->set_approximate_chunk_size(w*k);
    }
//        else if (Tfr::Cwt* cwt = dynamic_cast<Tfr::Cwt*>(newt.get ()))
//        {
//            float s = stft->scales_per_octave ();
//            float k = std::sqrt(ratio/current_ratio);
//            cwt->scales_per_octave(s/k);
//            if (*newt != *t)
//                render_model()->set_transform_desc (newt);
//        }
    else if (dynamic_cast<Tfr::WaveformRepresentationDesc*>(newt.get ()))
    {
        // do nothing
    }
    else if (Tfr::Cwt* cwt = dynamic_cast<Tfr::Cwt*>(newt.get ()))
    {
        float spo = cwt->scales_per_octave ();
        float k = std::sqrt(ratio/current_ratio);
        cwt->scales_per_octave (std::min(40.f,spo*k), fs);
    }
    else
    {
        Log("OptimalTimeFrequencyResolution: unknown transform");
    }

    if (*newt != *t)
    {
        LOG_TRANSFORM Log("OptimalTimeFrequencyResolution: %s") % newt->toString ();
        render_model.set_transform_desc (newt);
        emit squircle_->displayedTransformDetailsChanged();

        lmp_gc ();
    }
}


void OptimalTimeFrequencyResolution::
        onChainChanged()
{
    if (squircle_ && squircle_->chain ())
        connect(squircle_->chain (), SIGNAL(titleChanged()), SLOT(onTitleChanged()));
    onTitleChanged ();
}


void OptimalTimeFrequencyResolution::
        onTitleChanged()
{
    if (!squircle_)
        return;

    auto rm = squircle_->renderModel ();
    rm->recompute_extent ();

    RenderViewAxes(*rm).cameraOnFront();

    onShowAll();
}


void OptimalTimeFrequencyResolution::
        onShowAll()
{
    if (!squircle_)
        return;

    auto rm = squircle_->renderModel ();
    auto viewport = rm->gl_projection.read ()->viewport;
    float aspect = viewport[2]/(float)viewport[3];

    double L = rm->tfr_mapping ().read ()->length();

    if (L)
    {
        const auto c = *rm->camera.read();
        squircle_->setTimezoom (-c.p[2]/L*0.8);
        squircle_->setScalezoom (-c.p[2]/aspect*0.8);
        squircle_->setTimepos (0.5*L);
        squircle_->setScalepos (0.5);

        emit updateSharedCamera();
        // this whole procedure really doesn't belong in this class
    }

    onCameraChanged();
}
