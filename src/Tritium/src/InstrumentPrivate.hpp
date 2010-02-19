/*
 * Copyright(c) 2009 by Gabriel M. Beddingfield <gabriel@teuton.org>
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
#ifndef TRITIUM_INSTRUMENTPRIVATE_HPP
#define TRITIUM_INSTRUMENTPRIVATE_HPP

#include <Tritium/globals.hpp>
#include <Tritium/Instrument.hpp>
#include <QString>

namespace Tritium
{
    class ADSR;
    class InstrumentLayer;

    /**
     * /brief Internal class for Tritium::Instrument.
     *
     */
    class Instrument::InstrumentPrivate
    {
    public:
	int queued;
	InstrumentLayer* layer_list[MAX_LAYERS];
	ADSR* adsr;
	bool muted;
	QString name;               ///< Instrument name
	float pan_l;                ///< Pan of the instrument (left)
	float pan_r;                ///< Pan of the instrument (right)
	float gain;
	float filter_resonance;     ///< Filter resonant frequency (0..1)
	float filter_cutoff;        ///< Filter cutoff (0..1)
	float peak_l;               ///< current peak value (left)
	float peak_r;               ///< current peak value (right)
	float random_pitch_factor;
	QString id;                 ///< ID of the instrument
	QString drumkit_name;       ///< Drumkit name
	bool filter_active;         ///< Is filter active?
	int mute_group;             ///< Mute group

	bool active;			///< is the instrument active?
	bool soloed;
	bool stop_notes;		///

	InstrumentPrivate(const QString& id, const QString& name, ADSR* adsr );
	~InstrumentPrivate();
    };


} // namespace Tritium

#endif // TRITIUM_INSTRUMENTPRIVATE_HPP
