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
#ifndef TRITIUM_SONGHELPERS_H
#define TRITIUM_SONGHELPERS_H

/**
 * This declares several functions that are useful in translating Song
 * information into bars, beats, ticks, etc.
 */

#include <stdint.h>

namespace Tritium
{
    class Song;

    /**
     * Returns the number of measures in a song.  Returns -1 if there is an
     * error (e.g. null pointer).
     */
    uint32_t song_bar_count(Song* s);

    /**
     * Returns the number of ticks in a song.  Returns -1 if there was an error
     * (s == 0).
     */
    uint32_t song_tick_count(Song* s);

    /**
     * Returns the index of the pattern group that represents measure number
     * 'bar'.  Returns -1 if bar > song_bar_count(s).
     */
    uint32_t pattern_group_index_for_bar(Song* s, uint32_t bar);

    /**
     * Returns the bar number of the pattern group that contains the given
     * absolute tick.  (Always assuming tick 0 is at 1:1.0000.  Returns -1 if
     * there is an error (s == 0) or of the tick is beyond the end of the song.
     */
    uint32_t bar_for_absolute_tick(Song* s, uint32_t abs_tick);

    /**
     * Returns the absolute tick number for the start of this bar.
     */
    uint32_t bar_start_tick(Song* s, uint32_t bar);

    /**
     * Returns the number of ticks in measure 'bar'
     */
    uint32_t ticks_in_bar(Song* s, uint32_t bar);

} // namespace Tritium

#endif // TRITIUM_SONGHELPERS_H
