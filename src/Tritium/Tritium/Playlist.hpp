/*
 * Copyright(c) 2002-2008 by Alex >Comix< Cominu [comix@users.sourceforge.net]
 * Copyright (c) 2009 by Gabriel M. Beddingfield <gabriel@teuton.org>
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

#ifndef TRITIUM_PLAYLIST_HPP
#define TRITIUM_PLAYLIST_HPP

#include <QMutex>
#include <vector>
#include <cassert>

namespace Tritium
{

///
/// handle playlist  
///

    class Engine;
    class Song;

    /**
     * This class gives an interactive experience between CompositeApp and
     * the Playlist object without requiring the Playlist to have a
     * pointer to CompositeApp.
     */
    class PlaylistListener
    {
    public:
        virtual ~PlaylistListener() {}

        /**
         * Signals that the playist selection has changed
         * and any vies need to be updated.
         */
        virtual void selection_changed() = 0;
        /**
         * Signals that the current song should be
         * changed to the one pointed to by @pSong.
         */
        virtual void set_song(Song* pSong) = 0;
    };

    class Playlist
    {
        
    public:
        Playlist(Engine* parent);
        ~Playlist();

        /**
         * Subscribe to the playlist's signals.  There may be only
         * on subscriber... and this function will overwrite whoever
         * the current subscriber is.
         */
        void subscribe(PlaylistListener* listener);
        void unsubscribe();

        // std::vector<HPlayListNode> m_PlayList;
        void setNextSongByNumber(int SongNumber);
        void setNextSongPlaylist();
        void setPrevSongPlaylist();
        void setSelectedSongNr( int songNumber);

        int selectedSongNumber;
                
        int getSelectedSongNr();
        void setActiveSongNumber( int ActiveSongNumber);
        int getActiveSongNumber();

        QString __playlistName;

    private:
	Engine* m_engine;
        PlaylistListener* m_listener;
        QMutex m_listener_mutex;

        void loadSong( QString songName );
        void execScript( int index);
    };

} // namespace Tritium

#endif // TRITIUM_PLAYLIST_HPP
