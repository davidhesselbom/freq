#include "cepstrum.h"
#include "stft.h"
#include "stftkernel.h"

#include "signal/buffer.h"

#include "TaskTimer.h"
#include "neat_math.h"

using namespace Signal;
namespace Tfr {


Cepstrum::
        Cepstrum(const CepstrumParams& p)
    :
      p(p)
{
}


pChunk Cepstrum::
        operator()( pMonoBuffer b )
{
    TaskTimer tt("Cepstrum");
    CepstrumParams p2 = p;
    p2.compute_redundant ( true );
    pTransform t = p2.StftParams::createTransform ();
    pChunk cepstra = (*t)(b);

    ::cepstrumPrepareCepstra( cepstra->transform_data, 4.f/p.chunk_size() );

    ((Stft*)t.get())->compute( cepstra->transform_data, cepstra->transform_data, FftDirection_Forward );
    cepstra->freqAxis = p.freqAxis( cepstra->original_sample_rate );

    TaskInfo("Cepstrum debug. Was %s , returned %s ",
        b->getInterval().toString().c_str(),
        cepstra->getInterval().toString().c_str());

    return cepstra;

}


Signal::pMonoBuffer Cepstrum::
        inverse( pChunk )
{
    throw std::logic_error("Not implemented");
}


pTransform CepstrumParams::
        createTransform() const
{
    return pTransform(new Cepstrum(*this));
}


FreqAxis CepstrumParams::
        freqAxis( float FS ) const
{
    FreqAxis fa;
    fa.setQuefrency( FS, chunk_size());
    return fa;
}


} // namespace Tfr
