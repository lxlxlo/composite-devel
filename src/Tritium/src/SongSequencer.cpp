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

#include <cassert>
#include <QtCore/QMutexLocker>

#include <Tritium/Hydrogen.hpp>
#include <Tritium/TransportPosition.hpp>
#include <Tritium/Song.hpp>
#include <Tritium/SeqScript.hpp>
#include <Tritium/Pattern.hpp>
#include <Tritium/Instrument.hpp>
#include <Tritium/InstrumentList.hpp>

#include "SongSequencer.hpp"

using namespace Tritium;

SongSequencer::SongSequencer() :
    m_pSong( 0 )
{
}

SongSequencer::~SongSequencer()
{
}

void SongSequencer::set_current_song(Song* pSong)
{
    QMutexLocker mx(&m_mutex);
    m_pSong = pSong;
}

// This loads up song events into the SeqScript 'seq'.
#warning "audioEngine_song_sequence_process() does not have any lookahead implemented."
#warning "audioEngine_song_sequence_process() does not have pattern mode."
int SongSequencer::process(SeqScript& seq, const TransportPosition& pos, uint32_t nframes, bool& pattern_changed)
{
    QMutexLocker mx(&m_mutex);

    if( m_pSong == 0 ) return 0;

	Song* pSong = m_pSong;
	TransportPosition cur;
	uint32_t end_frame = pos.frame + nframes;  // 1 past end of this process() cycle
	uint32_t this_tick;
	Note* pNote;
	SeqEvent ev;
	uint32_t pat_grp;
	PatternList* patterns;
	Pattern::note_map_t::const_iterator n;
	int k;
	uint32_t default_note_length, length;

	pattern_changed = false;

	if( m_pSong == 0 ) {
		return 0;
	}
	if( pos.state != TransportPosition::ROLLING) {
		return 0;
	}

	cur = pos;
	cur.ceil(TransportPosition::TICK);
	// Default note length is -1, meaning "play till there's no more sample."
	default_note_length = (uint32_t)-1;

	while( cur.frame < end_frame ) {
		this_tick = cur.tick_in_bar();
		if( this_tick == 0 ) {
			pattern_changed = true;
		}
		pat_grp = pSong->pattern_group_index_for_bar(pos.bar);
		patterns = pSong->get_pattern_group_vector()->at(pat_grp);

		for( k=0 ; unsigned(k) < patterns->get_size() ; ++k ) {
			for( n = patterns->get(k)->note_map.begin() ;
			     n != patterns->get(k)->note_map.end() ;
			     ++n ) {
				if( n->first != this_tick ) continue;
				pNote = n->second;
				ev.frame = cur.frame - pos.frame;
				ev.type = SeqEvent::NOTE_ON;
				ev.note = *pNote;
				ev.instrument_index =
					Hydrogen::get_instance()->getSong()
					->get_instrument_list()->get_pos( pNote->get_instrument() );
				if( pNote->get_length() < 0 ) {
					length = default_note_length;
				} else {
					length = unsigned(pNote->get_length()) * cur.frames_per_tick();
				}
				seq.insert_note(ev, length);
			}
		}
		++cur;
	}

	return 0;
}
