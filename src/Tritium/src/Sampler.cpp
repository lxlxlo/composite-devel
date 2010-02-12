/*
 * Copyright(c) 2002-2008 by Alex >Comix< Cominu [comix@users.sourceforge.net]
 *
 * This file is part of Tritium
 *
 * Tritium is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Tritium is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY, without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <cassert>
#include <cmath>
#include <list>
#include <QMutexLocker>

#include "SamplerPrivate.hpp"

#include <Tritium/IO/AudioOutput.hpp>
#include <Tritium/IO/JackOutput.hpp>

#include <Tritium/ADSR.hpp>
#include <Tritium/DataPath.hpp>
#include <Tritium/globals.hpp>
#include <Tritium/Engine.hpp>
#include <Tritium/Instrument.hpp>
#include <Tritium/InstrumentLayer.hpp>
#include <Tritium/Note.hpp>
#include <Tritium/Sample.hpp>
#include <Tritium/SeqScriptIterator.hpp>
#include <Tritium/Logger.hpp>

#include <Tritium/fx/Effects.hpp>
#include <Tritium/Sampler.hpp>
#include <Tritium/TransportPosition.hpp>
#include <Tritium/memory.hpp>

using namespace Tritium;

inline static float linear_interpolation( float fVal_A, float fVal_B, float fVal )
{
    return fVal_A * ( 1 - fVal ) + fVal_B * fVal;
//	return fVal_A + fVal * ( fVal_B - fVal_A );
//	return fVal_A + ((fVal_B - fVal_A) * fVal);
}

void SamplerPrivate::handle_event(const SeqEvent& ev)
{
    // TODO: If we receive a note that we don't have an instrument
    // for, we still try to process it.  The results in a
    // segfault.  Need to filter these out.  See handle_note_on()
    switch(ev.type) {
    case SeqEvent::NOTE_ON:
	handle_note_on(ev);
	break;
    case SeqEvent::NOTE_OFF:
	handle_note_off(ev);
	break;
    case SeqEvent::ALL_OFF:
	panic();
	break;
    }
}

void SamplerPrivate::panic()
{
    parent.stop_playing_notes();
}

void SamplerPrivate::handle_note_on(const SeqEvent& ev)
{
    // Respect the mute groups.
    T<Instrument>::shared_ptr pInstr = ev.note.get_instrument(); // TODO: May return invalid note
    if ( pInstr->get_mute_group() != -1 ) {
	// remove all notes using the same mute group
	NoteList::iterator j, prev;
	T<Instrument>::shared_ptr otherInst;
	for ( j = current_notes.begin() ; j != current_notes.end() ; ++j ) {
	    otherInst = j->get_instrument();
	    if( (otherInst != pInstr)
		&& (otherInst->get_mute_group() == pInstr->get_mute_group())) {
		j->m_adsr.release();
	    }
	}
    }
    pInstr->enqueue();
    QMutexLocker lk( &mutex_current_notes );
    current_notes.push_back( ev.note );
    current_notes.back().m_nSilenceOffset = ev.frame;
    current_notes.back().m_nReleaseOffset = (uint32_t)-1;
}

void SamplerPrivate::handle_note_off(const SeqEvent& ev)
{
    NoteList::iterator k;
    for( k=current_notes.begin() ; k!=current_notes.end() ; ++k ) {
	if( k->get_instrument() == ev.note.get_instrument() ) {
	    k->m_nReleaseOffset = ev.frame;
	}
    }
}

Sampler::Sampler(Engine* parent, T<AudioPortManager>::shared_ptr apm)
{
    INFOLOG( "INIT" );
    assert(parent);

    d = new SamplerPrivate(this, parent, apm);

    // instrument used in file preview
    QString sEmptySampleFilename = DataPath::get_data_path() + "/emptySample.wav";
    d->preview_instrument.reset( new Instrument( sEmptySampleFilename, "preview", new ADSR() ) );
    d->preview_instrument->set_volume( 0.8 );
    d->preview_instrument->set_layer( new InstrumentLayer( Sample::load( sEmptySampleFilename ) ), 0 );
}



Sampler::~Sampler()
{
    delete d;
}

void Sampler::panic()
{
    d->panic();
}

int Sampler::get_playing_notes_number()
{
    return d->current_notes.size();
}

// Do not use B:b.t or frame info from pos.
// This param may be replaced with 'frame_rate' instead.
void Sampler::process( SeqScriptConstIterator beg,
		       SeqScriptConstIterator end,
		       const TransportPosition& pos,
		       uint32_t nFrames )
{
    d->main_out->write_zeros(nFrames);
    d->main_out->set_zero_flag(false);

    if(d->per_instrument_outs) {
	for(int k=0 ; k<MAX_INSTRUMENTS ; ++k) {
	    d->track_out[k]->set_zero_flag(true);
	}
    }

    // Max notes limit
    // If max_notes == -1, this means "unlimited"
    if ( d->current_notes.size() > (unsigned)d->max_notes ) {
	QMutexLocker lk( &d->mutex_current_notes );
	while( d->current_notes.size() > (unsigned)d->max_notes) {
	    assert(d->max_notes >= 0);
	    d->current_notes.front().get_instrument()->dequeue();
	    d->current_notes.pop_front();
	}
    }

    // Handle new events from the sequencer (add/remove notes from the "currently playing"
    // list.
    SeqScriptConstIterator ev;
    for( ev = beg ; ev != end ; ++ev ) {
	d->handle_event(*ev);
    }

    // Play all of the currently playing notes.
    SamplerPrivate::NoteList::iterator k, die;
    QMutexLocker lk( &d->mutex_current_notes );
    for( k=d->current_notes.begin() ; k != d->current_notes.end() ; /*++k*/ ) {
	unsigned res = d->render_note( *k, nFrames, pos.frame_rate );
	if( res == 1 ) { // Note is finished playing
	    die = k;  ++k;
	    die->get_instrument()->dequeue();
	    d->current_notes.erase(die);
	} else {
	    ++k;
	}
    }
}

