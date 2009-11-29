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



    /**

       \brief Instrument class

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

	/// Returns a layer in the list
	/// See below for definition.
	inline InstrumentLayer* get_layer( int index );

	/// Sets a layer in the list
	void set_layer( InstrumentLayer* layer, unsigned index );


	void set_name( const QString& name ) {
	    __name = name;
	}
	const QString& get_name() {
	    return __name;
	}

	void set_id( const QString& id ) {
	    __id = id;
	}
	inline const QString& get_id() {
	    return __id;
	}

	void set_adsr( ADSR* adsr );
	ADSR* get_adsr() {
	    return __adsr;
	}

	void set_mute_group( int group ) {
	    __mute_group = group;
	}
	inline int get_mute_group() {
	    return __mute_group;
	}

	void set_muted( bool muted ) {
	    __muted = muted;
	}
	inline bool is_muted() {
	    return __muted;
	}

	inline float get_pan_l() {
	    return __pan_l;
	}
	void set_pan_l( float val ) {
	    __pan_l = val;
	}

	inline float get_pan_r() {
	    return __pan_r;
	}
	void set_pan_r( float val ) {
	    __pan_r = val;
	}

	inline float get_gain() {
	    return __gain;
	}
	void set_gain( float gain ) {
	    __gain = gain;
	}

	inline float get_volume() {
	    return __volume;
	}
	void set_volume( float volume ) {
	    __volume = volume;
	}

	inline bool is_filter_active() {
	    return __filter_active;
	}
	void set_filter_active( bool active ) {
	    __filter_active = active;
	}

	inline float get_filter_resonance() {
	    return __filter_resonance;
	}
	void set_filter_resonance( float val ) {
	    __filter_resonance = val;
	}

	inline float get_filter_cutoff() {
	    return __filter_cutoff;
	}
	void set_filter_cutoff( float val ) {
	    __filter_cutoff = val;
	}

	inline float get_peak_l() {
	    return __peak_l;
	}
	void set_peak_l( float val ) {
	    __peak_l = val;
	}

	inline float get_peak_r() {
	    return __peak_r;
	}
	void set_peak_r( float val ) {
	    __peak_r = val;
	}

	inline float get_fx_level( int index ) {
	    return __fx_level[index];
	}
	void set_fx_level( float level, int index ) {
	    __fx_level[index] = level;
	}

	inline float get_random_pitch_factor() {
	    return __random_pitch_factor;
	}
	void set_random_pitch_factor( float val ) {
	    __random_pitch_factor = val;
	}

	void set_drumkit_name( const QString& name ) {
	    __drumkit_name = name;
	}
	const QString& get_drumkit_name() {
	    return __drumkit_name;
	}

	inline bool is_active() {
	    return __active;
	}
	void set_active( bool active ) {
	    __active = active;
	}

	inline bool is_soloed() {
	    return __soloed;
	}
	void set_soloed( bool soloed ) {
	    __soloed = soloed;
	}
	inline void enqueue() {
	    __queued++;
	}
	inline void dequeue() {
	    assert( __queued > 0 );
	    __queued--;
	}
	inline int is_queued() {
	    return __queued;
	}

	inline bool is_stop_notes() {
	    return __stop_notes;
	}
	void set_stop_note( bool stopnotes ) {
	    __stop_notes = stopnotes;
	}



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

    inline InstrumentLayer* Instrument::get_layer( int nLayer )
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


    /**

       \brief Instrument List

    */
    class InstrumentList
    {
    public:
	typedef std::deque<Instrument*> sequence_t;
	typedef std::map<Instrument*, unsigned> map_t;

	InstrumentList();
	~InstrumentList();

	void add( Instrument* pInstrument );
	Instrument* get( unsigned int pos );
	int get_pos( Instrument* inst );
	unsigned get_size();

	void del( int pos );

	void replace( Instrument* pNewInstr, unsigned nPos );

    private:
	sequence_t m_list;
	map_t m_posmap;
    };

} // namespace Tritium

#endif // TRITIUM_INSTRUMENT_HPP
