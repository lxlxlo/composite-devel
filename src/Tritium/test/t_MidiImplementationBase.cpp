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

/**
 * t_MidiImplementationBase.cpp
 *
 * Test both MidiImplementation<T> and MidiImplementationBase<T>.
 */

#include <Tritium/MidiImplementationBase.hpp>
#include <cstdlib>

// CHANGE THIS TO MATCH YOUR FILE:
#define THIS_NAMESPACE t_MidiImplementationBase
#include "test_macros.hpp"
#include "test_config.hpp"

using namespace Tritium;

namespace THIS_NAMESPACE
{

    struct Fixture
    {
	// SETUP AND TEARDOWN OBJECTS FOR YOUR TESTS.

	Fixture() {}
	~Fixture() {}
    };

    typedef enum {
	NONE = 0,
	NOTE_ON,
	NOTE_OFF,
	AFTERTOUCH,
	CONTROL_CHANGE,
	PROGRAM_CHANGE,
	CHANNEL_PRESSURE,
	PITCH_WHEEL,
	SYSTEM_EXCLUSIVE,
	QUARTER_FRAME,
	SONG_POSITION_POINTER,
	SONG_SELECT,
	TUNE_REQUEST,
	CLOCK,
	START,
	CONTINUE,
	STOP,
	ACTIVE_SENSING,
	SYSTEM_RESET,
	UNKNOWN
    } TestEv;

    class TestImp : public MidiImplementationBase<TestEv>
    {
    public:

	bool handle_note_on(TestEv& dest, uint32_t size, const uint8_t *midi) {
	    dest = NOTE_ON;
	    return true;
	}
	bool handle_note_off(TestEv& dest, uint32_t size, const uint8_t *midi) {
	    dest = NOTE_OFF;
	    return true;
	}
	bool handle_aftertouch(TestEv& dest, uint32_t size, const uint8_t *midi) {
	    dest = AFTERTOUCH;
	    return true;
	}
	bool handle_control_change(TestEv& dest, uint32_t size, const uint8_t *midi) {
	    dest = CONTROL_CHANGE;
	    return true;
	}
	bool handle_program_change(TestEv& dest, uint32_t size, const uint8_t *midi) {
	    dest = PROGRAM_CHANGE;
	    return true;
	}
	bool handle_channel_pressure(TestEv& dest, uint32_t size, const uint8_t *midi) {
	    dest = CHANNEL_PRESSURE;
	    return true;
	}
	bool handle_pitch_wheel(TestEv& dest, uint32_t size, const uint8_t *midi) {
	    dest = PITCH_WHEEL;
	    return true;
	}
	bool handle_system_exclusive(TestEv& dest, uint32_t size, const uint8_t *midi) {
	    dest = SYSTEM_EXCLUSIVE;
	    return true;
	}
	bool handle_quarter_frame(TestEv& dest, uint32_t size, const uint8_t *midi) {
	    dest = QUARTER_FRAME;
	    return true;
	}
	bool handle_song_position_pointer(TestEv& dest, uint32_t size, const uint8_t *midi) {
	    dest = SONG_POSITION_POINTER;
	    return true;
	}
	bool handle_song_select(TestEv& dest, uint32_t size, const uint8_t *midi) {
	    dest = SONG_SELECT;
	    return true;
	}
	bool handle_tune_request(TestEv& dest, uint32_t size, const uint8_t *midi) {
	    dest = TUNE_REQUEST;
	    return true;
	}
	bool handle_clock(TestEv& dest, uint32_t size, const uint8_t *midi) {
	    dest = CLOCK;
	    return true;
	}
	bool handle_start(TestEv& dest, uint32_t size, const uint8_t *midi) {
	    dest = START;
	    return true;
	}
	bool handle_continue(TestEv& dest, uint32_t size, const uint8_t *midi) {
	    dest = CONTINUE;
	    return true;
	}
	bool handle_stop(TestEv& dest, uint32_t size, const uint8_t *midi) {
	    dest = STOP;
	    return true;
	}
	bool handle_active_sensing(TestEv& dest, uint32_t size, const uint8_t *midi) {
	    dest = ACTIVE_SENSING;
	    return true;
	}
	bool handle_system_reset(TestEv& dest, uint32_t size, const uint8_t *midi) {
	    dest = SYSTEM_RESET;
	    return true;
	}
	bool handle_unknown(TestEv& dest, uint32_t size, const uint8_t *midi) {
	    dest = UNKNOWN;
	    return true;
	}

    }; // class TestImp

