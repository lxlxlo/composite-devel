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

#include <hydrogen/SeqScript.h>
#include <hydrogen/SeqEvent.h>
#include <hydrogen/SeqScriptIterator.h>

#include <cassert>

#include "SeqScriptPrivate.h"

using namespace H2Core;

SeqScript::SeqScript()
{
    d = new SeqScriptPrivate();
}

SeqScript::~SeqScript()
{
    delete d;
}

/**
 * Properties
 */
/// True if there are no events.  False otherwise.
bool SeqScript::empty() const
{
    return d->empty();
}

/// Returns the number of events stored.
SeqScript::size_type SeqScript::size() const
{
    return d->size();
}

/// Returns the number of events before frame 'before_frame'
SeqScript::size_type SeqScript::size(SeqScript::frame_type before_frame) const
{
    SeqScript::size_type cnt = 0;
    SeqScriptPrivate::iterator cur;
    for( cur = d->begin() ; (cur != d->end()) && (cur->ev.frame < before_frame) ; ++cur ) {
	++cnt;
    }
    return cnt;
}

/// Returns the number of SeqEvents reserved in a buffer.
SeqScript::size_type SeqScript::max_size() const
{
    return d->max_size();
}

/**
 * METHODS FOR THE SEQUENCER
 *
 * These methods should *only* be used by the sequencer inputs.  Sequencer
 * outputs should *not* use these.
 */

/// Reserves memory for a specific number of events.  This
/// will almost always reallocate memory and should not be
/// called from any realtime events.
void SeqScript::reserve(SeqScript::size_type events)
{
    d->reserve(events);
}

/// Insert an event (SeqScript handles sorting)
void SeqScript::insert(const SeqScript::value_type& event)
{
    d->insert(event);
}

/// Remove an event.  It remains to be seen if this invalidates
/// any iterators.
void SeqScript::remove(const SeqScript::value_type& event)
{
    d->remove(event);
}

void SeqScript::remove(SeqScript::value_type* event)
{
    assert(false);
}

/// Removes all the events before 'before_frame' (i.e. after
/// they have been processed.
void SeqScript::consumed(SeqScript::frame_type before_frame)
{
    d->consumed(before_frame);
}

/// Clears out all queued events.
void SeqScript::clear()
{
    d->clear();
}

/// Schedules the note on/off pair, and cancel any note-off events
/// currently between the two (unless this note is already scheduled to
/// be interrupted by another note-on event).
void SeqScript::insert_note(const SeqScript::value_type& event, SeqScript::frame_type length)
{
    SeqScript::value_type off = event;
    off.frame += length;
    off.type = SeqEvent::NOTE_OFF;
    off.note.set_velocity(0.0);
}

/// Provides a read-only view of this sequence script from frames 0 to
/// nframes.
SeqScriptConstIterator SeqScript::begin_const() const
{
    return SeqScriptConstIterator(d->begin());
}

SeqScriptConstIterator SeqScript::end_const() const
{
    return SeqScriptConstIterator(d->end());
}

SeqScriptConstIterator SeqScript::end_const(SeqScript::frame_type nframes) const
{
    SeqScriptPrivate::iterator cur;
    for( cur = d->begin() ; (cur != d->end()) && (cur->ev.frame < nframes) ; ++cur );
    return SeqScriptConstIterator(cur);
}
