/*
 * Copyright(c) 2010 by Gabriel M. Beddingfield <gabriel@teuton.org>
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

#include <Tritium/DefaultMidiImplementation.hpp>
#include <Tritium/SeqEvent.hpp>
#include <Tritium/Instrument.hpp>
#include <Tritium/InstrumentList.hpp>
#include <Tritium/Sampler.hpp>

#include <cassert>

namespace Tritium
{
    DefaultMidiImplementation::DefaultMidiImplementation()
	: _note_min(36),
	  _ignore_note_off(true),
	  _volume(0x3FFF),
	  _bank(0)
    {
    }

    DefaultMidiImplementation::~DefaultMidiImplementation()
    {
    }

    bool DefaultMidiImplementation::handle_note_off(
	SeqEvent& dest,
	uint32_t size,
	const uint8_t *midi
	)
    {
	if(_ignore_note_off) return false;

	assert(size == 3);
	assert(0x80 == (midi[0] & 0xF0));

	uint32_t note_no;
	note_no = midi[1];
	if(note_no < _note_min) {
	    return false;
	} else {
	    note_no -= _note_min;
	}

	T<Sampler>::shared_ptr samp = _sampler;
	if( !samp ) return false;
	T<InstrumentList>::shared_ptr i_list = samp->get_instrument_list();
	T<Instrument>::shared_ptr inst;
	if( note_no < i_list->get_size() ) {
	    inst = i_list->get(note_no);
	}

	bool rv = false;
	if(inst) {
	    dest.type = SeqEvent::NOTE_OFF;
	    dest.note.set_velocity(0.0f);
	    dest.note.set_instrument(inst);
	    rv = true;
	}
	return rv;
    }

    bool DefaultMidiImplementation::handle_note_on(
	SeqEvent& dest,
	uint32_t size,
	const uint8_t *midi
	)
    {
	assert(size == 3);
	assert(0x90 == (midi[0] & 0xF0));

	uint32_t note_no;
	float velocity;

	note_no = midi[1];
	if(note_no < _note_min) {
	    return false;
	} else {
	    note_no -= _note_min;
	}

	if( midi[2] == 0 ) {
	    return handle_note_off(dest, size, midi);
	}
	velocity = float(midi[2]) / 127.0f;

	T<Sampler>::shared_ptr samp = _sampler;
	if( !samp ) return false;
	T<InstrumentList>::shared_ptr i_list = samp->get_instrument_list();
	T<Instrument>::shared_ptr inst;
	if( note_no < i_list->get_size() ) {
	    inst = i_list->get(note_no);
	}

	bool rv = false;
	if(inst) {
	    dest.type = SeqEvent::NOTE_ON;
	    dest.note.set_velocity(velocity);
	    dest.note.set_instrument(inst);
	    dest.note.set_length(-1);
	    rv = true;
	}
	return rv;	    
    }

    bool DefaultMidiImplementation::handle_control_change(
	SeqEvent& dest,
	uint32_t size,
	const uint8_t *midi
	)
    {
	assert(size == 3);
	assert(0xB0 == (midi[0] & 0xF0));
	const uint16_t coarse_mask = 0x3F80;
	const uint16_t fine_mask = 0x7F;

	bool rv = false;
	const uint8_t& controller = midi[1];
	const uint8_t& value = midi[2];

	/******************************************************
	 * IF YOU HANDLE AN EVENT THE RESULTS IN A SeqEvent,
	 * BE SURE TO SET 'rv' TO TRUE
	 *******************************************************
	 */
	switch(controller) {
	case 0: // Bank (coarse)
	    _bank = (_bank & fine_mask) | ((value << 7) & coarse_mask);
	    rv = false;
	    break;
	case 7: // Volume (coarse)
	    /* XXX TODO: Consider using an exponential taper on this.
	     */
	    _volume = (_volume & fine_mask) | ((value << 7) & coarse_mask);
	    dest.type = SeqEvent::VOL_UPDATE;
	    dest.fdata = float(_volume)/16383.0f;
	    rv = true;
	    break;
	case 8: // Balance (coarse)
	    break;
	case 10: // Pan position (coarse)
	    break;
	case 11: // Expression (coarse)
	    break;
	case 32: // Bank (fine)
	    _bank = (_bank & coarse_mask) | (value & fine_mask);
	    rv = false;
	    break;
	case 39: // Volume (fine)
	    /* XXX TODO: Consider using an exponential taper on this.
	     */
	    _volume = (_volume & coarse_mask) | (value & fine_mask);
	    dest.type = SeqEvent::VOL_UPDATE;
	    dest.fdata = float(_volume)/16383.0f;
	    rv = true;
	    break;
	case 120: // All Sound Off
	    dest.type = SeqEvent::ALL_OFF;
	    rv = true;
	    break;
	case 121: // All Controllers Off (go to default values)
	    break;
	case 123: // All Notes Off
	    dest.type = SeqEvent::ALL_OFF;
	    rv = true;
	    break;
	case 124: // Omni Mode Off
	    break;
	case 125: // Omni Mode On
	    break;
	default:
	    rv = false;
	}

	assert( _volume == (_volume & 0x3FFF) );

	return rv;
    } // DefaultMidiImplementation::handle_control_change()

    bool DefaultMidiImplementation::handle_program_change(
	SeqEvent& dest,
	uint32_t size,
	const uint8_t *midi
	)
    {
	assert(size == 2);
	assert(0xC0 == (midi[0] & 0xF0));

	bool rv = false;

	dest.type = SeqEvent::PATCH_CHANGE;
	dest.idata = ((uint32_t(_bank) & 0x3FFF) << 16) | midi[1];
	rv = true;

	return rv;
    } // DefaultMidiImplementation::handle_program_change()

    bool DefaultMidiImplementation::handle_system_reset(
	SeqEvent& dest,
	uint32_t size,
	const uint8_t *midi
	)
    {
	/* XXX TODO A system reset should set us back to a "default
	 * state."  (I.e. set volume and banks to default, set patch
	 * to default, etc.)  However, this requires a new SeqEvent
	 * type.  This will need to be done later.
	 */
	dest.type = SeqEvent::ALL_OFF;
	return true;
    }
} // namespace Tritium
