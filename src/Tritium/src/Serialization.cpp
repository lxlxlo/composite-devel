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

#include <Tritium/Serialization.hpp>
#include "SerializationPrivate.hpp"
#include <Tritium/Logger.hpp>
#include <Tritium/LocalFileMng.hpp>
#include <Tritium/Song.hpp>
#include <Tritium/Instrument.hpp>
#include <Tritium/InstrumentList.hpp>
#include <Tritium/InstrumentLayer.hpp>
#include <Tritium/ADSR.hpp>
#include <Tritium/Pattern.hpp>
#include <Tritium/Engine.hpp>
#include <Tritium/Sample.hpp>
#include <Tritium/Note.hpp>
#include <Tritium/fx/Effects.hpp>
#include "version.h"

#include <QFile>
#include <QtXml>
#include <cassert>

using namespace Tritium;
using namespace Tritium::Serialization;

/*********************************************************************
 * Serialization implementation
 *********************************************************************
 */

void load_file(const QString& filename, LoadBundle& report_to)
{
    assert(false); // TODO
}

void save_song(const QString& filename,
	       Song& song,
	       SaveReport& report_t,
	       bool overwrite)
{
    assert(false);
}

void save_drumkit(const QString& filename,
		  Drumkit& song,
		  SaveReport& report_to,
		  bool overwrite = false)
{
    assert(false);
}

void save_pattern(const QString& filename,
		  Pattern& pattern,
		  SaveReport& report_to,
		  bool overwrite = false)
{
    assert(false);
}


/*********************************************************************
 * SongReader implementation
 *********************************************************************
 */

SongReader::SongReader()
{
}

SongReader::~SongReader()
{
}