/// Render a note
/// Return 0: the note is not ended
/// Return 1: the note is ended
int SamplerPrivate::render_note( Note& note, uint32_t nFrames, uint32_t frame_rate )
{
    //infoLog( "[renderNote] instr: " + note.getInstrument()->m_sName );

    T<Instrument>::shared_ptr pInstr = note.get_instrument();
    if ( !pInstr ) {
	ERRORLOG( "NULL instrument" );
	return 1;
    }

    float fLayerGain = 1.0;
    float fLayerPitch = 0.0;

    // scelgo il sample da usare in base alla velocity
    T<Sample>::shared_ptr pSample;
    for ( unsigned nLayer = 0; nLayer < MAX_LAYERS; ++nLayer ) {
	InstrumentLayer *pLayer = pInstr->get_layer( nLayer );
	if ( pLayer == NULL ) continue;

	if ( pLayer->in_velocity_range(note.get_velocity()) ) {
	    pSample = pLayer->get_sample();
	    fLayerGain = pLayer->get_gain();
	    fLayerPitch = pLayer->get_pitch();
	    break;
	}
    }
    if ( !pSample ) {
	QString dummy = QString( "NULL sample for instrument %1. Note velocity: %2" )
	    .arg( pInstr->get_name() )
	    .arg( note.get_velocity() );
	WARNINGLOG( dummy );
	return 1;
    }

    if ( note.m_fSamplePosition >= pSample->get_n_frames() ) {
	WARNINGLOG( "sample position out of bounds. The layer has been resized during note play?" );
	return 1;
    }

    float cost_L = 1.0f;
    float cost_R = 1.0f;
    float cost_track_L = 1.0f;
    float cost_track_R = 1.0f;
    float fSendFXLevel_L = 1.0f;
    float fSendFXLevel_R = 1.0f;

    if ( pInstr->is_muted() ) {                             // is instrument muted?
	cost_L = 0.0;
	cost_R = 0.0;
	if ( ! instrument_outs_prefader ) {
	    // Post-Fader
	    cost_track_L = 0.0;
	    cost_track_R = 0.0;
	}

	fSendFXLevel_L = 0.0f;
	fSendFXLevel_R = 0.0f;
    } else {	// Precompute some values...
	cost_L = cost_L * note.get_velocity();		// note velocity
	cost_L = cost_L * note.get_pan_l();		// note pan
	cost_L = cost_L * fLayerGain;				// layer gain
	cost_L = cost_L * pInstr->get_pan_l();		// instrument pan
	cost_L = cost_L * pInstr->get_gain();		// instrument gain
	fSendFXLevel_L = cost_L;

	if ( instrument_outs_prefader ) {
	    // Post-Fader
	    cost_track_L = cost_L * 2;
	}
	cost_L = cost_L * pInstr->get_volume();		// instrument volume
#warning "WTF is song volume???"
	/*
	  cost_L = cost_L * pSong->get_volume();	// song volume
	*/
	cost_L = cost_L * 2; // max pan is 0.5


	cost_R = cost_R * note.get_velocity();		// note velocity
	cost_R = cost_R * note.get_pan_r();		// note pan
	cost_R = cost_R * fLayerGain;				// layer gain
	cost_R = cost_R * pInstr->get_pan_r();		// instrument pan
	cost_R = cost_R * pInstr->get_gain();		// instrument gain
	fSendFXLevel_R = cost_R;

	if ( instrument_outs_prefader ) {
	    // Post-Fader
	    cost_track_R = cost_R * 2;
	}
	cost_R = cost_R * pInstr->get_volume();		// instrument volume
#warning "WTF is song volume???"
	/*
	  cost_R = cost_R * pSong->get_volume();	// song pan
	*/
	cost_R = cost_R * 2; // max pan is 0.5
    }

    if ( ! instrument_outs_prefader ) {
	cost_track_L = cost_L;
	cost_track_R = cost_R;
    }

    // Se non devo fare resample (drumkit) posso evitare di utilizzare i float e gestire il tutto in
    // maniera ottimizzata
    //	constant^12 = 2, so constant = 2^(1/12) = 1.059463.
    //	float nStep = 1.0;1.0594630943593

    float fTotalPitch = note.m_noteKey.m_nOctave * 12 + note.m_noteKey.m_key;
    fTotalPitch += note.get_pitch();
    fTotalPitch += fLayerPitch;

    //INFOLOG( "total pitch: " + to_string( fTotalPitch ) );

    if ( fTotalPitch == 0.0
	 && pSample->get_sample_rate() == frame_rate ) {
	// NO RESAMPLE
	return render_note_no_resample(
	    pSample,
	    note,
	    nFrames,
	    cost_L,
	    cost_R,
	    cost_track_L,
	    cost_track_R,
	    fSendFXLevel_L,
	    fSendFXLevel_R
	    );
    } else {
	// RESAMPLE
	return render_note_resample(
	    pSample,
	    note,
	    nFrames,
	    frame_rate,
	    cost_L,
	    cost_R,
	    cost_track_L,
	    cost_track_R,
	    fLayerPitch,
	    fSendFXLevel_L,
	    fSendFXLevel_R
	    );
    }
} // SamplerPrivate::render_note()




