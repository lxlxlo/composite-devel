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

#ifndef TRITIUM_INSTRUMENT_HPP
#define TRITIUM_INSTRUMENT_HPP

#include <QtCore/QString>
#include <Tritium/globals.hpp>
#include <Tritium/Logger.hpp>
#include <deque>
#include <utility> // std::pair
#include <cassert>

namespace Tritium
{

    class ADSR;
    class Sample;
    class InstrumentLayer;

    /**
     * Class for managing a single voice inside the sampler.  The
     * instrument may be made up of several layered samples (chosen by
     * the input velocity).
     */
    class Instrument
    {
    public:
	Instrument(
	    const QString& id,
	    const QString& name,
	    ADSR* adsr
	    );
	
	/// create a new object without anything in it.
	static Instrument * create_empty();
	~Instrument();

	/// creates a new object; loads samples from drumkit/instrument.
	static Instrument* load_instrument(
	    const QString& drumkit_name,
	    const QString& instrument_name
	    );
	
	/// loads state _and_ samples into an Instrument from a `placeholder` instrument
	/// (i.e. an Instrument that has everything but the actal samples.)
	void load_from_placeholder( Instrument* placeholder, bool is_live = true );
	
	/// loads instrument from path into a `live` Instrument object.
	void load_from_name(
	    const QString& drumkit_name,
	    const QString& instrument_name,
	    bool is_live = true
	    );

	InstrumentLayer* get_layer( int index );
	void set_layer( InstrumentLayer* layer, unsigned index );


	void set_name( const QString& name );
	const QString& get_name();

	void set_id( const QString& id );
	const QString& get_id();

	void set_adsr( ADSR* adsr );
	ADSR* get_adsr();

	void set_mute_group( int group );
	int get_mute_group();

	void set_muted( bool muted );
	bool is_muted();

	float get_pan_l();
	void set_pan_l( float val );

	float get_pan_r();
	void set_pan_r( float val );

	float get_gain();
	void set_gain( float gain );

	float get_volume();
	void set_volume( float volume );

	bool is_filter_active();
	void set_filter_active( bool active );

	float get_filter_resonance();
	void set_filter_resonance( float val );

	float get_filter_cutoff();
	void set_filter_cutoff( float val );

	float get_peak_l();
	void set_peak_l( float val );

	float get_peak_r();
	void set_peak_r( float val );

	float get_fx_level( int index );
	void set_fx_level( float level, int index );

	float get_random_pitch_factor();
	void set_random_pitch_factor( float val );

	void set_drumkit_name( const QString& name );
	const QString& get_drumkit_name();

	bool is_active();
	void set_active( bool active );

	bool is_soloed();
	void set_soloed( bool soloed );
	void enqueue();
	void dequeue();
	int is_queued();

	bool is_stop_notes();
	void set_stop_note( bool stopnotes );

    private:
	int __queued;
	InstrumentLayer* __layer_list[MAX_LAYERS];
	ADSR* __adsr;
	bool __muted;
	QString __name;			///< Instrument name
	float __pan_l;			///< Pan of the instrument (left)
	float __pan_r;			///< Pan of the instrument (right)
	float __gain;
	float __volume;			///< Volume of the instrument
	float __filter_resonance;	///< Filter resonant frequency (0..1)
	float __filter_cutoff;		///< Filter cutoff (0..1)
	float __peak_l;			///< current peak value (left)
	float __peak_r;			///< current peak value (right)
	float __fx_level[MAX_FX];	///< Ladspa FX level
	float __random_pitch_factor;
	QString __id;			///< ID of the instrument
	QString __drumkit_name;		///< Drumkit name
	bool __filter_active;		///< Is filter active?
	int __mute_group;		///< Mute group

	bool __active;			///< is the instrument active?
	bool __soloed;
	bool __stop_notes;		///
    };

} // namespace Tritium

#endif // TRITIUM_INSTRUMENT_HPP