///
/// Reads a song.
/// return NULL = error reading song file.
///
Song* SongReader::readSong( const QString& filename )
{
    INFOLOG( filename );
    Song* song = NULL;

    if (QFile( filename ).exists() == false ) {
	ERRORLOG( "Song file " + filename + " not found." );
	return NULL;
    }

    QDomDocument doc = LocalFileMng::openXmlDocument( filename );
    QDomNodeList nodeList = doc.elementsByTagName( "song" );
	

    if( nodeList.isEmpty() ){
	ERRORLOG( "Error reading song: song node not found" );
	return NULL;
    }

    QDomNode songNode = nodeList.at(0);

    m_sSongVersion = LocalFileMng::readXmlString( songNode , "version", "Unknown version" );

	
    if ( m_sSongVersion != QString( get_version().c_str() ) ) {
	WARNINGLOG( "Trying to load a song created with a different version of Hydrogen/Tritium/Composite." );
	WARNINGLOG( "Song [" + filename + "] saved with version " + m_sSongVersion );
    }

	
	
		
    float fBpm = LocalFileMng::readXmlFloat( songNode, "bpm", 120 );
    float fVolume = LocalFileMng::readXmlFloat( songNode, "volume", 0.5 );
    float fMetronomeVolume = LocalFileMng::readXmlFloat( songNode, "metronomeVolume", 0.5 );
    QString sName( LocalFileMng::readXmlString( songNode, "name", "Untitled Song" ) );
    QString sAuthor( LocalFileMng::readXmlString( songNode, "author", "Unknown Author" ) );
    QString sNotes( LocalFileMng::readXmlString( songNode, "notes", "..." ) );
    QString sLicense( LocalFileMng::readXmlString( songNode, "license", "Unknown license" ) );
    bool bLoopEnabled = LocalFileMng::readXmlBool( songNode, "loopEnabled", false );

    Song::SongMode nMode = Song::PATTERN_MODE;	// Mode (song/pattern)
    QString sMode = LocalFileMng::readXmlString( songNode, "mode", "pattern" );
    if ( sMode == "song" ) {
	nMode = Song::SONG_MODE;
    }

    float fHumanizeTimeValue = LocalFileMng::readXmlFloat( songNode, "humanize_time", 0.0 );
    float fHumanizeVelocityValue = LocalFileMng::readXmlFloat( songNode, "humanize_velocity", 0.0 );
    float fSwingFactor = LocalFileMng::readXmlFloat( songNode, "swing_factor", 0.0 );

    song = new Song( sName, sAuthor, fBpm, fVolume );
    song->set_metronome_volume( fMetronomeVolume );
    song->set_notes( sNotes );
    song->set_license( sLicense );
    song->set_loop_enabled( bLoopEnabled );
    song->set_mode( nMode );
    song->set_humanize_time_value( fHumanizeTimeValue );
    song->set_humanize_velocity_value( fHumanizeVelocityValue );
    song->set_swing_factor( fSwingFactor );
	
	

    /*
      song->m_bDelayFXEnabled = LocalFileMng::readXmlBool( songNode, "delayFXEnabled", false, false );
      song->m_fDelayFXWetLevel = LocalFileMng::readXmlFloat( songNode, "delayFXWetLevel", 1.0, false, false );
      song->m_fDelayFXFeedback= LocalFileMng::readXmlFloat( songNode, "delayFXFeedback", 0.4, false, false );
      song->m_nDelayFXTime = LocalFileMng::readXmlInt( songNode, "delayFXTime", MAX_NOTES / 4, false, false );
    */


    //  Instrument List
	
    LocalFileMng localFileMng;
    InstrumentList *instrumentList = new InstrumentList();

    QDomNode instrumentListNode = songNode.firstChildElement( "instrumentList" );
    if ( ( ! instrumentListNode.isNull()  ) ) {
	// INSTRUMENT NODE
	int instrumentList_count = 0;
	QDomNode instrumentNode;
	instrumentNode = instrumentListNode.firstChildElement( "instrument" );
	while ( ! instrumentNode.isNull()  ) {
	    instrumentList_count++;

	    QString sId = LocalFileMng::readXmlString( instrumentNode, "id", "" );			// instrument id
	    QString sDrumkit = LocalFileMng::readXmlString( instrumentNode, "drumkit", "" );	// drumkit
	    Engine::get_instance()->setCurrentDrumkitname( sDrumkit ); 
	    QString sName = LocalFileMng::readXmlString( instrumentNode, "name", "" );		// name
	    float fVolume = LocalFileMng::readXmlFloat( instrumentNode, "volume", 1.0 );	// volume
	    bool bIsMuted = LocalFileMng::readXmlBool( instrumentNode, "isMuted", false );	// is muted
	    float fPan_L = LocalFileMng::readXmlFloat( instrumentNode, "pan_L", 0.5 );	// pan L
	    float fPan_R = LocalFileMng::readXmlFloat( instrumentNode, "pan_R", 0.5 );	// pan R
	    float fFX1Level = LocalFileMng::readXmlFloat( instrumentNode, "FX1Level", 0.0 );	// FX level
	    float fFX2Level = LocalFileMng::readXmlFloat( instrumentNode, "FX2Level", 0.0 );	// FX level
	    float fFX3Level = LocalFileMng::readXmlFloat( instrumentNode, "FX3Level", 0.0 );	// FX level
	    float fFX4Level = LocalFileMng::readXmlFloat( instrumentNode, "FX4Level", 0.0 );	// FX level
	    float fGain = LocalFileMng::readXmlFloat( instrumentNode, "gain", 1.0, false, false );	// instrument gain

	    int fAttack = LocalFileMng::readXmlInt( instrumentNode, "Attack", 0, false, false );		// Attack
	    int fDecay = LocalFileMng::readXmlInt( instrumentNode, "Decay", 0, false, false );		// Decay
	    float fSustain = LocalFileMng::readXmlFloat( instrumentNode, "Sustain", 1.0, false, false );	// Sustain
	    int fRelease = LocalFileMng::readXmlInt( instrumentNode, "Release", 1000, false, false );	// Release

	    float fRandomPitchFactor = LocalFileMng::readXmlFloat( instrumentNode, "randomPitchFactor", 0.0f, false, false );

	    bool bFilterActive = LocalFileMng::readXmlBool( instrumentNode, "filterActive", false );
	    float fFilterCutoff = LocalFileMng::readXmlFloat( instrumentNode, "filterCutoff", 1.0f, false );
	    float fFilterResonance = LocalFileMng::readXmlFloat( instrumentNode, "filterResonance", 0.0f, false );
	    QString sMuteGroup = LocalFileMng::readXmlString( instrumentNode, "muteGroup", "-1", false );
	    int nMuteGroup = sMuteGroup.toInt();


	    if ( sId.isEmpty() ) {
		ERRORLOG( "Empty ID for instrument '" + sName + "'. skipping." );
		continue;
	    }


	    // create a new instrument
	    Instrument *pInstrument = new Instrument( sId, sName, new ADSR( fAttack, fDecay, fSustain, fRelease ) );
	    pInstrument->set_volume( fVolume );
	    pInstrument->set_muted( bIsMuted );
	    pInstrument->set_pan_l( fPan_L );
	    pInstrument->set_pan_r( fPan_R );
	    pInstrument->set_drumkit_name( sDrumkit );
	    pInstrument->set_fx_level( fFX1Level, 0 );
	    pInstrument->set_fx_level( fFX2Level, 1 );
	    pInstrument->set_fx_level( fFX3Level, 2 );
	    pInstrument->set_fx_level( fFX4Level, 3 );
	    pInstrument->set_random_pitch_factor( fRandomPitchFactor );
	    pInstrument->set_filter_active( bFilterActive );
	    pInstrument->set_filter_cutoff( fFilterCutoff );
	    pInstrument->set_filter_resonance( fFilterResonance );
	    pInstrument->set_gain( fGain );
	    pInstrument->set_mute_group( nMuteGroup );

	    QString drumkitPath;
	    if ( ( !sDrumkit.isEmpty() ) && ( sDrumkit != "-" ) ) {
//				drumkitPath = localFileMng.getDrumkitDirectory( sDrumkit ) + sDrumkit + "/";
		drumkitPath = localFileMng.getDrumkitDirectory( sDrumkit ) + sDrumkit;
	    }
			
			
	    QDomNode filenameNode = instrumentNode.firstChildElement( "filename" );
			
			
	    // back compatibility code ( song version <= 0.9.0 )
	    if ( ! filenameNode.isNull() ) {
		WARNINGLOG( "Using back compatibility code. filename node found" );
		QString sFilename = LocalFileMng::readXmlString( instrumentNode, "filename", "" );

		if ( !drumkitPath.isEmpty() ) {
		    sFilename = drumkitPath + "/" + sFilename;
		}
		Sample *pSample = Sample::load( sFilename );
		if ( pSample == NULL ) {
		    // nel passaggio tra 0.8.2 e 0.9.0 il drumkit di default e' cambiato.
		    // Se fallisce provo a caricare il corrispettivo file in formato flac
//					warningLog( "[readSong] Error loading sample: " + sFilename + " not found. Trying to load a flac..." );
		    sFilename = sFilename.left( sFilename.length() - 4 );
		    sFilename += ".flac";
		    pSample = Sample::load( sFilename );
		}
		if ( pSample == NULL ) {
		    ERRORLOG( "Error loading sample: " + sFilename + " not found" );
		    pInstrument->set_muted( true );
		}
		InstrumentLayer *pLayer = new InstrumentLayer( pSample );
		pInstrument->set_layer( pLayer, 0 );
	    }
	    //~ back compatibility code
	    else {
		unsigned nLayer = 0;
		QDomNode layerNode = instrumentNode.firstChildElement( "layer" );
		while (  ! layerNode.isNull()  ) {
		    if ( nLayer >= MAX_LAYERS ) {
			ERRORLOG( "nLayer > MAX_LAYERS" );
			continue;
		    }
		    QString sFilename = LocalFileMng::readXmlString( layerNode, "filename", "" );
		    float fMin = LocalFileMng::readXmlFloat( layerNode, "min", 0.0 );
		    float fMax = LocalFileMng::readXmlFloat( layerNode, "max", 1.0 );
		    float fGain = LocalFileMng::readXmlFloat( layerNode, "gain", 1.0 );
		    float fPitch = LocalFileMng::readXmlFloat( layerNode, "pitch", 0.0, false, false );

		    if ( !drumkitPath.isEmpty() ) {
			sFilename = drumkitPath + "/" + sFilename;
		    }
		    Sample *pSample = Sample::load( sFilename );
		    if ( pSample == NULL ) {
			ERRORLOG( "Error loading sample: " + sFilename + " not found" );
			pInstrument->set_muted( true );
		    }
		    InstrumentLayer *pLayer = new InstrumentLayer( pSample );
		    pLayer->set_velocity_range( fMin, fMax );
		    pLayer->set_gain( fGain );
		    pLayer->set_pitch( fPitch );
		    pInstrument->set_layer( pLayer, nLayer );
		    nLayer++;

		    layerNode = ( QDomNode ) layerNode.nextSiblingElement( "layer" );
		}
	    }

	    instrumentList->add( pInstrument );
	    instrumentNode = (QDomNode) instrumentNode.nextSiblingElement( "instrument" );
	}
	if ( instrumentList_count == 0 ) {
	    WARNINGLOG( "0 instruments?" );
	}

	song->set_instrument_list( instrumentList );
    } else {
	ERRORLOG( "Error reading song: instrumentList node not found" );
	delete song;
	return NULL;
    }


	
    // Pattern list
    QDomNode patterns = songNode.firstChildElement( "patternList" );

    PatternList *patternList = new PatternList();
    int pattern_count = 0;

    QDomNode patternNode =  patterns.firstChildElement( "pattern" );
    while (  !patternNode.isNull()  ) {
	pattern_count++;
	Pattern *pat = getPattern( patternNode, instrumentList );
	if ( pat ) {
	    patternList->add( pat );
	} else {
	    ERRORLOG( "Error loading pattern" );
	    delete patternList;
	    delete song;
	    return NULL;
	}
	patternNode = ( QDomNode ) patternNode.nextSiblingElement( "pattern" );
    }
    if ( pattern_count == 0 ) {
	WARNINGLOG( "0 patterns?" );
    }
    song->set_pattern_list( patternList );
	
	
	
    // Pattern sequence
    QDomNode patternSequenceNode = songNode.firstChildElement( "patternSequence" );

    Song::pattern_group_t* pPatternGroupVector = new Song::pattern_group_t;
	
    // back-compatibility code..
    QDomNode pPatternIDNode = patternSequenceNode.firstChildElement( "patternID" );
    while ( ! pPatternIDNode.isNull()  ) {
	WARNINGLOG( "Using old patternSequence code for back compatibility" );
	PatternList *patternSequence = new PatternList();
	QString patId = pPatternIDNode.firstChildElement().text();
	ERRORLOG(patId);

	Pattern *pat = NULL;
	for ( unsigned i = 0; i < patternList->get_size(); i++ ) {
	    Pattern *tmp = patternList->get( i );
	    if ( tmp ) {
		if ( tmp->get_name() == patId ) {
		    pat = tmp;
		    break;
		}
	    }
	}
	if ( pat == NULL ) {
	    WARNINGLOG( "patternid not found in patternSequence" );
	    pPatternIDNode = ( QDomNode ) pPatternIDNode.nextSiblingElement( "patternID" );
	    continue;
	}
	patternSequence->add( pat );

	pPatternGroupVector->push_back( patternSequence );

	pPatternIDNode = ( QDomNode ) pPatternIDNode.nextSiblingElement( "patternID" );
    }

    QDomNode groupNode = patternSequenceNode.firstChildElement( "group" );
    while (  !groupNode.isNull()  ) {
	PatternList *patternSequence = new PatternList();
	QDomNode patternId = groupNode.firstChildElement( "patternID" );
	while (  !patternId.isNull()  ) {
	    QString patId = patternId.firstChild().nodeValue();

	    Pattern *pat = NULL;
	    for ( unsigned i = 0; i < patternList->get_size(); i++ ) {
		Pattern *tmp = patternList->get( i );
		if ( tmp ) {
		    if ( tmp->get_name() == patId ) {
			pat = tmp;
			break;
		    }
		}
	    }
	    if ( pat == NULL ) {
		WARNINGLOG( "patternid not found in patternSequence" );
		patternId = ( QDomNode ) patternId.nextSiblingElement( "patternID" );
		continue;
	    }
	    patternSequence->add( pat );
	    patternId = ( QDomNode ) patternId.nextSiblingElement( "patternID" );
	}
	pPatternGroupVector->push_back( patternSequence );

	groupNode = groupNode.nextSiblingElement( "group" );
    }

    song->set_pattern_group_vector( pPatternGroupVector );
	
#ifdef LADSPA_SUPPORT
    // reset FX
    for ( int fx = 0; fx < MAX_FX; ++fx ) {
	//LadspaFX* pFX = Engine::get_instance()->get_effects()->getLadspaFX( fx );
	//delete pFX;
	Engine::get_instance()->get_effects()->setLadspaFX( NULL, fx );
    }
#endif
	
    // LADSPA FX
    QDomNode ladspaNode = songNode.firstChildElement( "ladspa" );
    if ( !ladspaNode.isNull() ) {
	int nFX = 0;
	QDomNode fxNode = ladspaNode.firstChildElement( "fx" );
	while (  !fxNode.isNull()  ) {
	    QString sName = LocalFileMng::readXmlString( fxNode, "name", "" );
	    QString sFilename = LocalFileMng::readXmlString( fxNode, "filename", "" );
	    bool bEnabled = LocalFileMng::readXmlBool( fxNode, "enabled", false );
	    float fVolume = LocalFileMng::readXmlFloat( fxNode, "volume", 1.0 );

	    if ( sName != "no plugin" ) {
		// FIXME: il caricamento va fatto fare all'engine, solo lui sa il samplerate esatto
#ifdef LADSPA_SUPPORT
		LadspaFX* pFX = LadspaFX::load( sFilename, sName, 44100 );
		Engine::get_instance()->get_effects()->setLadspaFX( pFX, nFX );
		if ( pFX ) {
		    pFX->setEnabled( bEnabled );
		    pFX->setVolume( fVolume );
		    QDomNode inputControlNode = fxNode.firstChildElement( "inputControlPort" );
		    while ( !inputControlNode.isNull() ) {
			QString sName = LocalFileMng::readXmlString( inputControlNode, "name", "" );
			float fValue = LocalFileMng::readXmlFloat( inputControlNode, "value", 0.0 );

			for ( unsigned nPort = 0; nPort < pFX->inputControlPorts.size(); nPort++ ) {
			    LadspaControlPort *port = pFX->inputControlPorts[ nPort ];
			    if ( QString( port->sName ) == sName ) {
				port->fControlValue = fValue;
			    }
			}
			inputControlNode = ( QDomNode ) inputControlNode.nextSiblingElement( "inputControlPort" );
		    }

		    /*
		      TiXmlNode* outputControlNode;
		      for ( outputControlNode = fxNode->FirstChild( "outputControlPort" ); outputControlNode; outputControlNode = outputControlNode->NextSibling( "outputControlPort" ) ) {
		      }*/
		}
#endif
	    }
	    nFX++;
	    fxNode = ( QDomNode ) fxNode.nextSiblingElement( "fx" );
	}
    } else {
	WARNINGLOG( "ladspa node not found" );
    }

	
    song->set_modified(false);
    song->set_filename( filename );
	

    return song;
	
}



