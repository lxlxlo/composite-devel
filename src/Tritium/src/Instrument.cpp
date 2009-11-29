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

#include <Tritium/Logger.hpp>
#include <Tritium/Instrument.hpp>
#include <Tritium/ADSR.hpp>
#include <Tritium/Sample.hpp>
#include <Tritium/Song.hpp>
#include <Tritium/LocalFileMng.hpp>
#include <Tritium/SoundLibrary.hpp>
#include <Tritium/AudioEngine.hpp>

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



// ::::


InstrumentList::InstrumentList()
{
}



InstrumentList::~InstrumentList()
{
    for ( unsigned int i = 0; i < m_list.size(); ++i ) {
	delete m_list[i];
    }
}



void InstrumentList::add( Instrument* newInstrument )
{
    m_list.push_back( newInstrument );
    m_posmap[newInstrument] = m_list.size() - 1;
}



Instrument* InstrumentList::get( unsigned int pos )
{
    if ( pos >= m_list.size() ) {
	ERRORLOG( QString( "pos > list.size(). pos = %1" ).arg( pos ) );
	return NULL;
    }
    return m_list[pos];
}



/// Returns index of instrument in list, if instrument not found, returns -1
int InstrumentList::get_pos( Instrument *pInstr )
{
    if ( m_posmap.find( pInstr ) == m_posmap.end() )
	return -1;
    return m_posmap[ pInstr ];
}



unsigned int InstrumentList::get_size()
{
    return m_list.size();
}


void InstrumentList::replace( Instrument* pNewInstr, unsigned nPos )
{
    if ( nPos >= m_list.size() ) {
	ERRORLOG( QString( "Instrument index out of bounds in InstrumentList::replace. pos >= list.size() - %1 > %2" ).arg( nPos ).arg( m_list.size() ) );
	return;
    }
    m_list.insert( m_list.begin() + nPos, pNewInstr );	// insert the new Instrument
    // remove the old Instrument
    m_list.erase( m_list.begin() + nPos + 1 );
}


void InstrumentList::del( int pos )
{
    assert( pos < ( int )m_list.size() );
    assert( pos >= 0 );
    m_list.erase( m_list.begin() + pos );
}


/*********************************************************************
 * InstrumentLayer Definition
 *********************************************************************
 */

/**
 * \brief Creates an instrument layer for a sample.
 *
 * \param sample Pointer to a sample with audio data.  May be 0.  This
 *               class will take ownership of it, including deleting
 *               it.
 */
InstrumentLayer::InstrumentLayer( Sample *sample )
    : m_velocity_range(0.0, 1.0)
    , m_pitch( 0.0 )
    , m_gain( 1.0 )
    , m_sample( sample )
{
}

InstrumentLayer::~InstrumentLayer()
{
    delete m_sample;
    m_sample = 0;
}

/**
 * \brief Sets the range of velocities at which the sample will trigger.
 *
 * If a velocity is in the range [min, max] (inclusive), then the
 * sample will trigger.  Note that if two layers overlap or share an
 * endpoint, behavior is defined by the Instrument class.
 *
 * \param min The minimum velocity at which this sample will trigger.
 *            Must be in the range [0.0, 1.0].  It should be less than
 *            max, but if it is not, they will be swapped.
 *
 * \param max The maximum velocity at which this sample will trigger.
 *            Must be in the range [0.0, 1.0].  It should be greater
 *            than min, but if it is not, they will be swapped.
 *
 * \return Nothing.  On success, the velocity range will be changed.
 *         If min or max are degenerate, then the range will not be
 *         updated.
 */
void InstrumentLayer::set_velocity_range(float min, float max)
{
    if( min > max ) {
	float tmp = max;
	max = min;
	min = tmp;
    }
    if( min < 0.0 || min > 1.0 ) {
	assert(false);
	return;
    }
    if( max < 0.0 || max > 1.0 ) {
	assert(false);
	return;
    }
    m_velocity_range.first = min;
    m_velocity_range.second = max;
}

