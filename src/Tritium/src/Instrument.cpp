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

#include <Tritium/Instrument.hpp>
#include <Tritium/InstrumentLayer.hpp>
#include <Tritium/InstrumentList.hpp>
#include <Tritium/ADSR.hpp>
#include <Tritium/Sample.hpp>
#include <Tritium/Song.hpp>
#include <Tritium/LocalFileMng.hpp>
#include <Tritium/SoundLibrary.hpp>
#include <Tritium/AudioEngine.hpp>
#include <Tritium/Logger.hpp>

#include <cassert>

using namespace Tritium;

Instrument::Instrument( const QString& id, const QString& name, ADSR* adsr )
    : __queued( 0 )
    , __adsr( adsr )
    , __muted( false )
    , __name( name )
    , __pan_l( 1.0 )
    , __pan_r( 1.0 )
    , __gain( 1.0 )
    , __volume( 1.0 )
    , __filter_resonance( 0.0 )
    , __filter_cutoff( 1.0 )
    , __peak_l( 0.0 )
    , __peak_r( 0.0 )
    , __random_pitch_factor( 0.0 )
    , __id( id )
    , __drumkit_name( "" )
    , __filter_active( false )
    , __mute_group( -1 )
    , __active( true )
    , __soloed( false )
    , __stop_notes( false )
{
    for ( unsigned nFX = 0; nFX < MAX_FX; ++nFX ) {
	__fx_level[ nFX ] = 0.0;
    }

    for ( unsigned nLayer = 0; nLayer < MAX_LAYERS; ++nLayer ) {
	__layer_list[ nLayer ] = NULL;
    }
}



Instrument::~Instrument()
{
    for ( unsigned nLayer = 0; nLayer < MAX_LAYERS; ++nLayer ) {
	delete __layer_list[ nLayer ];
	__layer_list[ nLayer ] = NULL;
    }
    delete __adsr;
    __adsr = NULL;
}

InstrumentLayer* Instrument::get_layer( int nLayer )
{
    if ( nLayer < 0 ) {
	ERRORLOG( QString( "nLayer < 0 (nLayer=%1)" ).arg( nLayer ) );
	return NULL;
    }
    if ( nLayer >= MAX_LAYERS ) {
	ERRORLOG( QString( "nLayer > MAX_LAYERS (nLayer=%1)" ).arg( nLayer ) );
	return NULL;
    }

    return __layer_list[ nLayer ];
}

void Instrument::set_layer( InstrumentLayer* pLayer, unsigned nLayer )
{
    if ( nLayer < MAX_LAYERS ) {
	__layer_list[ nLayer ] = pLayer;
    } else {
	ERRORLOG( "nLayer > MAX_LAYER" );
    }
}

void Instrument::set_adsr( ADSR* adsr )
{
    delete __adsr;
    __adsr = adsr;
}

void Instrument::load_from_placeholder( Instrument* placeholder, bool is_live )
{
    LocalFileMng mgr;
    QString path = mgr.getDrumkitDirectory( placeholder->get_drumkit_name() ) + placeholder->get_drumkit_name() + "/";
    for ( unsigned nLayer = 0; nLayer < MAX_LAYERS; ++nLayer ) {
	InstrumentLayer *pNewLayer = placeholder->get_layer( nLayer );
	if ( pNewLayer != NULL ) {
	    // this is a 'placeholder sample:
	    Sample *pNewSample = pNewLayer->get_sample();
			
	    // now we load the actal data:
	    Sample *pSample = Sample::load( path + pNewSample->get_filename() );
	    InstrumentLayer *pOldLayer = this->get_layer( nLayer );

	    if ( pSample == NULL ) {
		ERRORLOG( "Error loading sample. Creating a new empty layer." );
		if ( is_live )
		    AudioEngine::get_instance()->lock( RIGHT_HERE );
				
		this->set_layer( NULL, nLayer );
				
		if ( is_live )
		    AudioEngine::get_instance()->unlock();
		delete pOldLayer;
		continue;
	    }
	    InstrumentLayer *pLayer = new InstrumentLayer( pSample );
	    pLayer->set_velocity_range( pNewLayer->get_velocity_range() );
	    pLayer->set_gain( pNewLayer->get_gain() );
	    pLayer->set_pitch(pNewLayer->get_pitch()); 

	    if ( is_live )
		AudioEngine::get_instance()->lock( RIGHT_HERE );
			
	    this->set_layer( pLayer, nLayer );	// set the new layer
			
	    if ( is_live )
		AudioEngine::get_instance()->unlock();
	    delete pOldLayer;		// delete the old layer

	} else {
	    InstrumentLayer *pOldLayer = this->get_layer( nLayer );
	    if ( is_live )
		AudioEngine::get_instance()->lock( RIGHT_HERE );
			
	    this->set_layer( NULL, nLayer );
			
	    if ( is_live )
		AudioEngine::get_instance()->unlock();
	    delete pOldLayer;		// delete the old layer
	}

    }
    if ( is_live )
	AudioEngine::get_instance()->lock( RIGHT_HERE );
	
    // update instrument properties
    this->set_gain( placeholder->get_gain() );
    this->set_id( placeholder->get_id() );
    this->set_name( placeholder->get_name() );
    this->set_pan_l( placeholder->get_pan_l() );
    this->set_pan_r( placeholder->get_pan_r() );
    this->set_volume( placeholder->get_volume() );
    this->set_drumkit_name( placeholder->get_drumkit_name() );
    this->set_muted( placeholder->is_muted() );
    this->set_random_pitch_factor( placeholder->get_random_pitch_factor() );
    this->set_adsr( new ADSR( *( placeholder->get_adsr() ) ) );
    this->set_filter_active( placeholder->is_filter_active() );
    this->set_filter_cutoff( placeholder->get_filter_cutoff() );
    this->set_filter_resonance( placeholder->get_filter_resonance() );
    this->set_mute_group( placeholder->get_mute_group() );
	
    if ( is_live )
	AudioEngine::get_instance()->unlock();
}

