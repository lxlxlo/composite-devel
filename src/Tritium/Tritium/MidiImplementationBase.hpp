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
#ifndef TRITIUM_MIDIIMPLEMENTATIONBASE_HPP
#define TRITIUM_MIDIIMPLEMENTATIONBASE_HPP

#include <Tritium/MidiImplementation.hpp>
#include <stdint.h>
#include <cassert>

namespace Tritium
{
    /**
     * This is a helper base class for creating a MIDI Implementation.
     *
     * It provides default handlers that do nothing.  To use a
     * handler, override the functions that you need.
     */
    template <typename EventType>
    class MidiImplementationBase : public MidiImplementation<EventType>
    {
    public:
	typedef EventType event_t;

	const uint8_t ALL; // Magic number for "all channels"

	MidiImplementationBase()
	    : ALL(-1),
	      _chan(ALL)
	    {}
	virtual ~MidiImplementationBase() {}

	/**
	 * Looks at the MIDI Status byte and calls one of the handlers.
	 *
	 * If the channel is set, it filters out only the channel that
	 * is currently being listened to.
	 */
	virtual bool translate(EventType& dest, uint32_t size, const uint8_t *midi);

	/**
	 * Sets the MIDI channel to listen on [0-15].
	 * MidiImplementationBase::ALL for all.
	 */
	void channel(uint8_t chan) {
	    if(chan<16)
		_chan = chan;
	    else
		_chan = ALL;
	}

	/**
	 * Get the MIDI channel currently listening on [0-15].
	 * Default is MidiImplementationBase::ALL for all channels.
	 */
	uint8_t channel() {
	    return _chan;
	}

	/* Override the following handler classes to handle each type
	 * of message.
	 */

	virtual bool handle_note_on(EventType& dest, uint32_t size, const uint8_t *midi)
	    { return false; }
	virtual bool handle_note_off(EventType& dest, uint32_t size, const uint8_t *midi)
	    { return false; }
	virtual bool handle_aftertouch(EventType& dest, uint32_t size, const uint8_t *midi)
	    { return false; }
	virtual bool handle_control_change(EventType& dest, uint32_t size, const uint8_t *midi)
	    { return false; }
	virtual bool handle_program_change(EventType& dest, uint32_t size, const uint8_t *midi)
	    { return false; }
	virtual bool handle_channel_pressure(EventType& dest, uint32_t size, const uint8_t *midi)
	    { return false; }
	virtual bool handle_pitch_wheel(EventType& dest, uint32_t size, const uint8_t *midi)
	    { return false; }
	virtual bool handle_system_exclusive(EventType& dest, uint32_t size, const uint8_t *midi)
	    { return false; }
	virtual bool handle_quarter_frame(EventType& dest, uint32_t size, const uint8_t *midi)
	    { return false; }
	virtual bool handle_song_position_pointer(EventType& dest, uint32_t size, const uint8_t *midi)
	    { return false; }
	virtual bool handle_song_select(EventType& dest, uint32_t size, const uint8_t *midi)
	    { return false; }
	virtual bool handle_tune_request(EventType& dest, uint32_t size, const uint8_t *midi)
	    { return false; }
	virtual bool handle_clock(EventType& dest, uint32_t size, const uint8_t *midi)
	    { return false; }
	virtual bool handle_start(EventType& dest, uint32_t size, const uint8_t *midi)
	    { return false; }
	virtual bool handle_continue(EventType& dest, uint32_t size, const uint8_t *midi)
	    { return false; }
	virtual bool handle_stop(EventType& dest, uint32_t size, const uint8_t *midi)
	    { return false; }
	virtual bool handle_active_sensing(EventType& dest, uint32_t size, const uint8_t *midi)
	    { return false; }
	virtual bool handle_system_reset(EventType& dest, uint32_t size, const uint8_t *midi)
	    { return false; }
	/**
	 * Called when none of the others are called.
	 *
	 * This will be called for undefined MIDI message types (such
	 * as 0xF9) and for invalid MIDI (e.g. status bit not set,
	 * 0xF7 as the status byte.
	 */
	virtual bool handle_unknown(EventType& dest, uint32_t size, const uint8_t *midi)
	    { return false; }

    protected:
	uint8_t _chan; //< -1 to listen on all channels.
    };


