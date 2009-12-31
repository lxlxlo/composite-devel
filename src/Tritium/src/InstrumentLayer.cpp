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

#include <Tritium/InstrumentLayer.hpp>
#include <Tritium/Sample.hpp>
#include <cassert>

using namespace Tritium;

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
InstrumentLayer::InstrumentLayer( T<Sample>::shared_ptr sample )
    : m_velocity_range(0.0, 1.0)
    , m_pitch( 0.0 )
    , m_gain( 1.0 )
    , m_sample( sample )
{
}

InstrumentLayer::~InstrumentLayer()
{
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
void InstrumentLayer::set_sample( T<Sample>::shared_ptr sample )
{
    m_sample = sample;
}

/**
 * \brief Return pointer to the sample for this layer.
 *
 * \return Pointer to a Tritium::Sample object, or 0 if there is not
 *         currently a sample loaded.
 */
T<Sample>::shared_ptr InstrumentLayer::get_sample()
{
    return m_sample;
}