int SamplerPrivate::render_note_no_resample(
    T<Sample>::shared_ptr pSample,
    Note& note,
    int nFrames,
    float cost_L,
    float cost_R,
    float cost_track_L,
    float cost_track_R,
    float fSendFXLevel_L,
    float fSendFXLevel_R
    )
{
    int retValue = 1; // the note is ended

    int nAvail_bytes = pSample->get_n_frames() - ( int )note.m_fSamplePosition;   // verifico 

    if ( nAvail_bytes > nFrames - note.m_nSilenceOffset ) {   // il sample e' piu' grande del buff
	// imposto il numero dei bytes disponibili uguale al buffersize
	nAvail_bytes = nFrames - note.m_nSilenceOffset;
	retValue = 0; // the note is not ended yet
    }
	
    //ADSR *pADSR = note.m_pADSR;

    int nInitialBufferPos = note.m_nSilenceOffset;
    int nInitialSamplePos = ( int )note.m_fSamplePosition;
    int nSamplePos = nInitialSamplePos;
    int nTimes = nInitialBufferPos + nAvail_bytes;
    int nInstrument = instrument_list->get_pos( note.get_instrument() );

    // filter
    bool bUseLPF = note.get_instrument()->is_filter_active();
    float fResonance = note.get_instrument()->get_filter_resonance();
    float fCutoff = note.get_instrument()->get_filter_cutoff();

    float *pSample_data_L = pSample->get_data_l();
    float *pSample_data_R = pSample->get_data_r();

    float fInstrPeak_L = note.get_instrument()->get_peak_l(); // this value will be reset to 0 by the mixer..
    float fInstrPeak_R = note.get_instrument()->get_peak_r(); // this value will be reset to 0 by the mixer..

    float fADSRValue;
    float fVal_L;
    float fVal_R;

    /*
     * nInstrument could be -1 if the instrument is not found in the current drumset.
     * This happens when someone is using the prelistening function of the soundlibrary.
     */

    if( nInstrument < 0 ) {
	nInstrument = 0;
    }

    float *buf_track_L = 0;
    float *buf_track_R = 0;
    if(per_instrument_outs) {
	assert( track_out[nInstrument]->size() >= nFrames );
	buf_track_L = track_out[nInstrument]->get_buffer(0);
	buf_track_R = track_out[nInstrument]->get_buffer(1);
    }

    float *buf_L = main_out->get_buffer(0);
    float *buf_R = main_out->get_buffer(1);
    for ( int nBufferPos = nInitialBufferPos; nBufferPos < nTimes; ++nBufferPos ) {
	if( note.m_nReleaseOffset != (uint32_t)-1
	    && nBufferPos >= note.m_nReleaseOffset ) {
	    if ( note.m_adsr.release() == 0 ) {
		retValue = 1;	// the note is ended
	    }
	}

	fADSRValue = note.m_adsr.get_value( 1 );
	fVal_L = pSample_data_L[ nSamplePos ] * fADSRValue;
	fVal_R = pSample_data_R[ nSamplePos ] * fADSRValue;

	// Low pass resonant filter
	if ( bUseLPF ) {
	    note.m_fBandPassFilterBuffer_L = fResonance * note.m_fBandPassFilterBuffer_L + fCutoff * ( fVal_L - note.m_fLowPassFilterBuffer_L );
	    note.m_fLowPassFilterBuffer_L += fCutoff * note.m_fBandPassFilterBuffer_L;
	    fVal_L = note.m_fLowPassFilterBuffer_L;

	    note.m_fBandPassFilterBuffer_R = fResonance * note.m_fBandPassFilterBuffer_R + fCutoff * ( fVal_R - note.m_fLowPassFilterBuffer_R );
	    note.m_fLowPassFilterBuffer_R += fCutoff * note.m_fBandPassFilterBuffer_R;
	    fVal_R = note.m_fLowPassFilterBuffer_R;
	}

#ifdef JACK_SUPPORT
	if( buf_track_L ) {
	    buf_track_L[nBufferPos] += fVal_L * cost_track_L;
	}
	if( buf_track_R ) {
	    buf_track_R[nBufferPos] += fVal_R * cost_track_R;
	}
#endif

	fVal_L = fVal_L * cost_L;
	fVal_R = fVal_R * cost_R;

	// update instr peak
	if ( fVal_L > fInstrPeak_L ) {
	    fInstrPeak_L = fVal_L;
	}
	if ( fVal_R > fInstrPeak_R ) {
	    fInstrPeak_R = fVal_R;
	}

	// to main mix
	buf_L[nBufferPos] += fVal_L;
	buf_R[nBufferPos] += fVal_R;

	++nSamplePos;
    }
    note.m_fSamplePosition += nAvail_bytes;
    note.m_nSilenceOffset = 0;
    note.get_instrument()->set_peak_l( fInstrPeak_L );
    note.get_instrument()->set_peak_r( fInstrPeak_R );


#ifdef LADSPA_SUPPORT
    // LADSPA
    for ( unsigned nFX = 0; nFX < MAX_FX; ++nFX ) {
	T<LadspaFX>::shared_ptr pFX = engine->get_effects()->getLadspaFX( nFX );

	float fLevel = note.get_instrument()->get_fx_level( nFX );

	if ( ( pFX ) && ( fLevel != 0.0 ) ) {
	    fLevel = fLevel * pFX->getVolume();
	    float *pBuf_L = pFX->m_pBuffer_L;
	    float *pBuf_R = pFX->m_pBuffer_R;

//			float fFXCost_L = cost_L * fLevel;
//			float fFXCost_R = cost_R * fLevel;
	    float fFXCost_L = fLevel * fSendFXLevel_L;
	    float fFXCost_R = fLevel * fSendFXLevel_R;

	    int nBufferPos = nInitialBufferPos;
	    int nSamplePos = nInitialSamplePos;
	    for ( int i = 0; i < nAvail_bytes; ++i ) {
		pBuf_L[ nBufferPos ] += pSample_data_L[ nSamplePos ] * fFXCost_L * cost_L;
		pBuf_R[ nBufferPos ] += pSample_data_R[ nSamplePos ] * fFXCost_R * cost_R;
		++nSamplePos;
		++nBufferPos;
	    }
	}
    }
    // ~LADSPA
#endif

    return retValue;
}



