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

#include <Tritium/Note.hpp>
#include <Tritium/Instrument.hpp>
#include <Tritium/Logger.hpp>

#include <cassert>
#include <cstdlib>

namespace Tritium
{

Note::Note(
    T<Instrument>::shared_ptr pInstrument,
    float velocity,
    float fPan_L,
    float fPan_R,
    int nLength,
    float fPitch
)
		: m_nSilenceOffset( 0 )
		, m_nReleaseOffset( 0 )
		, m_fSamplePosition( 0.0 )
		, m_fCutoff( 1.0 )
		, m_fResonance( 0.0 )
		, m_fBandPassFilterBuffer_L( 0.0 )
		, m_fBandPassFilterBuffer_R( 0.0 )
		, m_fLowPassFilterBuffer_L( 0.0 )
		, m_fLowPassFilterBuffer_R( 0.0 )
		, m_nHumanizeDelay( 0 )
		, __velocity( velocity )
		, __leadlag( 0.0 )
{
	set_pan_l( fPan_L );
	set_pan_r( fPan_R );
	set_length( nLength );

	set_instrument( pInstrument );
	set_pitch( fPitch );
}




Note::Note( const Note* pNote )
{
	m_nSilenceOffset          = pNote->m_nSilenceOffset;
	m_nReleaseOffset          = pNote->m_nReleaseOffset;
	m_fSamplePosition         = pNote->m_fSamplePosition;
	// m_adsr copied in set_instrument()
	m_fCutoff                 = pNote->m_fCutoff;
	m_fResonance              = pNote->m_fResonance;
	m_fBandPassFilterBuffer_L = pNote->m_fBandPassFilterBuffer_L;
	m_fBandPassFilterBuffer_R = pNote->m_fBandPassFilterBuffer_R;
	m_fLowPassFilterBuffer_L  = pNote->m_fLowPassFilterBuffer_L;
	m_fLowPassFilterBuffer_R  = pNote->m_fLowPassFilterBuffer_R;
	m_nHumanizeDelay          = pNote->m_nHumanizeDelay;
	set_instrument(             pNote->__instrument );
	__velocity                = pNote->get_velocity();
	set_pan_l(                  pNote->get_pan_l() );
	set_pan_r(                  pNote->get_pan_r() );
	set_leadlag(                pNote->get_leadlag() );
	set_length(                 pNote->get_length() );
	set_pitch(                  pNote->get_pitch() );
}



Note::~Note()
{
	//infoLog("DESTROY");
	//delete m_pADSR;
}



void Note::set_instrument( T<Instrument>::shared_ptr instrument )
{
	if ( ! instrument ) {
		return;
	}

	__instrument = instrument;
	assert( __instrument->get_adsr() );
	m_adsr = ADSR( *( __instrument->get_adsr() ) );

	/*
		if ( pInstrument->m_pADSR == NULL ) {
			ERRORLOG( "NULL ADSR? Instrument: " + pInstrument->m_sName );
		}
		else {
			DEBUGLOG( "copio l'adsr dallo strumento" );
			if ( m_pADSR ) {
				DEBUGLOG( "Replacing an existing ADSR" );
				delete m_pADSR;
				m_pADSR = NULL;
			}
			m_pADSR = new ADSR( *(m_pInstrument->m_pADSR) );
		}
	*/
}



void Note::dumpInfo() const
{
    DEBUGLOG( QString("humanize offset%2\t instr: %3\t pitch: %4")
	      .arg( m_nHumanizeDelay )
	      .arg( __instrument->get_name() )
	      .arg( get_pitch() )
	);
}



Note* Note::copy()
{
	Note* note = new Note(
	    get_instrument(),
	    get_velocity(),
	    get_pan_l(),
	    get_pan_r(),
	    get_length(),
	    get_pitch()
	);

	note->set_leadlag(get_leadlag());


	return note;
}


};