Pattern* SongReader::getPattern( QDomNode pattern, InstrumentList* instrList )
{
    Pattern *pPattern = NULL;

    QString sName;	// name
    sName = LocalFileMng::readXmlString( pattern, "name", sName );

    QString sCategory = ""; // category
    sCategory = LocalFileMng::readXmlString( pattern, "category", sCategory );
    int nSize = -1;
    nSize = LocalFileMng::readXmlInt( pattern, "size", nSize, false, false );

    pPattern = new Pattern( sName, sCategory, nSize );



    QDomNode pNoteListNode = pattern.firstChildElement( "noteList" );
    if ( ! pNoteListNode.isNull() ) {
	// new code :)
	QDomNode noteNode = pNoteListNode.firstChildElement( "note" );
	while ( ! noteNode.isNull()  ) {

	    Note* pNote = NULL;

	    unsigned nPosition = LocalFileMng::readXmlInt( noteNode, "position", 0 );
	    float fLeadLag = LocalFileMng::readXmlFloat( noteNode, "leadlag", 0.0 , false , false );
	    float fVelocity = LocalFileMng::readXmlFloat( noteNode, "velocity", 0.8f );
	    float fPan_L = LocalFileMng::readXmlFloat( noteNode, "pan_L", 0.5 );
	    float fPan_R = LocalFileMng::readXmlFloat( noteNode, "pan_R", 0.5 );
	    int nLength = LocalFileMng::readXmlInt( noteNode, "length", -1, true );
	    float nPitch = LocalFileMng::readXmlFloat( noteNode, "pitch", 0.0, false, false );
	    QString sKey = LocalFileMng::readXmlString( noteNode, "key", "C0", false, false );

	    QString instrId = LocalFileMng::readXmlString( noteNode, "instrument", "" );

	    Instrument *instrRef = NULL;
	    // search instrument by ref
	    for ( unsigned i = 0; i < instrList->get_size(); i++ ) {
		Instrument *instr = instrList->get( i );
		if ( instrId == instr->get_id() ) {
		    instrRef = instr;
		    break;
		}
	    }
	    if ( !instrRef ) {
		ERRORLOG( "Instrument with ID: '" + instrId + "' not found. Note skipped." );
		continue;
	    }
	    //assert( instrRef );

	    pNote = new Note( instrRef, fVelocity, fPan_L, fPan_R, nLength, nPitch, Note::stringToKey( sKey ) );
	    pNote->set_leadlag(fLeadLag);
	    pPattern->note_map.insert( std::make_pair( nPosition, pNote ) );
			
	    noteNode = ( QDomNode ) noteNode.nextSiblingElement( "note" );
	}
    } else {
	// Back compatibility code. Version < 0.9.4
	QDomNode sequenceListNode = pattern.firstChildElement( "sequenceList" );

	int sequence_count = 0;
	QDomNode sequenceNode = sequenceListNode.firstChildElement( "sequence" );
	while ( ! sequenceNode.isNull()  ) {
	    sequence_count++;

	    QDomNode noteListNode = sequenceNode.firstChildElement( "noteList" );
	    QDomNode noteNode = noteListNode.firstChildElement( "note" );
	    while (  !noteNode.isNull() ) {

		Note* pNote = NULL;

		unsigned nPosition = LocalFileMng::readXmlInt( noteNode, "position", 0 );
		float fLeadLag = LocalFileMng::readXmlFloat( noteNode, "leadlag", 0.0 , false , false );
		float fVelocity = LocalFileMng::readXmlFloat( noteNode, "velocity", 0.8f );
		float fPan_L = LocalFileMng::readXmlFloat( noteNode, "pan_L", 0.5 );
		float fPan_R = LocalFileMng::readXmlFloat( noteNode, "pan_R", 0.5 );
		int nLength = LocalFileMng::readXmlInt( noteNode, "length", -1, true );
		float nPitch = LocalFileMng::readXmlFloat( noteNode, "pitch", 0.0, false, false );

		QString instrId = LocalFileMng::readXmlString( noteNode, "instrument", "" );

		Instrument *instrRef = NULL;
		// search instrument by ref
		for ( unsigned i = 0; i < instrList->get_size(); i++ ) {
		    Instrument *instr = instrList->get( i );
		    if ( instrId == instr->get_id() ) {
			instrRef = instr;
			break;
		    }
		}
		assert( instrRef );

		pNote = new Note( instrRef, fVelocity, fPan_L, fPan_R, nLength, nPitch );
		pNote->set_leadlag(fLeadLag);

		//infoLog( "new note!! pos: " + toString( pNote->m_nPosition ) + "\t instr: " + instrId );
		pPattern->note_map.insert( std::make_pair( nPosition, pNote ) );
			
		noteNode = ( QDomNode ) noteNode.nextSiblingElement( "note" );

				
	    }
	    sequenceNode = ( QDomNode ) sequenceNode.nextSiblingElement( "sequence" );
	}
    }

    return pPattern;
}