int SamplerPrivate::render_note_resample(
    T<Sample>::shared_ptr pSample,
    Note& note,
    int nFrames,
    uint32_t frame_rate,
    float cost_L,
    float cost_R,
    float cost_track_L,
    float cost_track_R,
    float fLayerPitch,
    float fSendFXLevel_L,
    float fSendFXLevel_R
    )
{
    float fNotePitch = note.get_pitch() + fLayerPitch;
    fNotePitch += note.m_noteKey.m_nOctave * 12 + note.m_noteKey.m_key;

    //INFOLOG( "pitch: " + to_string( fNotePitch ) );

    // 2^(1/12) is a musical half-step in pitch.  If A=440, A#=440 * 2^1/12
    float fStep = pow( 1.0594630943593, ( double )fNotePitch );  // i.e. pow( 2, fNotePitch/12.0 )
    fStep *= ( float )pSample->get_sample_rate() / frame_rate; // Adjust for audio driver sample rate

    int nAvail_bytes = ( int )( ( float )( pSample->get_n_frames() - note.m_fSamplePosition ) / fStep );	// verifico il numero di frame disponibili ancora da eseguire

    int retValue = 1; // the note is ended
    if ( nAvail_bytes > nFrames - note.m_nSilenceOffset ) {	// il sample e' piu' grande del buffersize
	// imposto il numero dei bytes disponibili uguale al buffersize
	nAvail_bytes = nFrames - note.m_nSilenceOffset;
	retValue = 0; // the note is not ended yet
    }

//	ADSR *pADSR = note.m_pADSR;

    int nInitialBufferPos = note.m_nSilenceOffset;
    float fInitialSamplePos = note.m_fSamplePosition;
    float fSamplePos = note.m_fSamplePosition;
    int nTimes = nInitialBufferPos + nAvail_bytes;
    int nInstrument = instrument_list->get_pos( note.get_instrument() );

    // filter
    bool bUseLPF = note.get_instrument()->is_filter_active();
    float fResonance = note.get_instrument()->get_filter_resonance();
    float fCutoff = note.get_instrument()->get_filter_cutoff();

    float *pSample_data_L = pSample->get_data_l();
    float *pSample_data_R = pSample->get_data_r();

    float fInstrPeak_L = note.get_instrument()->get_peak_l(); // this value will be reset to 0 by the mixer..
    float fInstrPeak_R = note.get_instrument()->get_peak_r(); // this value will be reset to 0 by the mixer..

    float fADSRValue = 1.0;
    float fVal_L;
    float fVal_R;
    int nSampleFrames = pSample->get_n_frames();

    /*
     * nInstrument could be -1 if the instrument is not found in the current drumset.
     * This happens when someone is using the prelistening function of the soundlibrary.
     */

    if( nInstrument < 0 ) {
	nInstrument = 0;
    }

    float *buf_track_L = 0;
    float *buf_track_R = 0;
    if(per_instrument_outs) {
	assert( track_out[nInstrument]->size() >= nFrames );
	buf_track_L = track_out[nInstrument]->get_buffer(0);
	buf_track_R = track_out[nInstrument]->get_buffer(1);
    }

    float *buf_L = main_out->get_buffer(0);
    float *buf_R = main_out->get_buffer(1);
    for ( int nBufferPos = nInitialBufferPos; nBufferPos < nTimes; ++nBufferPos ) {
	if( note.m_nReleaseOffset != (uint32_t)-1
	    && nBufferPos >= note.m_nReleaseOffset )
	{
	    if ( note.m_adsr.release() == 0 ) {
		retValue = 1;	// the note is ended
	    }
	}

	int nSamplePos = ( int )fSamplePos;
	double fDiff = fSamplePos - nSamplePos;
	if ( ( nSamplePos + 1 ) >= nSampleFrames ) {
	    fVal_L = linear_interpolation( pSample_data_L[ nSampleFrames ], 0, fDiff );
	    fVal_R = linear_interpolation( pSample_data_R[ nSampleFrames ], 0, fDiff );
	} else {
	    fVal_L = linear_interpolation( pSample_data_L[nSamplePos], pSample_data_L[nSamplePos + 1], fDiff );
	    fVal_R = linear_interpolation( pSample_data_R[nSamplePos], pSample_data_R[nSamplePos + 1], fDiff );
	}

	// ADSR envelope
	fADSRValue = note.m_adsr.get_value( fStep );
	fVal_L = fVal_L * fADSRValue;
	fVal_R = fVal_R * fADSRValue;

	// Low pass resonant filter
	if ( bUseLPF ) {
	    note.m_fBandPassFilterBuffer_L = fResonance * note.m_fBandPassFilterBuffer_L + fCutoff * ( fVal_L - note.m_fLowPassFilterBuffer_L );
	    note.m_fLowPassFilterBuffer_L += fCutoff * note.m_fBandPassFilterBuffer_L;
	    fVal_L = note.m_fLowPassFilterBuffer_L;

	    note.m_fBandPassFilterBuffer_R = fResonance * note.m_fBandPassFilterBuffer_R + fCutoff * ( fVal_R - note.m_fLowPassFilterBuffer_R );
	    note.m_fLowPassFilterBuffer_R += fCutoff * note.m_fBandPassFilterBuffer_R;
	    fVal_R = note.m_fLowPassFilterBuffer_R;
	}


#ifdef JACK_SUPPORT
	if( buf_track_L ) {
	    buf_track_L[nBufferPos] += (fVal_L * cost_track_L);
	}
	if( buf_track_R ) {
	    buf_track_R[nBufferPos] += (fVal_R * cost_track_R);
	}
#endif

	fVal_L = fVal_L * cost_L;
	fVal_R = fVal_R * cost_R;

	// update instr peak
	if ( fVal_L > fInstrPeak_L ) {
	    fInstrPeak_L = fVal_L;
	}
	if ( fVal_R > fInstrPeak_R ) {
	    fInstrPeak_R = fVal_R;
	}

	// to main mix
	buf_L[nBufferPos] += fVal_L;
	buf_R[nBufferPos] += fVal_R;

	fSamplePos += fStep;
    }
    note.m_fSamplePosition += nAvail_bytes * fStep;
    note.m_nSilenceOffset = 0;
    note.get_instrument()->set_peak_l( fInstrPeak_L );
    note.get_instrument()->set_peak_r( fInstrPeak_R );



#ifdef LADSPA_SUPPORT
    // LADSPA
    for ( unsigned nFX = 0; nFX < MAX_FX; ++nFX ) {
	T<LadspaFX>::shared_ptr pFX = engine->get_effects()->getLadspaFX( nFX );
	float fLevel = note.get_instrument()->get_fx_level( nFX );
	if ( ( pFX ) && ( fLevel != 0.0 ) ) {
	    fLevel = fLevel * pFX->getVolume();

	    float *pBuf_L = pFX->m_pBuffer_L;
	    float *pBuf_R = pFX->m_pBuffer_R;

//			float fFXCost_L = cost_L * fLevel;
//			float fFXCost_R = cost_R * fLevel;
	    float fFXCost_L = fLevel * fSendFXLevel_L;
	    float fFXCost_R = fLevel * fSendFXLevel_R;

	    int nBufferPos = nInitialBufferPos;
	    float fSamplePos = fInitialSamplePos;
	    for ( int i = 0; i < nAvail_bytes; ++i ) {
		int nSamplePos = ( int )fSamplePos;
		double fDiff = fSamplePos - nSamplePos;

		if ( ( nSamplePos + 1 ) >= nSampleFrames ) {
		    fVal_L = linear_interpolation( pSample_data_L[nSamplePos], 0, fDiff );
		    fVal_R = linear_interpolation( pSample_data_R[nSamplePos], 0, fDiff );
		} else {
		    fVal_L = linear_interpolation( pSample_data_L[nSamplePos], pSample_data_L[nSamplePos + 1], fDiff );
		    fVal_R = linear_interpolation( pSample_data_R[nSamplePos], pSample_data_R[nSamplePos + 1], fDiff );
		}

		pBuf_L[ nBufferPos ] += fVal_L * fFXCost_L * cost_L;
		pBuf_R[ nBufferPos ] += fVal_R * fFXCost_R * cost_R;
		fSamplePos += fStep;
		++nBufferPos;
	    }
	}
    }
#endif

    return retValue;
}

