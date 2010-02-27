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
#ifndef TRITIUM_MIDIIMPLEMENTATION_HPP
#define TRITIUM_MIDIIMPLEMENTATION_HPP

#include <stdint.h>  // int32_t, uint32_t

namespace Tritium
{
    /**
     * Interface for translating MIDI to some other kind of event.
     */
    template <typename EventType>
    class MidiImplementation
    {
    public:
	virtual ~MidiImplementation() {}

	/**
	 * Translates a raw midi byte sequence to another kind of event.
	 *
	 * \param dest The destination event.  It needs to already
	 * have all of its default parameters, because this function
	 * may only manipulate some parts of the event.  For example,
	 * if this is an instance of MidiImplementation<SeqEvent>,
	 * then the calling funtion will want to set SeqEvent::frame
	 * before or after calling this function (since raw MIDI will
	 * typically have no implication on the frame number).
	 *
	 * \param size The size of the midi event string.
	 *
	 * \param midi The raw midi data.  The first 'size' bytes must
	 * be valid.  This must be a normalized MIDI message.
	 * Broken-up Sysex Messages, or messages with intersparsed
	 * 1-byte MIDI Realtime messages are not allowed.  Likewise,
	 * running statuses (like pitch wheel) need to be broken up
	 * into individual, normalized midi messages, each with their
	 * own status byte.  If this happens, the results are
	 * undefined.  In the case of a Sysex start/end (0xF0, 0xF7),
	 * the last byte of the midi parameter should be 0xF7.
	 *
	 * \return True if the MIDI message was translated to the
	 * event type.  False if this message is unhandled.
	 */
	virtual bool translate(EventType& dest, uint32_t size, const uint8_t *midi) = 0;
    };

} // namespace Tritium

#endif // TRITIUM_MIDIIMPLEMENTATION_HPP
