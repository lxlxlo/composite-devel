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

#include "songhelpers.h"

#include <Tritium/Song.h>
#include <Tritium/Pattern.h>

#include <vector>

using namespace H2Core;

typedef std::vector<PatternList*> pgrp_list;
typedef pgrp_list::iterator pgrp_list_iter;

uint32_t H2Core::song_bar_count(Song* s)
{
    if( s == 0 ) return -1;
    return s->get_pattern_group_vector()->size();
}

uint32_t H2Core::song_tick_count(Song* s)
{
    if( s == 0 ) return -1;
    uint32_t count = 0;
    uint32_t bar = 1;
    uint32_t tmp;

    tmp = ticks_in_bar(s, bar);
    while( tmp != unsigned(-1) ) {
        count += tmp;
        ++bar;
        tmp = ticks_in_bar(s, bar);
    }
    return count;
}

uint32_t H2Core::pattern_group_index_for_bar(Song* s, uint32_t bar)
{
    if( s == 0 ) return -1;
    if( bar <= song_bar_count(s) ) {
        return bar-1;
    }
    return -1;
}

uint32_t H2Core::bar_for_absolute_tick(Song* s, uint32_t abs_tick)
{
    if( s == 0 ) return -1;
    uint32_t tick_count = abs_tick;
    uint32_t bar_count = 1;
    uint32_t tmp;

    tmp = ticks_in_bar(s, bar_count);
    while( tick_count >= tmp ) {
	tick_count -= tmp;
	++bar_count;
	tmp = ticks_in_bar(s, bar_count);
    }
    return bar_count;
}

uint32_t H2Core::bar_start_tick(Song* s, uint32_t bar)
{
    if( s == 0 ) return -1;
    if( bar > song_bar_count(s) ) return -1;
    uint32_t count = 0, k = 1;
    while( k < bar ) {
        count += ticks_in_bar(s, k);
	++k;
    }
    return count;
}

uint32_t H2Core::ticks_in_bar(Song* s, uint32_t bar)
{
    if( s == 0 ) return -1;
    if( bar < 1 ) return -1;
    if( bar > song_bar_count(s) ) return -1;

    PatternList* list = s->get_pattern_group_vector()->at(bar-1);
    uint32_t j;
    uint32_t max_ticks = 0;
    uint32_t tmp;
    for( j = 0 ; j < list->get_size() ; ++j ) {
        tmp = list->get(j)->get_length();
        if( tmp > max_ticks ) {
            max_ticks = tmp;
        }
    }

    return max_ticks;
}