void SamplerPrivate::note_on( Note& note )
{
    SeqEvent ev;

    ev.frame = 0;
    ev.type = SeqEvent::NOTE_ON;
    ev.note = note;
    ev.quantize = false;

    handle_note_on(ev);
}

void SamplerPrivate::note_off( Note& note )
{
    SeqEvent ev;

    ev.frame = 0;
    ev.type = SeqEvent::NOTE_OFF;
    ev.note = note;
    ev.quantize = false;

    handle_note_on(ev);
}

void Sampler::stop_playing_notes( T<Instrument>::shared_ptr instrument )
{
    /*
    // send a note-off event to all notes present in the playing note queue
    for ( int i = 0; i < d->current_notes.size(); ++i ) {
    Note *pNote = d->current_notes[ i ];
    note.m_pADSR->release();
    }
    */

    if ( instrument ) { // stop all notes using this instrument
	SamplerPrivate::NoteList::iterator k, die;
	for( k=d->current_notes.begin() ; k!=d->current_notes.end() ; /* ++k */ ) {
	    if( k->get_instrument() == instrument ) {
		die = k; ++k;
		QMutexLocker lk( &d->mutex_current_notes );
		d->current_notes.erase(die);
		lk.unlock();
		instrument->dequeue();
	    } else {
		++k;
	    }
	}
    } else { // stop all notes
	SamplerPrivate::NoteList::iterator k;
	for( k=d->current_notes.begin() ; k!=d->current_notes.end() ; ++k ) {
	    k->get_instrument()->dequeue();
	}
	QMutexLocker lk( &d->mutex_current_notes );
	d->current_notes.clear();
    }
}



