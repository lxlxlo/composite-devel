/*
 * Copyright(c) 2002-2008 by Alex >Comix< Cominu [comix@users.sourceforge.net]
 * Copyright (c) 2009 by Gabriel M. Beddingfield <gabriel@teuton.org>
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

#include "InstrumentPrivate.hpp"

#include <Tritium/Instrument.hpp>
#include <Tritium/InstrumentLayer.hpp>
#include <Tritium/InstrumentList.hpp>
#include <Tritium/ADSR.hpp>
#include <Tritium/Sample.hpp>
#include <Tritium/SimpleStereoSample.hpp>
#include <Tritium/Song.hpp>
#include <Tritium/LocalFileMng.hpp>
#include <Tritium/SoundLibrary.hpp>
#include <Tritium/Logger.hpp>
#include <Tritium/Engine.hpp>

#include <QFileInfo>
#include <cassert>

using namespace Tritium;

/*********************************************************************
 * InstrumentPrivate definition
 *********************************************************************
 */
Instrument::InstrumentPrivate::InstrumentPrivate(
    const QString& id,
    const QString& name,
    ADSR* adsr
    )
    : queued( 0 )
    , adsr( adsr )
    , muted( false )
    , name( name )
    , pan_l( 1.0 )
    , pan_r( 1.0 )
    , gain( 1.0 )
    , filter_resonance( 0.0 )
    , filter_cutoff( 1.0 )
    , peak_l( 0.0 )
    , peak_r( 0.0 )
    , random_pitch_factor( 0.0 )
    , id( id )
    , drumkit_name( "" )
    , filter_active( false )
    , mute_group( -1 )
    , active( true )
    , soloed( false )
    , stop_notes( false )
{
    for ( unsigned nLayer = 0; nLayer < MAX_LAYERS; ++nLayer ) {
	layer_list[ nLayer ] = NULL;
    }
}

Instrument::InstrumentPrivate::~InstrumentPrivate()
{
    for ( unsigned nLayer = 0; nLayer < MAX_LAYERS; ++nLayer ) {
	delete layer_list[ nLayer ];
	layer_list[ nLayer ] = NULL;
    }
    delete adsr;
    adsr = NULL;
}

/*********************************************************************
 * class Instrument Definition.
 *********************************************************************
 */

Instrument::Instrument(
    const QString& id,
    const QString& name,
    ADSR* adsr
    ) : d(0)
{
    d = new InstrumentPrivate(id, name, adsr);
}

Instrument::~Instrument()
{
    delete d;
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

    return d->layer_list[ nLayer ];
}

void Instrument::set_layer( InstrumentLayer* pLayer, unsigned nLayer )
{
    if ( nLayer < MAX_LAYERS ) {
	d->layer_list[ nLayer ] = pLayer;
    } else {
	ERRORLOG( "nLayer > MAX_LAYER" );
    }
}

void Instrument::set_adsr( ADSR* adsr )
{
    delete d->adsr;
    d->adsr = adsr;
}

/**
 * \brief Load stand and samples from a `placeholder` instrument.
 *
 * Loads the stand and samples into an Instrument object from a
 * `placeholder` instrument (an Instrument that has everything but the
 * actual samples).
 */
void Instrument::load_from_placeholder( Engine* engine, T<Instrument>::shared_ptr placeholder, bool is_live )
{
    LocalFileMng mgr(engine);
    QString path =
	mgr.getDrumkitDirectory( placeholder->get_drumkit_name() )
	+ placeholder->get_drumkit_name()
	+ "/";
    for ( unsigned nLayer = 0; nLayer < MAX_LAYERS; ++nLayer ) {
	InstrumentLayer *pNewLayer = placeholder->get_layer( nLayer );
	if ( pNewLayer != NULL ) {
	    // this is a 'placeholder sample:
	    T<Sample>::shared_ptr pNewSample = pNewLayer->get_sample();
			
	    // now we load the actal data:
	    QFileInfo samp_file( pNewSample->get_filename() );
	    if( !samp_file.exists() ) {
		samp_file.setFile( path + pNewSample->get_filename() );
	    }
	    T<SimpleStereoSample>::shared_ptr pSample = SimpleStereoSample::load( samp_file.absoluteFilePath() );
	    InstrumentLayer *pOldLayer = this->get_layer( nLayer );

	    if ( pSample == NULL ) {
		ERRORLOG( "Error loading sample. Creating a new empty layer." );
		if ( is_live )
		    engine->lock( RIGHT_HERE );
				
		this->set_layer( NULL, nLayer );
				
		if ( is_live )
		    engine->unlock();
		delete pOldLayer;
		continue;
	    }
	    InstrumentLayer *pLayer = new InstrumentLayer( pSample );
	    pLayer->set_velocity_range( pNewLayer->get_velocity_range() );
	    pLayer->set_gain( pNewLayer->get_gain() );
	    pLayer->set_pitch(pNewLayer->get_pitch()); 

	    if ( is_live )
		engine->lock( RIGHT_HERE );
			
	    this->set_layer( pLayer, nLayer );	// set the new layer
			
	    if ( is_live )
		engine->unlock();
	    delete pOldLayer;		// delete the old layer

	} else {
	    InstrumentLayer *pOldLayer = this->get_layer( nLayer );
	    if ( is_live )
		engine->lock( RIGHT_HERE );
			
	    this->set_layer( NULL, nLayer );
			
	    if ( is_live )
		engine->unlock();
	    delete pOldLayer;		// delete the old layer
	}

    }
    if ( is_live )
	engine->lock( RIGHT_HERE );
	
    // update instrument properties
    this->set_gain( placeholder->get_gain() );
    this->set_id( placeholder->get_id() );
    this->set_name( placeholder->get_name() );
    this->set_pan_l( placeholder->get_pan_l() );
    this->set_pan_r( placeholder->get_pan_r() );
    this->set_drumkit_name( placeholder->get_drumkit_name() );
    this->set_muted( placeholder->is_muted() );
    this->set_random_pitch_factor( placeholder->get_random_pitch_factor() );
    this->set_adsr( new ADSR( *( placeholder->get_adsr() ) ) );
    this->set_filter_active( placeholder->is_filter_active() );
    this->set_filter_cutoff( placeholder->get_filter_cutoff() );
    this->set_filter_resonance( placeholder->get_filter_resonance() );
    this->set_mute_group( placeholder->get_mute_group() );
	
    if ( is_live )
	engine->unlock();
}

