/*
 * Hydrogen
 * Copyright(c) 2002-2008 by Alex >Comix< Cominu [comix@users.sourceforge.net]
 *
 * http://www.hydrogen-music.org
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY, without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */
#ifndef H2CORE_SEQEVENT_H
#define H2CORE_SEQEVENT_H

#include <stdint.h>  // int32_t, uint32_t
#include <hydrogen/note.h>

namespace H2Core
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
    };

    bool less(SeqEvent a, SeqEvent b) {
	return a.frame < b.frame;
    }

} // namespace H2Core

#endif // H2CORE_SEQEVENT_H