/// Preview, uses only the first layer
void Sampler::preview_sample( T<Sample>::shared_ptr sample, int length )
{
    InstrumentLayer *pLayer = d->preview_instrument->get_layer( 0 );

    T<Sample>::shared_ptr pOldSample = pLayer->get_sample();
    pLayer->set_sample( sample );

    Note previewNote( d->preview_instrument, 1.0, 1.0, 0.5, 0.5, 0 );

    stop_playing_notes( d->preview_instrument );
    d->note_on( previewNote );
}



void Sampler::preview_instrument( T<Instrument>::shared_ptr instr )
{
    T<Instrument>::shared_ptr old_preview;

    stop_playing_notes( d->preview_instrument );

    old_preview = d->preview_instrument;
    d->preview_instrument = instr;
    Note previewNote( d->preview_instrument, 1.0, 1.0, 0.5, 0.5, 0 );

    d->note_on( previewNote );	// exclusive note
}

/**
 * \brief Convenience method for adding an instrument to the sampler.
 */
void Sampler::add_instrument(T<Instrument>::shared_ptr instr)
{
    d->instrument_list->add(instr);
}

/**
 * \brief Direct access to the instruments in the sampler.
 */
T<InstrumentList>::shared_ptr Sampler::get_instrument_list()
{
    return d->instrument_list;
}

void Sampler::set_max_note_limit(int max)
{
    d->max_notes = max;
}

int Sampler::get_max_note_limit()
{
    return d->max_notes;
}

void Sampler::set_per_instrument_outs(bool enabled)
{
    #warning "Code disabled:"
    // d->per_instrument_outs = enabled;
    ERRORLOG("Per instrument outs is not implemented");
}

bool Sampler::get_per_instrument_outs()
{
    return d->per_instrument_outs;
}

void Sampler::set_per_instrument_outs_prefader(bool enabled)
{
    d->instrument_outs_prefader = enabled;
}

bool Sampler::get_per_instrument_outs_prefader()
{
    return d->instrument_outs_prefader;
}