/**
 * \brief Create a new Instrument without anything in it.
 */
T<Instrument>::shared_ptr Instrument::create_empty()
{
    return T<Instrument>::shared_ptr( new Instrument( "", "Empty Instrument", new ADSR() ) );
}

/**
 * \brief Create a new Instrument and load samples from the drumkit/instrument.
 */
T<Instrument>::shared_ptr Instrument::load_instrument(
    Engine* engine,
    const QString& drumkit_name,
    const QString& instrument_name
    )
{
    T<Instrument>::shared_ptr I = create_empty();
    I->load_from_name( engine, drumkit_name, instrument_name, false );
    return I;
}

/**
 * \brief Loads instrument from path into a `live` Instrument object.
 */
void Instrument::load_from_name(
    Engine* engine,
    const QString& drumkit_name,
    const QString& instrument_name,
    bool is_live
    )
{
    T<Instrument>::shared_ptr pInstr;
	
    LocalFileMng mgr(engine);
    QString sDrumkitPath = mgr.getDrumkitDirectory( drumkit_name );

    // find the drumkit
    QString drdir = mgr.getDrumkitDirectory( drumkit_name ) + drumkit_name;
    if ( !QDir( drdir ).exists() )
	return;
    T<Drumkit>::shared_ptr pDrumkitInfo = mgr.loadDrumkit( drdir );
    assert( pDrumkitInfo );

    // find the instrument
    T<InstrumentList>::shared_ptr pInstrList = pDrumkitInfo->getInstrumentList();
    for ( unsigned nInstr = 0; nInstr < pInstrList->get_size(); ++nInstr ) {
	pInstr = pInstrList->get( nInstr );
	if ( pInstr->get_name() == instrument_name ) {
	    break;
	}
    }
	
    if ( pInstr ) {
	load_from_placeholder( engine, pInstr, is_live );
    }
    pDrumkitInfo.reset();
}

void Instrument::set_name( const QString& name )
{
    d->name = name;
}

const QString& Instrument::get_name()
{
    return d->name;
}

void Instrument::set_id( const QString& id )
{
    d->id = id;
}

const QString& Instrument::get_id()
{
    return d->id;
}

ADSR* Instrument::get_adsr()
{
    return d->adsr;
}

void Instrument::set_mute_group( int group )
{
    d->mute_group = group;
}

int Instrument::get_mute_group()
{
    return d->mute_group;
}

void Instrument::set_muted( bool muted )
{
    d->muted = muted;
}

bool Instrument::is_muted()
{
    return d->muted;
}

float Instrument::get_pan_l()
{
    return d->pan_l;
}

void Instrument::set_pan_l( float val )
{
    d->pan_l = val;
}

float Instrument::get_pan_r()
{
    return d->pan_r;
}

void Instrument::set_pan_r( float val )
{
    d->pan_r = val;
}

float Instrument::get_gain()
{
    return d->gain;
}

void Instrument::set_gain( float gain )
{
    d->gain = gain;
}

bool Instrument::is_filter_active()
{
    return d->filter_active;
}

void Instrument::set_filter_active( bool active )
{
    d->filter_active = active;
}

float Instrument::get_filter_resonance()
{
    return d->filter_resonance;
}

void Instrument::set_filter_resonance( float val )
{
    d->filter_resonance = val;
}

float Instrument::get_filter_cutoff()
{
    return d->filter_cutoff;
}

void Instrument::set_filter_cutoff( float val )
{
    d->filter_cutoff = val;
}

float Instrument::get_peak_l()
{
    return d->peak_l;
}

void Instrument::set_peak_l( float val )
{
    d->peak_l = val;
}

float Instrument::get_peak_r()
{
    return d->peak_r;
}

void Instrument::set_peak_r( float val )
{
    d->peak_r = val;
}

float Instrument::get_random_pitch_factor()
{
    return d->random_pitch_factor;
}

void Instrument::set_random_pitch_factor( float val )
{
    d->random_pitch_factor = val;
}

void Instrument::set_drumkit_name( const QString& name )
{
    d->drumkit_name = name;
}

const QString& Instrument::get_drumkit_name()
{
    return d->drumkit_name;
}

bool Instrument::is_active()
{
    return d->active;
}

void Instrument::set_active( bool active )
{
    d->active = active;
}

bool Instrument::is_soloed()
{
    return d->soloed;
}

void Instrument::set_soloed( bool soloed )
{
    d->soloed = soloed;
}

void Instrument::enqueue()
{
    d->queued++;
}

void Instrument::dequeue()
{
    assert( d->queued > 0 );
    d->queued--;
}

int Instrument::is_queued()
{
    return d->queued;
}

bool Instrument::is_stop_notes()
{
    return d->stop_notes;
}

void Instrument::set_stop_note( bool stopnotes )
{
    d->stop_notes = stopnotes;
}