Instrument * Instrument::create_empty()
{
    return new Instrument( "", "Empty Instrument", new ADSR() );
}

Instrument * Instrument::load_instrument(
    const QString& drumkit_name,
    const QString& instrument_name
    )
{
    Instrument * I = create_empty();
    I->load_from_name( drumkit_name, instrument_name, false );
    return I;
}

void Instrument::load_from_name(
    const QString& drumkit_name,
    const QString& instrument_name,
    bool is_live
    )
{
    Instrument * pInstr = NULL;
	
    LocalFileMng mgr;
    QString sDrumkitPath = mgr.getDrumkitDirectory( drumkit_name );

    // find the drumkit
    QString drdir = mgr.getDrumkitDirectory( drumkit_name ) + drumkit_name;
    if ( !QDir( drdir ).exists() )
	return;
    Drumkit *pDrumkitInfo = mgr.loadDrumkit( drdir );
    assert( pDrumkitInfo );

    // find the instrument
    InstrumentList *pInstrList = pDrumkitInfo->getInstrumentList();
    for ( unsigned nInstr = 0; nInstr < pInstrList->get_size(); ++nInstr ) {
	pInstr = pInstrList->get( nInstr );
	if ( pInstr->get_name() == instrument_name ) {
	    break;
	}
    }
	
    if ( pInstr != NULL ) {
	load_from_placeholder( pInstr, is_live );
    }
    delete pDrumkitInfo;
}

void Instrument::set_name( const QString& name )
{
    __name = name;
}

const QString& Instrument::get_name()
{
    return __name;
}

void Instrument::set_id( const QString& id )
{
    __id = id;
}

const QString& Instrument::get_id()
{
    return __id;
}

ADSR* Instrument::get_adsr()
{
    return __adsr;
}

void Instrument::set_mute_group( int group )
{
    __mute_group = group;
}

int Instrument::get_mute_group()
{
    return __mute_group;
}

void Instrument::set_muted( bool muted )
{
    __muted = muted;
}

bool Instrument::is_muted()
{
    return __muted;
}

float Instrument::get_pan_l()
{
    return __pan_l;
}

void Instrument::set_pan_l( float val )
{
    __pan_l = val;
}

float Instrument::get_pan_r()
{
    return __pan_r;
}

void Instrument::set_pan_r( float val )
{
    __pan_r = val;
}

float Instrument::get_gain()
{
    return __gain;
}

void Instrument::set_gain( float gain )
{
    __gain = gain;
}

float Instrument::get_volume()
{
    return __volume;
}

void Instrument::set_volume( float volume )
{
    __volume = volume;
}

bool Instrument::is_filter_active()
{
    return __filter_active;
}

void Instrument::set_filter_active( bool active )
{
    __filter_active = active;
}

float Instrument::get_filter_resonance()
{
    return __filter_resonance;
}

void Instrument::set_filter_resonance( float val )
{
    __filter_resonance = val;
}

float Instrument::get_filter_cutoff()
{
    return __filter_cutoff;
}

void Instrument::set_filter_cutoff( float val )
{
    __filter_cutoff = val;
}

float Instrument::get_peak_l()
{
    return __peak_l;
}

void Instrument::set_peak_l( float val )
{
    __peak_l = val;
}

float Instrument::get_peak_r()
{
    return __peak_r;
}

void Instrument::set_peak_r( float val )
{
    __peak_r = val;
}

float Instrument::get_fx_level( int index )
{
    return __fx_level[index];
}

void Instrument::set_fx_level( float level, int index )
{
    __fx_level[index] = level;
}

float Instrument::get_random_pitch_factor()
{
    return __random_pitch_factor;
}

void Instrument::set_random_pitch_factor( float val )
{
    __random_pitch_factor = val;
}

void Instrument::set_drumkit_name( const QString& name )
{
    __drumkit_name = name;
}

const QString& Instrument::get_drumkit_name()
{
    return __drumkit_name;
}

bool Instrument::is_active()
{
    return __active;
}

void Instrument::set_active( bool active )
{
    __active = active;
}

bool Instrument::is_soloed()
{
    return __soloed;
}

void Instrument::set_soloed( bool soloed )
{
    __soloed = soloed;
}

void Instrument::enqueue()
{
    __queued++;
}

void Instrument::dequeue()
{
    assert( __queued > 0 );
    __queued--;
}

int Instrument::is_queued()
{
    return __queued;
}

bool Instrument::is_stop_notes()
{
    return __stop_notes;
}

void Instrument::set_stop_note( bool stopnotes )
{
    __stop_notes = stopnotes;
}