/**
 * \brief Sets the range of velocities at which the sample will
 *
 * This is an overloaded function, provided for convenience.
 *
 * \param range A std::pair with the min/max range for the velocity.
 *
 * \return Nothing.  On success, the velocity range will be changed.
 *         If min or max are degenerate, then the range will not be
 *         updated.
 */
void InstrumentLayer::set_velocity_range(InstrumentLayer::velocity_range_t range)
{
    set_velocity_range(range.first, range.second);
}

/**
 * \brief Returns the current velocity range.
 *
 * \return A std::pair with the velocity range, [first, second].
 */
InstrumentLayer::velocity_range_t InstrumentLayer::get_velocity_range()
{
    return m_velocity_range;
}

/**
 * \brief Returns the minimum velocity for this sample.
 *
 * \return The minimum velocity, in the range [0.0, 1.0].
 */
float InstrumentLayer::get_min_velocity()
{
    return m_velocity_range.first;
}

/**
 * \brief Returns the maximum velocity for this sample.
 *
 * \return The maximum velocity, in the range [0.0, 1.0].
 */
float InstrumentLayer::get_max_velocity()
{
    return m_velocity_range.second;
}

/**
 * Determine if velocity 'vel' is in the range for this layer.
 *
 * \param vel A velocity value ([0.0, 1.0]).
 *
 * \return true if 'vel' is in the range [min, max].  false if outside
 *         the range.  The range is the same as returned by
 *         get_velocity_range().
 */
bool InstrumentLayer::in_velocity_range(float vel)
{
    return (vel >= m_velocity_range.first
	    && vel <= m_velocity_range.second);
}

/**
 * \brief Set the pitch scaling factor for sample.
 *
 * When the sample is played, scale the pitch of the sample by 'pitch'
 * (in musical half-steps).  If pitch = 0.0, then the sample's pitch
 * will not be manipulated.
 *
 * Note that pitch is being changed using the doppler effect
 * (resampling, i.e. playing the sample back faster or slower).
 * Therefore, changing the pitch also changes the effective length of
 * the sample.
 *
 * \param pitch The scale factor for the pitch, in musical
 *              half-steps.  May be positive or negative.
 *
 * \return Nothing.
 */
void InstrumentLayer::set_pitch( float pitch )
{
    m_pitch = pitch;
}

/**
 * \brief Returns the pitch manipulation setting for the sample.
 *
 * See InstrumentLayer::set_pitch()
 *
 * \return The current pitch parameter, in musical half-steps (may be
 *         positive or negative).
 */
float InstrumentLayer::get_pitch() {
    return m_pitch;
}

/**
 * \brief Sets the gain for scaling the sample's amplitude.
 *
 * If an envelope is used with this sample, this gain will be in
 * addition to velocity and envelope settings.
 *
 * \param gain The factor by which to scale the sample's amplitude.
 *             Must be >= 0.
 *
 * \return Nothing.  If gain is outside the range, then it will be
 *         silently ignored.
 */
void InstrumentLayer::set_gain( float gain ) {
    assert(gain >= 0.0);
    if(gain >= 0.0) m_gain = gain;
}

/**
 * \brief Returns the current gain for tha sample's amplitude.
 *
 * \return The current gain.  Will be >= 0.
 */
float InstrumentLayer::get_gain() {
    return m_gain;
}

/**
 * \brief Resets the sample for this layer.
 *
 * \param sample The new sample to use.  May be 0.  This class will
 *               take ownership of the object, including deleting it.
 *
 * \return Nothing.
 */
void InstrumentLayer::set_sample( Sample* sample ) {
    delete m_sample;
    m_sample = sample;
}

/**
 * \brief Return pointer to the sample for this layer.
 *
 * \return Pointer to a Tritium::Sample object, or 0 if there is not
 *         currently a sample loaded.
 */
Sample* InstrumentLayer::get_sample() {
    return m_sample;
}
