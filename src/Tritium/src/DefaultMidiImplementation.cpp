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
	  _ignore_note_off(true)
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

} // namespace Tritium
