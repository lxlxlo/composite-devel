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
#ifndef TRITIUM_SONGSEQUENCER_H
#define TRITIUM_SONGSEQUENCER_H

#include <stdint.h>
#include <QtCore/QMutex>

namespace Tritium
{

class Song;
struct TransportPosition;
class SeqScript;

class SongSequencer
{
public:

    SongSequencer();
    ~SongSequencer();

    void set_current_song(Song* pSong);
    int process(SeqScript& seq, const TransportPosition& pos, uint32_t nframes, bool& pattern_changed);

private:
    QMutex m_mutex;
    Song* m_pSong;
};  // class SongSequencer

} // namespace Tritium

#endif // TRITIUM_H2TRANSPORT_H
