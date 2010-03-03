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
#ifndef TRITIUM_DEFAULTMIDIIMPLEMENTATION_HPP
#define TRITIUM_DEFAULTMIDIIMPLEMENTATION_HPP

#include <Tritium/MidiImplementationBase.hpp>
#include <Tritium/SeqEvent.hpp>
#include <Tritium/memory.hpp>

namespace Tritium
{
    class Sampler;

    /**
     * Interface for translating MIDI to some other kind of event.
     */
    class DefaultMidiImplementation : public MidiImplementationBase<SeqEvent>
    {
    public:
	DefaultMidiImplementation();
	virtual ~DefaultMidiImplementation();

	// Internal handlers.
	bool handle_note_off(SeqEvent& dest, uint32_t size, const uint8_t *midi);
	bool handle_note_on(SeqEvent& dest, uint32_t size, const uint8_t *midi);
	bool handle_control_change(SeqEvent& dest, uint32_t size, const uint8_t *midi);

	T<Sampler>::shared_ptr sampler() {
	    return _sampler;
	}
	void sampler(T<Sampler>::shared_ptr samp) {
	    _sampler = samp;
	}

	bool ignore_note_off() {
	    return _ignore_note_off;
	}

	void ignore_note_off(bool yes) {
	    _ignore_note_off = yes;
	}

    private:
	uint8_t _note_min; //< Minimum note number
	T<Sampler>::shared_ptr _sampler;
	bool _ignore_note_off;
	uint16_t _volume; //< 14-bit volume
    };

} // namespace Tritium

#endif // TRITIUM_DEFAULTMIDIIMPLEMENTATION_HPP