    template <typename EventType>
    bool MidiImplementationBase<EventType>::translate(EventType& dest, uint32_t size, const uint8_t *midi)
    {
	if(size == 0) return false;
	if( !(midi[0] & 0x80) ) return false; // Invalid MIDI, status bit not set.

	uint8_t command = midi[0] & 0xF0;
	uint8_t channel = ALL;
	if(command == 0xF0) {
	    command = midi[0];
	} else {
	    channel = midi[0] & 0x0F;
	}

	if( (_chan != ALL) && (channel != ALL) && (_chan != channel) ) {
	    return false;
	}

	bool rv = false;
	bool degenerate = false;
	switch(command) {
	case 0x80: // Note Off
	    if(size != 3) {
		degenerate = true;
		break;
	    }
	    rv = handle_note_off(dest, size, midi);
	    break;
	case 0x90: // Note On
	    if(size != 3) {
		degenerate = true;
		break;
	    }
	    rv = handle_note_on(dest, size, midi);
	    break;
	case 0xA0: // Aftertouch
	    if(size != 3) {
		degenerate = true;
		break;
	    }
	    rv = handle_aftertouch(dest, size, midi);
	    break;
	case 0xB0: // Control Change
	    if(size != 3) {
		degenerate = true;
		break;
	    }
	    rv = handle_control_change(dest, size, midi);
	    break;
	case 0xC0: // Program (patch) change
	    if(size != 2) {
		degenerate = true;
		break;
	    }
	    rv = handle_program_change(dest, size, midi);
	    break;
	case 0xD0: // Channel Pressure
	    if(size != 2) {
		degenerate = true;
		break;
	    }
	    rv = handle_channel_pressure(dest, size, midi);
	    break;
	case 0xE0: // Pitch Wheel
	    if(size != 3) {
		degenerate = true;
		break;
	    }
	    rv = handle_pitch_wheel(dest, size, midi);
	    break;
	case 0xF0: // System Exclusive Start
	    rv = handle_system_exclusive(dest, size, midi);
	    break;
	case 0xF1: // MTC Quarter Frame
	    if(size != 2) {
		degenerate = true;
		break;
	    }
	    rv = handle_quarter_frame(dest, size, midi);
	    break;
	case 0xF2: // Song Position Pointer
	    if(size != 3) {
		degenerate = true;
		break;
	    }
	    rv = handle_song_position_pointer(dest, size, midi);
	    break;
	case 0xF3: // Song Select
	    if(size != 2) {
		degenerate = true;
		break;
	    }
	    rv = handle_song_select(dest, size, midi);
	    break;
	case 0xF4: // *undefined*
	    rv = handle_unknown(dest, size, midi);
	    break;
	case 0xF5: // *undefined*
	    rv = handle_unknown(dest, size, midi);
	    break;
	case 0xF6: // Tune
	    if(size != 1) {
		degenerate = true;
		break;
	    }
	    rv = handle_tune_request(dest, size, midi);
	    break;
	case 0xF7: // System Exclusive End
	    rv = handle_unknown(dest, size, midi);
	    break;
	case 0xF8: // Midi Clock Pulse
	    if(size != 1) {
		degenerate = true;
		break;
	    }
	    rv = handle_clock(dest, size, midi);
	    break;
	case 0xF9: // *undefined*
	    rv = handle_unknown(dest, size, midi);
	    break;
	case 0xFA: // Start
	    if(size != 1) {
		degenerate = true;
		break;
	    }
	    rv = handle_start(dest, size, midi);
	    break;
	case 0xFB: // Continue
	    if(size != 1) {
		degenerate = true;
		break;
	    }
	    rv = handle_continue(dest, size, midi);
	    break;
	case 0xFC: // Stop
	    if(size != 1) {
		degenerate = true;
		break;
	    }
	    rv = handle_stop(dest, size, midi);
	    break;
	case 0xFD: // *unknown*/
	    rv = handle_unknown(dest, size, midi);
	    break;
	case 0xFE: // Active Sensing
	    if(size != 1) {
		degenerate = true;
		break;
	    }
	    rv = handle_active_sensing(dest, size, midi);
	    break;
	case 0xFF: // System Reset (Panic)
	    /* This should be size == 1, but if 0xFF is so
	     * important that we'll handle it anyway.
	     */
	    rv = handle_system_reset(dest, size, midi);
	    break;
	default:
	    assert(false);
	};

	if(degenerate) {
	    rv = handle_unknown(dest, size, midi);
	}

	return rv;
    }

} // namespace Tritium

#endif // TRITIUM_MIDIIMPLEMENTATIONBASE_HPP
