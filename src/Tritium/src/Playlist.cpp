/*
 * Copyright(c) 2002-2008 by Alex >Comix< Cominu [comix@users.sourceforge.net]
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

#include <Tritium/LocalFileMng.hpp>
#include <Tritium/H2Exception.hpp>
#include <Tritium/Preferences.hpp>
#include <Tritium/Engine.hpp>
#include <Tritium/Transport.hpp>
#include <Tritium/Playlist.hpp>

#include <vector>
#include <cstdlib>
#include <QMutexLocker>


using namespace Tritium;

//playlist globals
int selectedSongNumber = -1;
int activeSongNumber = -1;

Playlist::Playlist(Engine* parent) :
	m_engine(parent),
	m_listener(0)
{
	assert(parent);
	//INFOLOG( "[Playlist]" );
	__playlistName = "";

}



Playlist::~Playlist()
{
}

void Playlist::subscribe(PlaylistListener* listener)
{
	QMutexLocker lock(&m_listener_mutex);
	m_listener = listener;
}

void Playlist::unsubscribe()
{
	subscribe(0);
}

void Playlist::setNextSongByNumber(int SongNumber)
{
	
	int realNumber = SongNumber;
	
	if ( realNumber > (int)m_engine->get_internal_playlist().size() -1 || (int)m_engine->get_internal_playlist().size() == 0 )
		return;	

	setSelectedSongNr(  realNumber );
	setActiveSongNumber( realNumber );

	QString selected;
	selected = m_engine->get_internal_playlist()[ realNumber ].m_hFile;

	loadSong( selected );
	execScript( realNumber );

	if(m_listener)
		m_listener->selection_changed();
}



void Playlist::setNextSongPlaylist()
{
	
	int index = getSelectedSongNr();
	//INFOLOG( "index" + to_string( index ) );
	if (index == -1 ){
		if ( getActiveSongNumber() != -1){
			index = getActiveSongNumber();
		}else
		{
			return;
		}
	}

	index = index +1;
	if ( (int) index > (int)m_engine->get_internal_playlist().size() -1 || index < 0) 
		return;
	setSelectedSongNr( index );
	setActiveSongNumber( index );

	QString selected;
	selected = m_engine->get_internal_playlist()[ index ].m_hFile;

	loadSong( selected );
	execScript( index );

	if(m_listener)
		m_listener->selection_changed();
}



void Playlist::setPrevSongPlaylist()
{
	int index = getSelectedSongNr();

	if (index == -1 ){
		if ( getActiveSongNumber() != -1 ){
			index = getActiveSongNumber();
		}else
		{
			return;
		}
	}

	index = index -1;

	if (index < 0 ) 
		return;

	setSelectedSongNr( index );
	setActiveSongNumber( index );

	QString selected;
	selected = m_engine->get_internal_playlist()[ index ].m_hFile;

	loadSong( selected );
	execScript( index );

	if(m_listener)
		m_listener->selection_changed();
}



void Playlist::setSelectedSongNr( int songNumber )
{
	selectedSongNumber = songNumber;
}



int Playlist::getSelectedSongNr()
{
	return selectedSongNumber;
}



void Playlist::setActiveSongNumber( int ActiveSongNumber)
{
	 activeSongNumber = ActiveSongNumber ;
}



int Playlist::getActiveSongNumber()
{
	return activeSongNumber;
}



void Playlist::loadSong( QString songName )
{
	m_engine->get_transport()->stop();

	T<Song>::shared_ptr pSong = Song::load ( m_engine, songName );
	if ( ! pSong ){
		return;
	}

	if(m_listener)
		m_listener->set_song(pSong);
	m_engine->setSelectedPatternNumber ( 0 );
}



void Playlist::execScript( int index)
{
	QString file;
	QString script;

	file = m_engine->get_internal_playlist()[ index ].m_hScript;
	script = m_engine->get_internal_playlist()[ index ].m_hScriptEnabled;

	if( file == "no Script" || script == "Script not used")
		return;

	std::system( file.toLocal8Bit() );

	return;
}
