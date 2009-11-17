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

#include <hydrogen/SeqEvent.h>

using namespace H2Core;

// NOTE: SeqEvent is fully defined in the header SeqEvent.h

bool SeqEvent::operator==(const SeqEvent& o) const
{
    return ((frame == o.frame)
	    && (type == o.type)
	    && (quantize == o.quantize)
	    && (instrument_index == o.instrument_index)
	    && (note.get_instrument() == o.note.get_instrument())
	    && (note.get_velocity() == o.note.get_velocity()) );
}

bool SeqEvent::operator!=(const SeqEvent& o) const
{
    return ((frame != o.frame)
	    || (type != o.type)
	    || (quantize != o.quantize)
	    || (instrument_index != o.instrument_index)
	    || (note.get_instrument() != o.note.get_instrument())
	    || (note.get_velocity() != o.note.get_velocity()) );
}

bool SeqEvent::operator<(const SeqEvent& o) const
{
    return (frame < o.frame);
}

bool H2Core::less(const SeqEvent& a, const SeqEvent& b)
{
    return a < b;
}