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
#ifndef TRITIUM_PATTERNMODEMANAGER_HPP
#define TRITIUM_PATTERNMODEMANAGER_HPP

#include <Tritium/Song.hpp>

namespace Tritium
{
    /**
     * /brief Internal class for Tritium::Song.
     *
     */
    class PatternModeManager
    {
    public:
	typedef PatternModeList PatternModeList_t;

	PatternModeManager();

	Song::PatternModeType get_pattern_mode_type();
	void set_pattern_mode_type(Song::PatternModeType t);
	void toggle_pattern_mode_type();

	// Manipulate the pattern lists and queues.
	// Patterns may only be added/removed once, so subsequent add/remove
	// operations will have no affect.
	// If 'pos' is not in the range 0 <= pos <= __pattern_list->get_size(),
	// these will silently ignore the request.
	void append_pattern(int pos);      // Appends pattern to the current group on next cycle.
	void remove_pattern(int pos);      // Remove the pattern from the current group on next cycle.
	void reset_patterns();             // Clears out the current and "next" queues.
	void set_next_pattern(int pos);    // Sched. a pattern to replace the current group.
	                                   // ...clears out any that are currently queued.
	void append_next_pattern(int pos); // Adds pattern to the "next" queued patterns.
	void remove_next_pattern(int pos); // Removes pattern from the "next" queue
	void clear_queued_patterns();      // Clears out the "next" queued patterns.

	// Returns the current patterns that are playing in pattern
	// mode.  Return 0 if there are none, or we are in song mode.
	void get_playing_patterns(PatternModeList_t::list_type& pats);

	// This method should *ONLY* be used by the sequencer.
	// This signals to the Song class that the current pattern
	// is done playing, and to switch to the next pattern if
	// there are any queued.
	void go_to_next_patterns();


    private:
	Song::PatternModeType __type;
	QMutex __mutex;  // Locked when accessing more than one of the lists.
	PatternModeList_t __current;
	PatternModeList_t __append;
	PatternModeList_t __delete;
	PatternModeList_t __next;
    };

} // namespace Tritium

#endif // TRITIUM_PATTERNMODEMANAGER_HPP
