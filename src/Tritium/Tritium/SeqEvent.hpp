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
        enum {
	    UNDEFINED,
	    NOTE_ON,
	    NOTE_OFF,
	    ALL_OFF,
	    VOL_UPDATE,
	    PATCH_CHANGE
	} type;

	Note note; // Valid for all NOTE_* events
	bool quantize; // Valid for all NOTE_* events
	float fdata; // Valid for all VOL_* events
	uint32_t idata; // Valid for PATCH_CHANGE

	/* For a PATCH_CHANGE, idata's bytes will be:
	   BANK_MSB, BANK_LSB, 0, PROGRAM_NO
	 */

	SeqEvent() :
	    frame(0),
	    type(UNDEFINED),
	    note(),
	    quantize(false)
	    {}

	bool operator==(const SeqEvent& o) const;
	bool operator!=(const SeqEvent& o) const;
	bool operator<(const SeqEvent& o) const;
    };

    bool less(const SeqEvent& a, const SeqEvent& b);

} // namespace Tritium

#endif // TRITIUM_SEQEVENT_HPP
