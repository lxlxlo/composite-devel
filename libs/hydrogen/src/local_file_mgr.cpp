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

#include "config.h"


#include <hydrogen/adsr.h>
#include <hydrogen/data_path.h>
#include <hydrogen/hydrogen.h>
#include <hydrogen/h2_exception.h>
#include <hydrogen/instrument.h>
#include <hydrogen/LocalFileMng.h>
#include <hydrogen/note.h>
#include <hydrogen/Pattern.h>
#include <hydrogen/Preferences.h>
#include <hydrogen/Song.h>
#include <hydrogen/SoundLibrary.h>
#include <hydrogen/sample.h>
#include <hydrogen/fx/Effects.h>


#include <cstdlib>
#include <cassert>
#include <sys/stat.h>

#include <QDir>
#include <QApplication>

#include "xml/tinyxml.h"



namespace H2Core
{

LocalFileMng::LocalFileMng()
		: Object( "LocalFileMng" )
{
//	infoLog("INIT");
}



LocalFileMng::~LocalFileMng()
{
//	infoLog("DESTROY");
}


Pattern* LocalFileMng::loadPattern( const std::string& directory )
{
	
	InstrumentList* instrList = Hydrogen::get_instance()->getSong()->get_instrument_list(); 
	Pattern *pPattern = NULL; 
	std::string patternInfoFile = directory;
	std::ifstream verify( patternInfoFile.c_str() , std::ios::in | std::ios::binary ); 
	if ( verify == NULL ) {
		ERRORLOG( "Load Pattern: Data file " + patternInfoFile + " not found." ); return NULL;
	}

	TiXmlDocument doc( patternInfoFile.c_str() );
	doc.LoadFile();

	// root element
	TiXmlNode* rootNode;	// root element
	if ( !( rootNode = doc.FirstChild( "drumkit_pattern" ) ) ) {
		ERRORLOG( "Error reading Pattern: Pattern_drumkit_infonode not found" ); return NULL;
	}	

	TiXmlNode* patternNode = rootNode->FirstChild( "pattern" );

	std::string sName = LocalFileMng::readXmlString( patternNode,"pattern_name", "" );

	int nSize = -1;
	nSize = LocalFileMng::readXmlInt( patternNode, "size",nSize ,false,false ); 
	pPattern = new Pattern( sName, nSize );



	TiXmlNode* pNoteListNode = patternNode->FirstChild( "noteList" ); 
	if ( pNoteListNode ) 
	{
		// new code  :) 
		for ( TiXmlNode* noteNode = pNoteListNode->FirstChild( "note" ); noteNode; noteNode = noteNode->NextSibling( "note" ) ) 
		{
			Note* pNote = NULL;
			unsigned nPosition = LocalFileMng::readXmlInt( noteNode, "position", 0 ); 
			float fVelocity = LocalFileMng::readXmlFloat( noteNode, "velocity", 0.8f ); 
			float fPan_L = LocalFileMng::readXmlFloat( noteNode, "pan_L", 0.5 ); 
			float fPan_R = LocalFileMng::readXmlFloat( noteNode, "pan_R", 0.5 ); 
			int nLength = LocalFileMng::readXmlInt( noteNode, "length", -1, true ); 
			float nPitch = LocalFileMng::readXmlFloat( noteNode, "pitch", 0.0, false, false );
			std::string sKey = LocalFileMng::readXmlString( noteNode, "key", "C0", false, false );

			std::string instrId = LocalFileMng::readXmlString( noteNode, "instrument", "" );

			Instrument *instrRef = NULL;
			// search instrument by ref
			for ( unsigned i = 0; i < instrList->get_size(); i++ ) { Instrument *instr = instrList->get( i );
				if ( instrId == instr->get_id() ) {
					instrRef = instr;
					break;
				}
			}
			if ( !instrRef ) {
				ERRORLOG( "Instrument with ID: '" + instrId + "' not found. Note skipped." ); continue;
			}
			//assert( instrRef );

			pNote = new Note( instrRef, nPosition, fVelocity, fPan_L, fPan_R, nLength, nPitch, Note::stringToKey( sKey ) );
			pPattern->note_map.insert( std::make_pair( pNote->get_position(),pNote ) ); 
		} 
	}

	return pPattern;

}


int LocalFileMng::savePattern( Song *song , int selectedpattern , const std::string& patternname, const std::string& realpatternname, int mode)
{
	//int mode = 1 save, int mode = 2 save as 
	// INSTRUMENT NODE

	Instrument *instr = song->get_instrument_list()->get( 0 );
	assert( instr );


	Pattern *pat = song->get_pattern_list()->get( selectedpattern );
	
	std::string sDrumkitDir = Preferences::getInstance()->getDataDirectory() +  instr->get_drumkit_name() .c_str();
	
	//std::string sDrumkitPatternDir = sDrumkitDir + std::string( "/Pattern/" ) 
			
	INFOLOG( "[savePattern]" + to_string ( sDrumkitDir ));
	
	// check if the directory exists
	QDir dir( QString( sDrumkitDir.c_str() ) );
	QDir dirPattern( QString( sDrumkitDir.c_str() + QString( "/Pattern" )) );
	if ( !dir.exists() ) {
		dir.mkdir( QString( sDrumkitDir.c_str() ) );// create the drumkit directory
		dir.mkdir( QString( sDrumkitDir.c_str() + QString( "/Pattern" )) ); //create the pattern dir
	}else{
		if ( !dirPattern.exists() ) {
			dir.mkdir( QString( sDrumkitDir.c_str() + QString( "/Pattern" )) ); //create the pattern dir
		}
		
	} 

	std::string sPatternXmlFilename = "";
	// create the drumkit.xml file
	switch ( mode ){
		case 1: //save
			sPatternXmlFilename = sDrumkitDir + std::string( "/Pattern/" ) + std::string( patternname + std::string( ".h2pattern" ));
			WARNINGLOG( "Patternfile" +to_string(sPatternXmlFilename));
			//TiXmlDocument doc( sPatternXmlFilename.c_str() );
			break;
		case 2: //save as
			sPatternXmlFilename = patternname;
			WARNINGLOG( "Patternfile" +to_string(sPatternXmlFilename));
			//TiXmlDocument doc( sPatternXmlFilename.c_str() );
			break;
		default:
			WARNINGLOG( "Pattern Save unknown status");
			break;

	}
	TiXmlDocument doc( sPatternXmlFilename.c_str() );
	
	TiXmlElement rootNode( "drumkit_pattern" );
	//LIB_ID just in work to get better usability
	writeXmlString( &rootNode, "LIB_ID", "in_work" );
	writeXmlString( &rootNode, "pattern_for_drumkit", instr->get_drumkit_name() );	
		

	// pattern
	TiXmlElement patternNode( "pattern" );
	LocalFileMng::writeXmlString( &patternNode, "pattern_name", realpatternname );
	writeXmlString( &patternNode, "size", to_string( pat->get_lenght() ) );

		TiXmlElement noteListNode( "noteList" );
		std::multimap <int, Note*>::iterator pos;
		for ( pos = pat->note_map.begin(); pos != pat->note_map.end(); ++pos ) {
			Note *pNote = pos->second;
			assert( pNote );

			TiXmlElement noteNode( "note" );
			writeXmlString( &noteNode, "position", to_string( pNote->get_position() ) );
			writeXmlString( &noteNode, "velocity", to_string( pNote->get_velocity() ) );
			writeXmlString( &noteNode, "pan_L", to_string( pNote->get_pan_l() ) );
			writeXmlString( &noteNode, "pan_R", to_string( pNote->get_pan_r() ) );
			writeXmlString( &noteNode, "pitch", to_string( pNote->get_pitch() ) );

			writeXmlString( &noteNode, "key", Note::keyToString( pNote->m_noteKey ) );

			writeXmlString( &noteNode, "length", to_string( pNote->get_lenght() ) );
			writeXmlString( &noteNode, "instrument", pNote->get_instrument()->get_id() );
			noteListNode.InsertEndChild( noteNode );
		}
		patternNode.InsertEndChild( noteListNode );

	rootNode.InsertEndChild( patternNode );

	doc.InsertEndChild( rootNode );
	doc.SaveFile();

	return 0; // ok
}




void LocalFileMng::fileCopy( const std::string& sOrigFilename, const std::string& sDestFilename )
{
	INFOLOG( sOrigFilename + " --> " + sDestFilename );

	if ( sOrigFilename == sDestFilename ) {
		return;
	}

	FILE *inputFile = fopen( sOrigFilename.c_str(), "rb" );
	if ( inputFile == NULL ) {
		ERRORLOG( "Error opening " + sOrigFilename );
		return;
	}

	FILE *outputFile = fopen( sDestFilename.c_str(), "wb" );
	if ( outputFile == NULL ) {
		ERRORLOG( "Error opening " + sDestFilename );
		return;
	}

	const int bufferSize = 512;
	char buffer[ bufferSize ];
	while ( feof( inputFile ) == 0 ) {
		size_t read = fread( buffer, sizeof( char ), bufferSize, inputFile );
		fwrite( buffer, sizeof( char ), read, outputFile );
	}

	fclose( inputFile );
	fclose( outputFile );
}

std::vector<std::string> LocalFileMng::getSongList()
{
	std::vector<std::string> list;
	QString sDirectory = ( Preferences::getInstance()->getDataDirectory()  + "/songs").c_str();

	QDir dir( sDirectory );

	if ( !dir.exists() ) {
		ERRORLOG( std::string( "[getSongList] Directory " ).append( sDirectory.toStdString() ).append( " not found." ) );
	} else {
		QFileInfoList fileList = dir.entryInfoList();
		dir.setFilter( QDir::Dirs );
		for ( int i = 0; i < fileList.size(); ++i ) {
			std::string sFile = fileList.at( i ).fileName().toStdString();

			if ( ( sFile == "." ) || ( sFile == ".." ) || ( sFile == "CVS" )  || ( sFile == ".svn" ) ) {
				continue;
			}
			
			list.push_back( sFile.substr( 0, sFile.rfind( "." ) ) );
		}
	}

	return list;
}

std::vector<std::string> LocalFileMng::getPatternList()
{
	std::vector<std::string> list;
	QString sDirectory = ( Preferences::getInstance()->getDataDirectory()  + "/patterns").c_str();

	QDir dir( sDirectory );

	if ( !dir.exists() ) {
		ERRORLOG( std::string( "[getPatternList] Directory " ).append( sDirectory.toStdString() ).append( " not found." ) );
	} else {
		QFileInfoList fileList = dir.entryInfoList();
		dir.setFilter( QDir::Dirs );
		for ( int i = 0; i < fileList.size(); ++i ) {
			std::string sFile = fileList.at( i ).fileName().toStdString();

			if ( ( sFile == "." ) || ( sFile == ".." ) || ( sFile == "CVS" )  || ( sFile == ".svn" ) ) {
				continue;
			}
			
			list.push_back( sFile.substr( 0, sFile.rfind( "." ) ) );
		}
	}

	return list;
}

std::vector<std::string> LocalFileMng::getUserDrumkitList()
{
	std::vector<std::string> list;

	//QString sDirectory = QDir::homePath().append( "/.hydrogen/data" );
	//QString sDirectory = ( Preferences::getInstance()->getDataDirectory() + "/drumkits").c_str();

	QString sDirectory = Preferences::getInstance()->getDataDirectory().c_str();

	QDir dir( sDirectory );
	if ( !dir.exists() ) {
		ERRORLOG( std::string( "[listUserDrumkits] Directory " ).append( sDirectory.toStdString() ).append( " not found." ) );
	} else {
		QFileInfoList fileList = dir.entryInfoList();
		dir.setFilter( QDir::Dirs );
		for ( int i = 0; i < fileList.size(); ++i ) {
			std::string sFile = fileList.at( i ).fileName().toStdString();
			if ( ( sFile == "." ) || ( sFile == ".." ) || ( sFile == "CVS" )  || ( sFile == ".svn" ) ) {
				continue;
			}
			list.push_back( sFile );
		}
	}

	return list;
}




std::vector<std::string> LocalFileMng::getSystemDrumkitList()
{
	std::vector<std::string> list;

	QString sDirectory = QString( DataPath::get_data_path().append( "/drumkits" ).c_str() );
	QDir dir( sDirectory );
	if ( !dir.exists() ) {
		WARNINGLOG( std::string( "Directory " ).append( sDirectory.toStdString() ).append( " not found." ) );
	} else {
		QFileInfoList fileList = dir.entryInfoList();
		dir.setFilter( QDir::Dirs );
		for ( int i = 0; i < fileList.size(); ++i ) {
			std::string sFile = fileList.at( i ).fileName().toStdString();
			if ( ( sFile == "." ) || ( sFile == ".." ) || ( sFile == "CVS" ) || ( sFile == ".svn" ) ) {
				continue;
			}
			list.push_back( sFile );
		}
	}
	return list;
}



std::string LocalFileMng::getDrumkitDirectory( const std::string& drumkitName )
{
	// search in system drumkit
	std::vector<std::string> systemDrumkits = Drumkit::getSystemDrumkitList();
	for ( unsigned i = 0; i < systemDrumkits.size(); i++ ) {
		if ( systemDrumkits[ i ] == drumkitName ) {
			std::string path = std::string( DataPath::get_data_path() ) + "/drumkits/";
			return path;
		}
	}

	// search in user drumkit
	std::vector<std::string> userDrumkits = Drumkit::getUserDrumkitList();
	for ( unsigned i = 0; i < userDrumkits.size(); i++ ) {
		if ( userDrumkits[ i ] == drumkitName ) {
			std::string path = Preferences::getInstance()->getDataDirectory();
			//std::string path = Preferences::getInstance()->getDataDirectory() + "drumkits/";
			//string path = QDir::homePath().append("/.hydrogen/data/").toStdString();
			return path;
		}
	}

	ERRORLOG( "drumkit \"" + drumkitName + "\" not found" );
	return "";	// FIXME
}



/// Restituisce un oggetto DrumkitInfo.
/// Gli strumenti non hanno dei veri propri sample,
/// viene utilizzato solo il campo filename.
Drumkit* LocalFileMng::loadDrumkit( const std::string& directory )
{
	//INFOLOG( directory );

	// che if the drumkit.xml file exists
	std::string drumkitInfoFile = directory + "/drumkit.xml";
	std::ifstream verify( drumkitInfoFile.c_str() , std::ios::in | std::ios::binary );
	if ( verify == NULL ) {
		ERRORLOG( "Load Instrument: Data file " + drumkitInfoFile + " not found." );
		return NULL;
	}

	TiXmlDocument doc( drumkitInfoFile.c_str() );
	doc.LoadFile();

	// root element
	TiXmlNode* drumkitNode;	// root element
	if ( !( drumkitNode = doc.FirstChild( "drumkit_info" ) ) ) {
		ERRORLOG( "Error reading drumkit: drumkit_info node not found" );
		return NULL;
	}

	// Name
	std::string sDrumkitName = readXmlString( drumkitNode, "name", "" );
	if ( sDrumkitName == "" ) {
		ERRORLOG( "Error reading drumkit: name node not found" );
		return NULL;
	}

	std::string author = readXmlString( drumkitNode, "author", "undefined author", true );
	std::string info = readXmlString( drumkitNode, "info", "defaultInfo", true );

	Drumkit *drumkitInfo = new Drumkit();
	drumkitInfo->setName( sDrumkitName );
	drumkitInfo->setAuthor( author );
	drumkitInfo->setInfo( info );

	InstrumentList *instrumentList = new InstrumentList();

	TiXmlNode* instrumentListNode;
	if ( ( instrumentListNode = drumkitNode->FirstChild( "instrumentList" ) ) ) {
		// INSTRUMENT NODE
		int instrumentList_count = 0;
		TiXmlNode* instrumentNode = 0;
		for ( instrumentNode = instrumentListNode->FirstChild( "instrument" ); instrumentNode; instrumentNode = instrumentNode->NextSibling( "instrument" ) ) {
			instrumentList_count++;
			if ( instrumentList_count > MAX_INSTRUMENTS ) {
				ERRORLOG( "Instrument count >= MAX_INSTRUMENTS. Drumkit: " + drumkitInfo->getName() );
				break;
			}

			std::string id = readXmlString( instrumentNode, "id", "" );
			std::string name = readXmlString( instrumentNode, "name", "" );
			float volume = readXmlFloat( instrumentNode, "volume", 1.0f );
			bool isMuted = readXmlBool( instrumentNode, "isMuted", false );
			float pan_L = readXmlFloat( instrumentNode, "pan_L", 1.0f );
			float pan_R = readXmlFloat( instrumentNode, "pan_R", 1.0f );
			bool bFilterActive = readXmlBool( instrumentNode, "filterActive", false, false );
			float fFilterCutoff = readXmlFloat( instrumentNode, "filterCutoff", 1.0f, false, false );
			float fFilterResonance = readXmlFloat( instrumentNode, "filterResonance", 0.0f, false, false );
			float fRandomPitchFactor = readXmlFloat( instrumentNode, "randomPitchFactor", 0.0f, false, false );
			float fAttack = LocalFileMng::readXmlFloat( instrumentNode, "Attack", 0, false, false );		// Attack
			float fDecay = LocalFileMng::readXmlFloat( instrumentNode, "Decay", 0, false, false  );		// Decay
			float fSustain = LocalFileMng::readXmlFloat( instrumentNode, "Sustain", 1.0, false, false );	// Sustain
			float fRelease = LocalFileMng::readXmlFloat( instrumentNode, "Release", 1000, false, false );	// Release
			float fGain = readXmlFloat( instrumentNode, "gain", 1.0f, false, false );
			std::string sMuteGroup = readXmlString( instrumentNode, "muteGroup", "-1", false, false );
			int nMuteGroup = atoi( sMuteGroup.c_str() );

			// some sanity checks
			if ( id == "" ) {
				ERRORLOG( "Empty ID for instrument. The drumkit '" + sDrumkitName + "' is corrupted. Skipping instrument '" + name + "'" );
				continue;
			}

			Instrument *pInstrument = new Instrument( id, name, new ADSR() );
			pInstrument->set_volume( volume );


			// back compatibility code
			TiXmlNode* filenameNode = instrumentNode->FirstChild( "filename" );
			if ( filenameNode ) {
				//warningLog( "Using back compatibility code. filename node found" );
				std::string sFilename = LocalFileMng::readXmlString( instrumentNode, "filename", "" );
				Sample *pSample = new Sample( 0, sFilename );
				InstrumentLayer *pLayer = new InstrumentLayer( pSample );
				pInstrument->set_layer( pLayer, 0 );
			}
			//~ back compatibility code
			else {
				unsigned nLayer = 0;
				for ( TiXmlNode* layerNode = instrumentNode->FirstChild( "layer" ); layerNode; layerNode = layerNode->NextSibling( "layer" ) ) {
					if ( nLayer >= MAX_LAYERS ) {
						ERRORLOG( "nLayer > MAX_LAYERS" );
						continue;
					}
					std::string sFilename = LocalFileMng::readXmlString( layerNode, "filename", "" );
					float fMin = LocalFileMng::readXmlFloat( layerNode, "min", 0.0 );
					float fMax = LocalFileMng::readXmlFloat( layerNode, "max", 1.0 );
					float fGain = LocalFileMng::readXmlFloat( layerNode, "gain", 1.0, false, false );
					float fPitch = LocalFileMng::readXmlFloat( layerNode, "pitch", 0.0, false, false );

					Sample *pSample = new Sample( 0, sFilename );
					InstrumentLayer *pLayer = new InstrumentLayer( pSample );
					pLayer->set_start_velocity( fMin );
					pLayer->set_end_velocity( fMax );
					pLayer->set_gain( fGain );
					pLayer->set_pitch( fPitch );
					pInstrument->set_layer( pLayer, nLayer );

					nLayer++;
				}
			}

			pInstrument->set_filter_active( bFilterActive );
			pInstrument->set_filter_cutoff( fFilterCutoff );
			pInstrument->set_filter_resonance( fFilterResonance );
			pInstrument->set_muted( isMuted );
			pInstrument->set_pan_l( pan_L );
			pInstrument->set_pan_r( pan_R );
			pInstrument->set_random_pitch_factor( fRandomPitchFactor );
			pInstrument->set_drumkit_name( drumkitInfo->getName() );
			pInstrument->set_gain( fGain );
			pInstrument->set_mute_group( nMuteGroup );

			pInstrument->set_adsr( new ADSR( fAttack, fDecay, fSustain, fRelease ) );
			instrumentList->add( pInstrument );
		}
	} else {
		WARNINGLOG( "Error reading drumkit: instrumentList node not found" );
	}
	drumkitInfo->setInstrumentList( instrumentList );

	return drumkitInfo;
}



int LocalFileMng::saveDrumkit( Drumkit *info )
{
	INFOLOG( "[saveDrumkit]" );
	info->dump();	// debug

	std::string sDrumkitDir = Preferences::getInstance()->getDataDirectory() + info->getName().c_str();

	// check if the directory exists
	QDir dir( QString( sDrumkitDir.c_str() ) );
	if ( !dir.exists() ) {
		dir.mkdir( QString( sDrumkitDir.c_str() ) );// create the drumkit directory
		//mkdir( sDrumkitDir.c_str(), S_IRWXU );
	} else {
		/*
		warningLog( "[saveDrumkit] Cleaning directory " + sDrumkitDir );
		// clear all the old files in the directory
		string clearCmd = "rm -f " + sDrumkitDir + "/*";
		system( clearCmd.c_str() );
		*/
	}


	// create the drumkit.xml file
	std::string sDrumkitXmlFilename = sDrumkitDir + std::string( "/drumkit.xml" );

	TiXmlDocument doc( sDrumkitXmlFilename.c_str() );

	TiXmlElement rootNode( "drumkit_info" );

	writeXmlString( &rootNode, "name", info->getName() );	// name
	writeXmlString( &rootNode, "author", info->getAuthor() );	// author
	writeXmlString( &rootNode, "info", info->getInfo() );	// info

	TiXmlElement instrumentListNode( "instrumentList" );		// instrument list
	unsigned nInstrument = info->getInstrumentList()->get_size();
	// INSTRUMENT NODE
	for ( unsigned i = 0; i < nInstrument; i++ ) {
		Instrument *instr = info->getInstrumentList()->get( i );

		for ( unsigned nLayer = 0; nLayer < MAX_LAYERS; nLayer++ ) {
			InstrumentLayer *pLayer = instr->get_layer( nLayer );
			if ( pLayer ) {
				Sample *pSample = pLayer->get_sample();
				std::string sOrigFilename = pSample->get_filename();

				std::string sDestFilename = sOrigFilename;

				int nPos = sDestFilename.rfind( '/' );
				sDestFilename = sDestFilename.substr( nPos + 1, sDestFilename.size() - nPos - 1 );
				sDestFilename = sDrumkitDir + "/" + sDestFilename;

				fileCopy( sOrigFilename, sDestFilename );
			}
		}

		TiXmlElement instrumentNode( "instrument" );

		LocalFileMng::writeXmlString( &instrumentNode, "id", instr->get_id() );
		LocalFileMng::writeXmlString( &instrumentNode, "name", instr->get_name() );
		LocalFileMng::writeXmlString( &instrumentNode, "volume", to_string( instr->get_volume() ) );
		LocalFileMng::writeXmlBool( &instrumentNode, "isMuted", instr->is_muted() );
		LocalFileMng::writeXmlString( &instrumentNode, "pan_L", to_string( instr->get_pan_l() ) );
		LocalFileMng::writeXmlString( &instrumentNode, "pan_R", to_string( instr->get_pan_r() ) );
		LocalFileMng::writeXmlString( &instrumentNode, "randomPitchFactor", to_string( instr->get_random_pitch_factor() ) );
		LocalFileMng::writeXmlString( &instrumentNode, "gain", to_string( instr->get_gain() ) );

		LocalFileMng::writeXmlBool( &instrumentNode, "filterActive", instr->is_filter_active() );
		LocalFileMng::writeXmlString( &instrumentNode, "filterCutoff", to_string( instr->get_filter_cutoff() ) );
		LocalFileMng::writeXmlString( &instrumentNode, "filterResonance", to_string( instr->get_filter_resonance() ) );

		LocalFileMng::writeXmlString( &instrumentNode, "Attack", to_string( instr->get_adsr()->__attack ) );
		LocalFileMng::writeXmlString( &instrumentNode, "Decay", to_string( instr->get_adsr()->__decay ) );
		LocalFileMng::writeXmlString( &instrumentNode, "Sustain", to_string( instr->get_adsr()->__sustain ) );
		LocalFileMng::writeXmlString( &instrumentNode, "Release", to_string( instr->get_adsr()->__release ) );

		LocalFileMng::writeXmlString( &instrumentNode, "muteGroup", to_string( instr->get_mute_group() ) );

		for ( unsigned nLayer = 0; nLayer < MAX_LAYERS; nLayer++ ) {
			InstrumentLayer *pLayer = instr->get_layer( nLayer );
			if ( pLayer == NULL ) continue;
			Sample *pSample = pLayer->get_sample();

			std::string sFilename = pSample->get_filename();

			//if (instr->getDrumkitName() != "") {
			// se e' specificato un drumkit, considero solo il nome del file senza il path
			int nPos = sFilename.rfind( "/" );
			sFilename = sFilename.substr( nPos + 1, sFilename.length() );
			//}

			TiXmlElement layerNode( "layer" );
			LocalFileMng::writeXmlString( &layerNode, "filename", sFilename );
			LocalFileMng::writeXmlString( &layerNode, "min", to_string( pLayer->get_start_velocity() ) );
			LocalFileMng::writeXmlString( &layerNode, "max", to_string( pLayer->get_end_velocity() ) );
			LocalFileMng::writeXmlString( &layerNode, "gain", to_string( pLayer->get_gain() ) );
			LocalFileMng::writeXmlString( &layerNode, "pitch", to_string( pLayer->get_pitch() ) );

			instrumentNode.InsertEndChild( layerNode );
		}

		instrumentListNode.InsertEndChild( instrumentNode );
	}

	rootNode.InsertEndChild( instrumentListNode );

	doc.InsertEndChild( rootNode );
	doc.SaveFile();

	return 0; // ok
}




std::string LocalFileMng::readXmlString( TiXmlNode* parent, const std::string& nodeName, const std::string& defaultValue, bool bCanBeEmpty, bool bShouldExists )
{
	TiXmlNode* node;
	if ( parent && ( node = parent->FirstChild( nodeName.c_str() ) ) ) {
		if ( node->FirstChild() ) {
			return node->FirstChild()->Value();
		} else {
			if ( !bCanBeEmpty ) {
				_WARNINGLOG( "Using default value in " + nodeName );
			}
			return defaultValue;
		}
	} else {
		if ( bShouldExists ) {
			_WARNINGLOG( "'" + nodeName + "' node not found" );
		}
		return defaultValue;
	}
}



float LocalFileMng::readXmlFloat( TiXmlNode* parent, const std::string& nodeName, float defaultValue, bool bCanBeEmpty, bool bShouldExists )
{
	TiXmlNode* node;
	if ( parent && ( node = parent->FirstChild( nodeName.c_str() ) ) ) {
		if ( node->FirstChild() ) {
			float res = string_to_float( node->FirstChild()->Value() );
			return res;
		} else {
			if ( !bCanBeEmpty ) {
				_WARNINGLOG( "Using default value in " + nodeName );
			}
			return defaultValue;
		}
	} else {
		if ( bShouldExists ) {
			_WARNINGLOG( "'" + nodeName + "' node not found" );
		}
		return defaultValue;
	}
}



int LocalFileMng::readXmlInt( TiXmlNode* parent, const std::string& nodeName, int defaultValue, bool bCanBeEmpty, bool bShouldExists )
{
	TiXmlNode* node;
	if ( parent && ( node = parent->FirstChild( nodeName.c_str() ) ) ) {
		if ( node->FirstChild() ) {
			return atoi( node->FirstChild()->Value() );
		} else {
			if ( !bCanBeEmpty ) {
				_WARNINGLOG( "Using default value in " + nodeName );
			}
			return defaultValue;
		}
	} else {
		if ( bShouldExists )  {
			_WARNINGLOG( "'" + nodeName + "' node not found" );
		}
		return defaultValue;
	}
}



bool LocalFileMng::readXmlBool( TiXmlNode* parent, const std::string& nodeName, bool defaultValue, bool bShouldExists )
{
	TiXmlNode* node;
	if ( parent && ( node = parent->FirstChild( nodeName.c_str() ) ) ) {
		if ( node->FirstChild() ) {
			if ( std::string( node->FirstChild()->Value() ) == "true" ) {
				return true;
			} else {
				return false;
			}
		} else {
			_WARNINGLOG( "Using default value in " + nodeName );
			return defaultValue;
		}
	} else {
		if ( bShouldExists ) {
			_WARNINGLOG( "'" + nodeName + "' node not found" );
		}
		return defaultValue;
	}
}



void LocalFileMng::writeXmlString( TiXmlNode *parent, const std::string& name, const std::string& text )
{
	TiXmlElement versionNode( name );
	TiXmlText versionText( text );
	versionNode.InsertEndChild( versionText );
	parent->InsertEndChild( versionNode );
}



void LocalFileMng::writeXmlBool( TiXmlNode *parent, const std::string& name, bool value )
{
	if ( value ) {
		writeXmlString( parent, name, std::string( "true" ) );
	} else {
		writeXmlString( parent, name, std::string( "false" ) );
	}
}












//-----------------------------------------------------------------------------
//	Implementation of SongWriter class
//-----------------------------------------------------------------------------


SongWriter::SongWriter()
		: Object( "SongWriter" )
{
//	infoLog("init");
}



SongWriter::~SongWriter()
{
//	infoLog("destroy");
}



void SongWriter::writeSong( Song *song, const std::string& filename )
{
	INFOLOG( "Saving song " + filename );

	// FIXME: verificare se e' possibile scrivere il file
	// FIXME: verificare che il file non sia gia' esistente
	// FIXME: effettuare copia di backup per il file gia' esistente

	TiXmlDocument doc( filename );

	TiXmlElement songNode( "song" );

	LocalFileMng::writeXmlString( &songNode, "version", std::string( VERSION ) );
	LocalFileMng::writeXmlString( &songNode, "bpm", to_string( song->__bpm ) );
	LocalFileMng::writeXmlString( &songNode, "volume", to_string( song->get_volume() ) );
	LocalFileMng::writeXmlString( &songNode, "metronomeVolume", to_string( song->get_metronome_volume() ) );
	LocalFileMng::writeXmlString( &songNode, "name", song->__name );
	LocalFileMng::writeXmlString( &songNode, "author", song->__author );
	LocalFileMng::writeXmlString( &songNode, "notes", song->get_notes() );
	LocalFileMng::writeXmlBool( &songNode, "loopEnabled", song->is_loop_enabled() );

	if ( song->get_mode() == Song::SONG_MODE ) {
		LocalFileMng::writeXmlString( &songNode, "mode", std::string( "song" ) );
	} else {
		LocalFileMng::writeXmlString( &songNode, "mode", std::string( "pattern" ) );
	}

	LocalFileMng::writeXmlString( &songNode, "humanize_time", to_string( song->get_humanize_time_value() ) );
	LocalFileMng::writeXmlString( &songNode, "humanize_velocity", to_string( song->get_humanize_velocity_value() ) );
	LocalFileMng::writeXmlString( &songNode, "swing_factor", to_string( song->get_swing_factor() ) );

	/*	LocalFileMng::writeXmlBool( &songNode, "delayFXEnabled", song->m_bDelayFXEnabled );
		LocalFileMng::writeXmlString( &songNode, "delayFXWetLevel", to_string( song->m_fDelayFXWetLevel ) );
		LocalFileMng::writeXmlString( &songNode, "delayFXFeedback", to_string( song->m_fDelayFXFeedback ) );
		LocalFileMng::writeXmlString( &songNode, "delayFXTime", to_string( song->m_nDelayFXTime ) );
	*/

	// instrument list
	TiXmlElement instrumentListNode( "instrumentList" );
	unsigned nInstrument = song->get_instrument_list()->get_size();

	// INSTRUMENT NODE
	for ( unsigned i = 0; i < nInstrument; i++ ) {
		Instrument *instr = song->get_instrument_list()->get( i );
		assert( instr );

		TiXmlElement instrumentNode( "instrument" );

		LocalFileMng::writeXmlString( &instrumentNode, "id", instr->get_id() );
		LocalFileMng::writeXmlString( &instrumentNode, "drumkit", instr->get_drumkit_name() );
		LocalFileMng::writeXmlString( &instrumentNode, "name", instr->get_name() );
		LocalFileMng::writeXmlString( &instrumentNode, "volume", to_string( instr->get_volume() ) );
		LocalFileMng::writeXmlBool( &instrumentNode, "isMuted", instr->is_muted() );
		LocalFileMng::writeXmlString( &instrumentNode, "pan_L", to_string( instr->get_pan_l() ) );
		LocalFileMng::writeXmlString( &instrumentNode, "pan_R", to_string( instr->get_pan_r() ) );
		LocalFileMng::writeXmlString( &instrumentNode, "gain", to_string( instr->get_gain() ) );

		LocalFileMng::writeXmlBool( &instrumentNode, "filterActive", instr->is_filter_active() );
		LocalFileMng::writeXmlString( &instrumentNode, "filterCutoff", to_string( instr->get_filter_cutoff() ) );
		LocalFileMng::writeXmlString( &instrumentNode, "filterResonance", to_string( instr->get_filter_resonance() ) );

		LocalFileMng::writeXmlString( &instrumentNode, "FX1Level", to_string( instr->get_fx_level( 0 ) ) );
		LocalFileMng::writeXmlString( &instrumentNode, "FX2Level", to_string( instr->get_fx_level( 1 ) ) );
		LocalFileMng::writeXmlString( &instrumentNode, "FX3Level", to_string( instr->get_fx_level( 2 ) ) );
		LocalFileMng::writeXmlString( &instrumentNode, "FX4Level", to_string( instr->get_fx_level( 3 ) ) );

		assert( instr->get_adsr() );
		LocalFileMng::writeXmlString( &instrumentNode, "Attack", to_string( instr->get_adsr()->__attack ) );
		LocalFileMng::writeXmlString( &instrumentNode, "Decay", to_string( instr->get_adsr()->__decay ) );
		LocalFileMng::writeXmlString( &instrumentNode, "Sustain", to_string( instr->get_adsr()->__sustain ) );
		LocalFileMng::writeXmlString( &instrumentNode, "Release", to_string( instr->get_adsr()->__release ) );

		LocalFileMng::writeXmlString( &instrumentNode, "randomPitchFactor", to_string( instr->get_random_pitch_factor() ) );

		LocalFileMng::writeXmlString( &instrumentNode, "muteGroup", to_string( instr->get_mute_group() ) );

		for ( unsigned nLayer = 0; nLayer < MAX_LAYERS; nLayer++ ) {
			InstrumentLayer *pLayer = instr->get_layer( nLayer );
			if ( pLayer == NULL ) continue;
			Sample *pSample = pLayer->get_sample();
			if ( pSample == NULL ) continue;

			std::string sFilename = pSample->get_filename();

			if ( instr->get_drumkit_name() != "" ) {
				// se e' specificato un drumkit, considero solo il nome del file senza il path
				int nPos = sFilename.rfind( "/" );
				sFilename = sFilename.substr( nPos + 1, sFilename.length() );
			}

			TiXmlElement layerNode( "layer" );
			LocalFileMng::writeXmlString( &layerNode, "filename", sFilename );
			LocalFileMng::writeXmlString( &layerNode, "min", to_string( pLayer->get_start_velocity() ) );
			LocalFileMng::writeXmlString( &layerNode, "max", to_string( pLayer->get_end_velocity() ) );
			LocalFileMng::writeXmlString( &layerNode, "gain", to_string( pLayer->get_gain() ) );
			LocalFileMng::writeXmlString( &layerNode, "pitch", to_string( pLayer->get_pitch() ) );

			instrumentNode.InsertEndChild( layerNode );
		}

		instrumentListNode.InsertEndChild( instrumentNode );
	}
	songNode.InsertEndChild( instrumentListNode );


	// pattern list
	TiXmlElement patternListNode( "patternList" );

	unsigned nPatterns = song->get_pattern_list()->get_size();
	for ( unsigned i = 0; i < nPatterns; i++ ) {
		Pattern *pat = song->get_pattern_list()->get( i );

		// pattern
		TiXmlElement patternNode( "pattern" );
		LocalFileMng::writeXmlString( &patternNode, "name", pat->get_name() );
		LocalFileMng::writeXmlString( &patternNode, "size", to_string( pat->get_lenght() ) );

		TiXmlElement noteListNode( "noteList" );
		std::multimap <int, Note*>::iterator pos;
		for ( pos = pat->note_map.begin(); pos != pat->note_map.end(); ++pos ) {
			Note *pNote = pos->second;
			assert( pNote );

			TiXmlElement noteNode( "note" );
			LocalFileMng::writeXmlString( &noteNode, "position", to_string( pNote->get_position() ) );
			LocalFileMng::writeXmlString( &noteNode, "velocity", to_string( pNote->get_velocity() ) );
			LocalFileMng::writeXmlString( &noteNode, "pan_L", to_string( pNote->get_pan_l() ) );
			LocalFileMng::writeXmlString( &noteNode, "pan_R", to_string( pNote->get_pan_r() ) );
			LocalFileMng::writeXmlString( &noteNode, "pitch", to_string( pNote->get_pitch() ) );

			LocalFileMng::writeXmlString( &noteNode, "key", Note::keyToString( pNote->m_noteKey ) );

			LocalFileMng::writeXmlString( &noteNode, "length", to_string( pNote->get_lenght() ) );
			LocalFileMng::writeXmlString( &noteNode, "instrument", pNote->get_instrument()->get_id() );
			noteListNode.InsertEndChild( noteNode );
		}
		patternNode.InsertEndChild( noteListNode );

		patternListNode.InsertEndChild( patternNode );
	}
	songNode.InsertEndChild( patternListNode );


	// pattern sequence
	TiXmlElement patternSequenceNode( "patternSequence" );

	unsigned nPatternGroups = song->get_pattern_group_vector()->size();
	for ( unsigned i = 0; i < nPatternGroups; i++ ) {
		TiXmlElement groupNode( "group" );

		PatternList *pList = ( *song->get_pattern_group_vector() )[i];
		for ( unsigned j = 0; j < pList->get_size(); j++ ) {
			Pattern *pPattern = pList->get( j );
			LocalFileMng::writeXmlString( &groupNode, "patternID", pPattern->get_name() );
		}
		patternSequenceNode.InsertEndChild( groupNode );
	}

	songNode.InsertEndChild( patternSequenceNode );


	// LADSPA FX
	TiXmlElement ladspaFxNode( "ladspa" );

	for ( unsigned nFX = 0; nFX < MAX_FX; nFX++ ) {
		TiXmlElement fxNode( "fx" );

#ifdef LADSPA_SUPPORT
		LadspaFX *pFX = Effects::getInstance()->getLadspaFX( nFX );
		if ( pFX ) {
			LocalFileMng::writeXmlString( &fxNode, "name", pFX->getPluginLabel() );
			LocalFileMng::writeXmlString( &fxNode, "filename", pFX->getLibraryPath() );
			LocalFileMng::writeXmlBool( &fxNode, "enabled", pFX->isEnabled() );
			LocalFileMng::writeXmlString( &fxNode, "volume", to_string( pFX->getVolume() ) );
			for ( unsigned nControl = 0; nControl < pFX->inputControlPorts.size(); nControl++ ) {
				LadspaControlPort *pControlPort = pFX->inputControlPorts[ nControl ];
				TiXmlElement controlPortNode( "inputControlPort" );
				LocalFileMng::writeXmlString( &controlPortNode, "name", pControlPort->sName );
				LocalFileMng::writeXmlString( &controlPortNode, "value", to_string( pControlPort->fControlValue ) );
				fxNode.InsertEndChild( controlPortNode );
			}
			for ( unsigned nControl = 0; nControl < pFX->outputControlPorts.size(); nControl++ ) {
				LadspaControlPort *pControlPort = pFX->inputControlPorts[ nControl ];
				TiXmlElement controlPortNode( "outputControlPort" );
				LocalFileMng::writeXmlString( &controlPortNode, "name", pControlPort->sName );
				LocalFileMng::writeXmlString( &controlPortNode, "value", to_string( pControlPort->fControlValue ) );
				fxNode.InsertEndChild( controlPortNode );
			}
		}
#else
		if ( false ) {
		}
#endif
		else {
			LocalFileMng::writeXmlString( &fxNode, "name", std::string( "no plugin" ) );
			LocalFileMng::writeXmlString( &fxNode, "filename", std::string( "-" ) );
			LocalFileMng::writeXmlBool( &fxNode, "enabled", false );
			LocalFileMng::writeXmlString( &fxNode, "volume", to_string( 0.0 ) );
		}
		ladspaFxNode.InsertEndChild( fxNode );
	}

	songNode.InsertEndChild( ladspaFxNode );




	doc.InsertEndChild( songNode );
	doc.SaveFile();

	song->__is_modified = false;
	song->set_filename( filename );
}


};

