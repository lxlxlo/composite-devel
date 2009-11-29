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

#ifndef TRITIUM_INSTRUMENTLAYER_HPP
#define TRITIUM_INSTRUMENTLAYER_HPP

#include <Tritium/globals.hpp>
#include <utility> // std::pair

namespace Tritium
{
    class Sample;

    /**
     * A single sample inside an instrument, with its gain and
     * velocity settings.  See documentation for Instrument for a
     * description for 'Layer'.  This class is largely for the
     * convenience of class Instrument.
     *
     */
    class InstrumentLayer
    {
    public:
	typedef std::pair<float, float> velocity_range_t;

	InstrumentLayer( Sample *sample );
	~InstrumentLayer();

	void set_velocity_range(float min, float max);
	void set_velocity_range(velocity_range_t range);
        velocity_range_t get_velocity_range();
	bool in_velocity_range(float vel);
	float get_min_velocity();
	float get_max_velocity();

	void set_pitch( float pitch );
	float get_pitch();

	void set_gain( float gain );
	float get_gain();

	void set_sample( Sample* sample );
	Sample* get_sample();

    private:
	velocity_range_t m_velocity_range; // Range: [min, max]
	float m_pitch;
	float m_gain;
	Sample *m_sample;
    };



} // namespace Tritium

#endif // TRITIUM_INSTRUMENTLAYER_HPP
