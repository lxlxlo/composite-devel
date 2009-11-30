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

#include "songhelpers.hpp"

#include <Tritium/Song.hpp>
#include <Tritium/Pattern.hpp>

#include <vector>

using namespace Tritium;

typedef std::vector<PatternList*> pgrp_list;
typedef pgrp_list::iterator pgrp_list_iter;

uint32_t Tritium::song_bar_count(Song* s)
{
    if( s == 0 ) return -1;
    return s->song_bar_count();
}

uint32_t Tritium::song_tick_count(Song* s)
{
    if( s == 0 ) return -1;
    return s->song_tick_count();
}

uint32_t Tritium::pattern_group_index_for_bar(Song* s, uint32_t bar)
{
    if( s == 0 ) return -1;
    return s->pattern_group_index_for_bar(bar);
}

uint32_t Tritium::bar_for_absolute_tick(Song* s, uint32_t abs_tick)
{
    if( s == 0 ) return -1;
    return s->bar_for_absolute_tick(abs_tick);
}

uint32_t Tritium::bar_start_tick(Song* s, uint32_t bar)
{
    if( s == 0 ) return -1;
    return s->bar_start_tick(bar);
}

uint32_t Tritium::ticks_in_bar(Song* s, uint32_t bar)
{
    if( s == 0 ) return -1;
    return s->ticks_in_bar(bar);
}
