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
#ifndef TRITIUM_SEQEVENT_HPP
#define TRITIUM_SEQEVENT_HPP

#include <stdint.h>  // int32_t, uint32_t
#include <Tritium/Note.hpp>

namespace Tritium
{
    /**
     * A container that maps a frame and a note object.
     */
    struct SeqEvent
    {
        typedef uint32_t frame_type;

        frame_type frame;
        enum { NOTE_ON, NOTE_OFF, ALL_OFF } type;
	Note note;
	bool quantize;
	unsigned instrument_index;  // For tracking outputs.

	SeqEvent() :
	    frame(0),
	    type(NOTE_ON),
	    note(),
	    quantize(false),
	    instrument_index(0)
	    {}

	bool operator==(const SeqEvent& o) const;
	bool operator!=(const SeqEvent& o) const;
	bool operator<(const SeqEvent& o) const;
    };

    bool less(const SeqEvent& a, const SeqEvent& b);

} // namespace Tritium

#endif // TRITIUM_SEQEVENT_HPP