    /**
     * Returns a random, but valid status byte.
     */
    static uint8_t rand_status() {
	uint8_t rv = rand();
	return rv | 0x80;
    }

    static uint32_t get_midi_size(uint8_t status, uint32_t max) {
	uint8_t word = status & 0xF0;
	if(word == 0xF0) word = status;

	uint32_t rv = 1;

	switch(word) {
	case 0x80: // Note Off
	    rv = 3;
	    break;
	case 0x90: // Note On
	    rv = 3;
	    break;
	case 0xA0: // Aftertouch
	    rv = 3;
	    break;
	case 0xB0: // Control Change
	    rv = 3;
	    break;
	case 0xC0: // Program (patch) change
	    rv = 2;
	    break;
	case 0xD0: // Channel Pressure
	    rv = 2;
	    break;
	case 0xE0: // Pitch Wheel
	    rv = 3;
	    break;
	case 0xF0: // System Exclusive Start
	    rv = 2;
	    break;
	case 0xF1: // MTC Quarter Frame
	    rv = 2;
	    break;
	case 0xF2: // Song Position Pointer
	    rv = 3;
	    break;
	case 0xF3: // Song Select
	    rv = 2;
	    break;
	case 0xF4: // *undefined*
	    rv = 1;
	    break;
	case 0xF5: // *undefined*
	    rv = 1;
	    break;
	case 0xF6: // Tune
	    rv = 1;
	    break;
	case 0xF7: // System Exclusive End
	    rv = 1;
	    break;
	case 0xF8: // Midi Clock Pulse
	    rv = 1;
	    break;
	case 0xF9: // *undefined*
	    rv = 1;
	    break;
	case 0xFA: // Start
	    rv = 1;
	    break;
	case 0xFB: // Continue
	    rv = 1;
	    break;
	case 0xFC: // Stop
	    rv = 1;
	    break;
	case 0xFD: // *unknown*/
	    rv = 1;
	    break;
	case 0xFE: // Active Sensing
	    rv = 1;
	    break;
	case 0xFF: // System Reset (Panic)
	    rv = 1;
	    break;
	default:
	    rv = 1;
	}
	return rv;
    }

} // namespace THIS_NAMESPACE

TEST_BEGIN( Fixture );

TEST_CASE( 010_defaults )
{
    MidiImplementationBase<int> m;
    int b, a;
    uint8_t midi[] = { 0x92, 64, 101 };

    int k = 128;
    while(k--) {
	b = a = rand();
	midi[0] = rand_status();
	CK( false == m.translate(b, 3, midi) );
	CK( b == a );
    }
}

TEST_CASE( 020_channel_setting )
{
    TestImp m;
    TestEv ev;
    uint8_t midi[] = { 0x81, 16, 120 };
    uint32_t size;

    m.channel(m.ALL);
    CK(m.channel() == m.ALL);
    int k = 128;
    while(k--) {
	ev = NONE;
	midi[0] = rand_status();
	size = get_midi_size(midi[0], 3);
	CK(true == m.translate(ev, size, midi));
	CK(ev != NONE);
    }

    uint8_t chan, t_chan;
    chan = 8;
    m.channel(chan);
    CK(m.channel() == 8);
    k = 1024;
    while(k--) {
	ev = NONE;
	midi[0] = (rand() % 8) | 0x08;
	t_chan = rand() % 16;
	midi[0] = (midi[0] << 4) | t_chan;
	size = get_midi_size(midi[0], 3);
	if(((midi[0] & 0xF0) == 0xF0) || (chan == t_chan)) {
	    CK(true == m.translate(ev, size, midi));
	    CK(ev != NONE);
	} else {
	    CK(false == m.translate(ev, size, midi));
	    CK(ev == NONE);
	}
    }
}

TEST_END()
