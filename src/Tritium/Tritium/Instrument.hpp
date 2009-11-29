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

#ifndef TRITIUM_INSTRUMENT_HPP
#define TRITIUM_INSTRUMENT_HPP

class QString;

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
	class InstrumentPrivate;

	Instrument(
	    const QString& id,
	    const QString& name,
	    ADSR* adsr
	    );
	
	static Instrument * create_empty();
	~Instrument();

	static Instrument* load_instrument(
	    const QString& drumkit_name,
	    const QString& instrument_name
	    );
	void load_from_placeholder(
	    Instrument* placeholder,
	    bool is_live = true
	    );
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
	InstrumentPrivate *d;
    };

} // namespace Tritium

#endif // TRITIUM_INSTRUMENT_HPP
