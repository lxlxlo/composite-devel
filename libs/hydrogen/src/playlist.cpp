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

//#include "PlaylistDialog.h"


#include <hydrogen/LocalFileMng.h>
#include <hydrogen/h2_exception.h>
#include <hydrogen/Preferences.h>
#include <hydrogen/hydrogen.h>
#include <hydrogen/playlist.h>
#include "gui/src/HydrogenApp.h"

#include <vector>
#include <cstdlib>



using namespace H2Core;

//playlist globals
int selectedSongNumber = -1;
int activeSongNumber = -1;

Playlist* Playlist::instance = NULL;	


Playlist::Playlist()
		: Object( "Playlist" )
{
	if ( instance ) {class HydrogenApp;

		_ERRORLOG( "Playlist in use" );
	}class HydrogenApp;

	_INFOLOG( "[Playlist]" );
	instance = this;

}



Playlist::~Playlist()
{
	_INFOLOG( "[~Playlist]" );
	instance = NULL;
}



Playlist* Playlist::get_instance()
{
	if ( instance == NULL ) {
		instance = new Playlist();
	}
	return instance;
}



void Playlist::setNextSongByNumber(int SongNumber)
{
	
	int realNumber = SongNumber;
	
	if ( realNumber > (int)Hydrogen::get_instance()->m_PlayList.size() -1 || (int)Hydrogen::get_instance()->m_PlayList.size() == 0 )
		return;	

	setSelectedSongNr(  realNumber );
	setActiveSongNumber( realNumber );

	QString selected = "";
	selected = Hydrogen::get_instance()->m_PlayList[ realNumber ].m_hFile;

	loadSong( selected );
	execScript( realNumber );
}



void Playlist::setNextSongPlaylist()
{
	
	int index = getSelectedSongNr();
_INFOLOG( "index" + to_string( index ) );
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

	QString selected = "";
	selected = Hydrogen::get_instance()->m_PlayList[ index ].m_hFile;

	loadSong( selected );
	execScript( index );
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

	QString selected = "";
	selected = Hydrogen::get_instance()->m_PlayList[ index ].m_hFile;

	loadSong( selected );
	execScript( index );
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

	HydrogenApp *pH2App = HydrogenApp::getInstance();
	Hydrogen *engine = Hydrogen::get_instance();
	

	if ( engine->getState() == STATE_PLAYING ){
		engine->sequencer_stop();
	}

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
#ifdef WIN32
	//I know nothing about windows scripts -wolke-
	return;
#else
	QString file = "";
	file = Hydrogen::get_instance()->m_PlayList[ index ].m_hScript;

	QString script = "";
	script = Hydrogen::get_instance()->m_PlayList[ index ].m_hScriptEnabled;

	std::string filename = file.toStdString();
	std::string execscript = script.toStdString();

	if( filename == "no Script" || execscript == "Script not used")
		return;
	
	char *filen;
	filen = new char[ filename.length() + 1 ];
	strcpy( filen, filename.c_str() );
	std::system( filen ); 
	delete [] filen;
	return;
#endif
}
