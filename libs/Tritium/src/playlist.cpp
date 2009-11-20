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

#include "gui/src/HydrogenApp.h"
#include "gui/src/InstrumentRack.h"
#include "gui/src/SoundLibrary/SoundLibraryPanel.h"

#include <Tritium/LocalFileMng.h>
#include <Tritium/h2_exception.h>
#include <Tritium/Preferences.h>
#include <Tritium/hydrogen.h>
#include <Tritium/Transport.h>
#include <Tritium/playlist.h>

#include <vector>
#include <cstdlib>



using namespace Tritium;

//playlist globals
int selectedSongNumber = -1;
int activeSongNumber = -1;

Playlist* Playlist::__instance = NULL;	


Playlist::Playlist()
		: Object( "Playlist" )
{
	if ( __instance ) {class HydrogenApp;

		_ERRORLOG( "Playlist in use" );
	}class HydrogenApp;

	//_INFOLOG( "[Playlist]" );
	__instance = this;
	__playlistName = "";

}



Playlist::~Playlist()
{
	//_INFOLOG( "[~Playlist]" );
	__instance = NULL;
}



void Playlist::create_instance()
{
	if ( __instance == 0 ) {
		__instance = new Playlist;
	}
}



void Playlist::setNextSongByNumber(int SongNumber)
{
	
	int realNumber = SongNumber;
	
	if ( realNumber > (int)Hydrogen::get_instance()->m_PlayList.size() -1 || (int)Hydrogen::get_instance()->m_PlayList.size() == 0 )
		return;	

	setSelectedSongNr(  realNumber );
	setActiveSongNumber( realNumber );

	QString selected;
	selected = Hydrogen::get_instance()->m_PlayList[ realNumber ].m_hFile;

	loadSong( selected );
	execScript( realNumber );

	HydrogenApp::get_instance()->getInstrumentRack()->getSoundLibraryPanel()->update_background_color();
}



void Playlist::setNextSongPlaylist()
{
	
	int index = getSelectedSongNr();
	//_INFOLOG( "index" + to_string( index ) );
	if (index == -1 ){
		if ( getActiveSongNumber() != -1){
			index = getActiveSongNumber();
		}else
		{
			return;
		}
	}

	index = index +1;
	if ( (int) index > (int)Hydrogen::get_instance()->m_PlayList.size() -1 || index < 0) 
		return;
	setSelectedSongNr( index );
	setActiveSongNumber( index );

	QString selected;
	selected = Hydrogen::get_instance()->m_PlayList[ index ].m_hFile;

	loadSong( selected );
	execScript( index );

	HydrogenApp::get_instance()->getInstrumentRack()->getSoundLibraryPanel()->update_background_color();
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
	selected = Hydrogen::get_instance()->m_PlayList[ index ].m_hFile;

	loadSong( selected );
	execScript( index );

	HydrogenApp::get_instance()->getInstrumentRack()->getSoundLibraryPanel()->update_background_color();
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

	HydrogenApp *pH2App = HydrogenApp::get_instance();
	Hydrogen *engine = Hydrogen::get_instance();
	

	engine->get_transport()->stop();

	LocalFileMng mng;
	Song *pSong = Song::load ( songName );
	if ( pSong == NULL ){
		return;
	}

	pH2App->setSong ( pSong );
	engine->setSelectedPatternNumber ( 0 );
}



void Playlist::execScript( int index)
{
	QString file;
	QString script;

	file = Hydrogen::get_instance()->m_PlayList[ index ].m_hScript;
	script = Hydrogen::get_instance()->m_PlayList[ index ].m_hScriptEnabled;

	if( file == "no Script" || script == "Script not used")
		return;

	std::system( file.toLocal8Bit() );

	return;
}
