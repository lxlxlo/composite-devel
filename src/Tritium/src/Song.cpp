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

#include "config.h"
#include "version.h"

#include "SongPrivate.hpp"
#include "PatternModeList.hpp"
#include "PatternModeManager.hpp"

#include <cassert>
#include <QtCore/QMutex>
#include <QtCore/QMutexLocker>
#include <algorithm>

#include <Tritium/ADSR.hpp>
#include <Tritium/DataPath.hpp>
#include <Tritium/LocalFileMng.hpp>
#include <Tritium/Logger.hpp>

#include <Tritium/fx/Effects.hpp>
#include <Tritium/globals.hpp>
#include <Tritium/Song.hpp>
#include <Tritium/Sample.hpp>
#include <Tritium/Instrument.hpp>
#include <Tritium/InstrumentLayer.hpp>
#include <Tritium/InstrumentList.hpp>
#include <Tritium/Pattern.hpp>
#include <Tritium/Note.hpp>
#include <Tritium/Hydrogen.hpp>

#include <QDomDocument>

namespace Tritium
{

    Song::SongPrivate::SongPrivate(
	const QString& name_p,
	const QString& author,
	float bpm,
	float volume )
	: is_muted( false )
	, resolution( 48 )
	, bpm( bpm )
	, is_modified( false )
	, name( name_p )
	, author( author )
	, volume( volume )
	, metronome_volume( 0.5 )
	, pattern_list( NULL )
	, pattern_group_sequence( NULL )
	, instrument_list( NULL )
	, filename( "" )
	, is_loop_enabled( false )
	, humanize_time_value( 0.0 )
	, humanize_velocity_value( 0.0 )
	, swing_factor( 0.0 )
	, song_mode( Song::PATTERN_MODE )
    {
	INFOLOG( QString( "INIT '%1'" ).arg( name ) );
	pat_mode = new PatternModeManager();
    }

    Song::SongPrivate::~SongPrivate()
    {
	// delete all patterns
	delete pattern_list;

	if ( pattern_group_sequence ) {
	    for ( unsigned i = 0; i < pattern_group_sequence->size(); ++i ) {
		PatternList *pPatternList = ( *pattern_group_sequence )[i];
		pPatternList->clear();	// pulisco tutto, i pattern non vanno distrutti qua
		delete pPatternList;
	    }
	    delete pattern_group_sequence;
	}

	delete instrument_list;

	INFOLOG( QString( "DESTROY '%1'" ).arg( name ) );
    }

    /*****************************************************************
     * Implementation of Song
     *****************************************************************
     */

    Song::Song( const QString& name, const QString& author, float bpm, float volume )
    {
	d = new SongPrivate(name, author, bpm, volume);
    }

    Song::~Song()
    {
	delete d;
    }

    void Song::purge_instrument( Instrument * I )
    {
	for ( int nPattern = 0; nPattern < (int)d->pattern_list->get_size(); ++nPattern ) {
	    d->pattern_list->get( nPattern )->purge_instrument( I );
	}
    }

    void Song::set_volume( float volume )
    {
	d->volume = volume;
    }

    float Song::get_volume()
    {
	return d->volume;
    }

    void Song::set_metronome_volume( float volume )
    {
	d->metronome_volume = volume;
    }

    float Song::get_metronome_volume()
    {
	return d->metronome_volume;
    }

    void Song::set_mute(bool m)
    {
	d->is_muted = m;
    }

    bool Song::get_mute()
    {
	return d->is_muted;
    }

    void Song::set_resolution(unsigned r)
    {
	d->resolution = r;
    }

    unsigned Song::get_resolution()
    {
	return d->resolution;
    }

    void Song::set_bpm(float r)
    {
	d->bpm = r;
    }

    float Song::get_bpm()
    {
	return d->bpm;
    }

    void Song::set_modified(bool m)
    {
	d->is_modified = m;
    }

    bool Song::get_modified()
    {
	return d->is_modified;
    }

    void Song::set_name(const QString& name_p)
    {
	d->name = name_p;
    }

    const QString& Song::get_name()
    {
	return d->name;
    }

    void Song::set_author(const QString& auth)
    {
	d->author = auth;
    }

    const QString& Song::get_author()
    {
	return d->author;
    }

    PatternList* Song::get_pattern_list()
    {
	return d->pattern_list;
    }

    void Song::set_pattern_list( PatternList *pattern_list )
    {
	d->pattern_list = pattern_list;
    }

    Song::pattern_group_t* Song::get_pattern_group_vector()
    {
	return d->pattern_group_sequence;
    }

    void Song::set_pattern_group_vector( Song::pattern_group_t* vect )
    {
	d->pattern_group_sequence = vect;
    }

    InstrumentList* Song::get_instrument_list()
    {
	return d->instrument_list;
    }

    void Song::set_instrument_list( InstrumentList *list )
    {
	d->instrument_list = list;
    }

    void Song::set_notes( const QString& notes )
    {
	d->notes = notes;
    }

    const QString& Song::get_notes()
    {
	return d->notes;
    }

    void Song::set_license( const QString& license )
    {
	d->license = license;
    }

    const QString& Song::get_license()
    {
	return d->license;
    }

    const QString& Song::get_filename()
    {
	return d->filename;
    }

    void Song::set_filename( const QString& filename )
    {
	d->filename = filename;
    }

    bool Song::is_loop_enabled()
    {
	return d->is_loop_enabled;
    }

    void Song::set_loop_enabled( bool enabled )
    {
	d->is_loop_enabled = enabled;
    }

    float Song::get_humanize_time_value()
    {
	return d->humanize_time_value;
    }

    void Song::set_humanize_time_value( float value )
    {
	d->humanize_time_value = value;
    }

    float Song::get_humanize_velocity_value()
    {
	return d->humanize_velocity_value;
    }

    void Song::set_humanize_velocity_value( float value )
    {
	d->humanize_velocity_value = value;
    }

    float Song::get_swing_factor()
    {
	return d->swing_factor;
    }

    Song::SongMode Song::get_mode()
    {
	return d->song_mode;
    }

    void Song::set_mode( SongMode mode )
    {
	d->song_mode = mode;
    }

    ///Load a song from file
    Song* Song::load( const QString& filename )
    {
	Song *song = NULL;

	SongReader reader;
	song = reader.readSong( filename );

	return song;
    }



    /// Save a song to file
    bool Song::save( const QString& filename )
    {
	SongWriter writer;
	int err;
	err = writer.writeSong( this, filename );

	if( err ) {
	    return false;
	}
	return QFile::exists( filename );
    }


    /// Create default song
    Song* Song::get_default_song(){
	Song *song = new Song( "empty", "hydrogen", 120, 0.5 );

	song->set_metronome_volume( 0.5 );
	song->set_notes( "..." );
	song->set_license( "" );
	song->set_loop_enabled( false );
	song->set_mode( Song::PATTERN_MODE );
	song->set_humanize_time_value( 0.0 );
	song->set_humanize_velocity_value( 0.0 );
	song->set_swing_factor( 0.0 );

	InstrumentList* pList = new InstrumentList();
	Instrument *pNewInstr = new Instrument(QString( 0 ), "New instrument", new ADSR());
	pList->add( pNewInstr );
	song->set_instrument_list( pList );
		
#ifdef JACK_SUPPORT
	Hydrogen::get_instance()->renameJackPorts();
#endif

	PatternList *patternList = new PatternList();
	Pattern *emptyPattern = Pattern::get_empty_pattern(); 
	emptyPattern->set_name( QString("Pattern 1") ); 
	emptyPattern->set_category( QString("not_categorized") );
	patternList->add( emptyPattern );
	song->set_pattern_list( patternList );
	pattern_group_t* pPatternGroupVector = new pattern_group_t;
	PatternList *patternSequence = new PatternList();
	patternSequence->add( emptyPattern );
	pPatternGroupVector->push_back( patternSequence );
	song->set_pattern_group_vector( pPatternGroupVector );
	song->d->is_modified = false;
	song->set_filename( "empty_song" );
		
	return song;
    }

    /// Return an empty song
    Song* Song::get_empty_song()
    {
	QString dataDir = DataPath::get_data_path();	
	QString filename = dataDir + "/DefaultSong.h2song";

	if( ! QFile::exists( filename ) ){
	    ERRORLOG("File " + filename + " exists not. Failed to load default song.");
	    filename = dataDir + "/DefaultSong.h2song";
	}
	
	Song *song = Song::load( filename );
	
	/* if file DefaultSong.h2song not accessible
	 * create a simple default song.
	 */
	if(!song){
	    song = Song::get_default_song();
	}

	return song;
    }



    void Song::set_swing_factor( float factor )
    {
	if ( factor < 0.0 ) {
	    factor = 0.0;
	} else if ( factor > 1.0 ) {
	    factor = 1.0;
	}

	d->swing_factor = factor;
    }

    /***********************************
     * Methods useful to sequencers.
     ***********************************
     */

    /**
     * Returns the number of measures in a song.
     *
     * Returns -1 if there is an error.
     */
    uint32_t Song::song_bar_count()
    {
	return get_pattern_group_vector()->size();
    }

    /**
     * Returns the number of ticks in a song.  Returns -1 if there was an error
     * (s == 0).
     */
    uint32_t Song::song_tick_count()
    {
	uint32_t count = 0;
	uint32_t bar = 1;
	uint32_t tmp;

	tmp = ticks_in_bar(bar);
	while( tmp != unsigned(-1) ) {
	    count += tmp;
	    ++bar;
	    tmp = ticks_in_bar(bar);
	}
	return count;
    }



    /**
     * Returns the index of the pattern group that represents measure number
     * 'bar'.  Returns -1 if bar > song_bar_count(s).
     */
    uint32_t Song::pattern_group_index_for_bar(uint32_t bar)
    {
	if( bar <= song_bar_count() ) {
	    return bar-1;
	}
	return -1;
    }

    /**
     * Returns the bar number of the pattern group that contains the given
     * absolute tick.  (Always assuming tick 0 is at 1:1.0000.  Returns -1 if
     * there is an error (s == 0) or of the tick is beyond the end of the song.
     */
    uint32_t Song::bar_for_absolute_tick(uint32_t abs_tick)
    {
	uint32_t tick_count = abs_tick;
	uint32_t bar_count = 1;
	uint32_t tmp;

	tmp = ticks_in_bar(bar_count);
	while( tick_count >= tmp ) {
	    tick_count -= tmp;
	    ++bar_count;
	    tmp = ticks_in_bar(bar_count);
	}
	return bar_count;
    }



    /**
     * Returns the absolute tick number for the start of this bar.
     */
    uint32_t Song::bar_start_tick(uint32_t bar)
    {
	if( bar > song_bar_count() ) return -1;
	uint32_t count = 0, k = 1;
	while( k < bar ) {
	    count += ticks_in_bar(k);
	    ++k;
	}
	return count;
    }



    /**
     * Returns the number of ticks in measure 'bar'
     */
    uint32_t Song::ticks_in_bar(uint32_t bar)
    {
	if( bar < 1 ) return -1;
	if( bar > song_bar_count() ) return -1;

	PatternList* list = get_pattern_group_vector()->at(bar-1);
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

    // PATTERN MODE METHODS

    Song::PatternModeType Song::get_pattern_mode_type()
    {
	return d->pat_mode->get_pattern_mode_type();
    }

    void Song::set_pattern_mode_type(Song::PatternModeType t)
    {
	d->pat_mode->set_pattern_mode_type(t);
    }

    void Song::toggle_pattern_mode_type()
    {
	d->pat_mode->toggle_pattern_mode_type();
    }

    void Song::append_pattern(int pos)
    {
	d->pat_mode->append_pattern(pos);
    }

    void Song::remove_pattern(int pos)
    {
	d->pat_mode->remove_pattern(pos);
    }

    void Song::reset_patterns()
    {
	d->pat_mode->reset_patterns();
    }

    void Song::set_next_pattern(int pos)
    {
	d->pat_mode->set_next_pattern(pos);
    }

    void Song::append_next_pattern(int pos)
    {
	d->pat_mode->append_next_pattern(pos);
    }

    void Song::remove_next_pattern(int pos)
    {
	d->pat_mode->remove_next_pattern(pos);
    }

    void Song::clear_queued_patterns()
    {
	d->pat_mode->clear_queued_patterns();
    }

    // Copies the currently playing patterns into rv.
    void Song::get_playing_patterns(PatternList& rv)
    {
	PatternModeList::list_type vec;
	PatternModeList::list_type::iterator k;
	d->pat_mode->get_playing_patterns(vec);
	rv.clear();
	for( k = vec.begin() ; k != vec.end() ; ++k ) {
	    if( (*k > 0) && (*k < d->pattern_list->get_size()) ) {
		rv.add( d->pattern_list->get(*k) );
	    } else {
		remove_pattern(*k);
	    }
	}
    }

    void Song::go_to_next_patterns()
    {
	d->pat_mode->go_to_next_patterns();
    }

    //::::::::::::::::::::

    //-----------------------------------------------------------------------------
    //	Implementation of SongReader class
    //-----------------------------------------------------------------------------

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
	    WARNINGLOG( "Trying to load a song created with a different version of hydrogen." );
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
		Hydrogen::get_instance()->setCurrentDrumkitname( sDrumkit ); 
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
	    //LadspaFX* pFX = Effects::get_instance()->getLadspaFX( fx );
	    //delete pFX;
	    Effects::get_instance()->setLadspaFX( NULL, fx );
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
		    Effects::get_instance()->setLadspaFX( pFX, nFX );
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

    ////////////////////////////////////////////////////////////////
    // PatternModeList
    ////////////////////////////////////////////////////////////////

    PatternModeList::PatternModeList()
    {
    }

    void PatternModeList::reserve(size_t size)
    {
	QMutexLocker mx(&__mutex);
	__vec.reserve(size);
    }

    size_t PatternModeList::size()
    {
	return __vec.size();
    }

    void PatternModeList::add(PatternModeList::value_type d)
    {
	QMutexLocker mx(&__mutex);
	iterator k = find(__vec.begin(), __vec.end(), d);
	if( k != __vec.end() ) {
	    __vec.push_back(d);
	}
    }

    void PatternModeList::remove(PatternModeList::value_type d)
    {
	QMutexLocker mx(&__mutex);
	iterator k;
	while(true) {
	    k = find(__vec.begin(), __vec.end(), d);
	    if( k != __vec.end() ) {
		__vec.erase(k);
	    } else {
		break;
	    }
	}
    }

    void PatternModeList::clear()
    {
	QMutexLocker mx(&__mutex);
	__vec.clear();
    }

    QMutex& PatternModeList::get_mutex()
    {
	return __mutex;
    }

    // This method is *not* thread safe and must be protected by
    // an external mutex.
    PatternModeList::iterator PatternModeList::begin()
    {
	return __vec.begin();
    }

    // This method is *not* thread safe and must be protected by
    // an external mutex.
    PatternModeList::iterator PatternModeList::end()
    {
	return __vec.end();
    }

    ///////////////////////////////////////////////////////////////////////
    // PatternModeManager
    ///////////////////////////////////////////////////////////////////////

    PatternModeManager::PatternModeManager() :
	__type( Song::SINGLE )
    {
	__current.reserve(64);
	__append.reserve(64);
	__delete.reserve(64);
	__next.reserve(64);
    }

    Song::PatternModeType PatternModeManager::get_pattern_mode_type()
    {
	return __type;
    }

    void PatternModeManager::set_pattern_mode_type(Song::PatternModeType t)
    {
	__type = t;
    }

    void PatternModeManager::toggle_pattern_mode_type()
    {
	Song::PatternModeType t = get_pattern_mode_type();
	if( t == Song::SINGLE ) {
	    set_pattern_mode_type( Song::STACKED );
	} else {
	    set_pattern_mode_type( Song::SINGLE );
	}
    }

    void PatternModeManager::append_pattern(int pos)
    {
	if( __type == Song::SINGLE ) {
	    __append.clear();
	}
	__append.add(pos);
    }

    void PatternModeManager::remove_pattern(int pos)
    {
	__delete.add(pos);
    }

    void PatternModeManager::reset_patterns()
    {
	QMutexLocker mx(&__mutex);
	__append.clear();
	__delete.clear();
	__next.clear();
	__append.add(0);
	QMutexLocker cmx(&__current.get_mutex());
	PatternModeList_t::iterator k;
	for( k = __current.begin() ; k != __current.end() ; ++k ) {
	    __delete.add(*k);
	}
    }

    void PatternModeManager::set_next_pattern(int pos)
    {
	__next.clear();
	__next.add(pos);
    }

    void PatternModeManager::append_next_pattern(int pos)
    {
	if( __type == Song::SINGLE ) {
	    __next.clear();
	}
	__next.add(pos);
    }

    void PatternModeManager::remove_next_pattern(int pos)
    {
	__next.remove(pos);
    }

    void PatternModeManager::clear_queued_patterns()
    {
	__next.clear();
    }

    void PatternModeManager::get_playing_patterns(PatternModeList_t::list_type& pats)
    {
	QMutexLocker mx(&__current.get_mutex());
	PatternModeList_t::iterator k;
	pats.clear();
	if( __type == Song::SINGLE ) {
	    pats.push_back( *__current.begin() );
	    return;
	}
	assert( __type == Song::STACKED );
	for(k = __current.begin() ; k != __current.end() ; ++k ) {
	    pats.push_back(*k);
	    if( __type == Song::SINGLE ) break;
	}
    }

    void PatternModeManager::go_to_next_patterns()
    {
	QMutexLocker mx(&__mutex);

	if( __next.size() != 0 ) {
	    __append.clear();
	    __delete.clear();
	    __current.clear();
	    QMutexLocker nmx(&__next.get_mutex());
	    PatternModeList_t::iterator k;
	    for( k = __next.begin() ; k != __next.end() ; ++k ) {
		__current.add(*k);
		if( __type == Song::SINGLE ) break;
	    }		
	} else {
	    PatternModeList_t::iterator k;
	    QMutexLocker dmx(&__delete.get_mutex());
	    for( k = __delete.begin() ; k != __delete.end() ; ++k ) {
		__delete.add(*k);
	    }
	    dmx.unlock();
	    QMutexLocker amx(&__append.get_mutex());
	    for( k = __append.begin() ; k != __append.end() ; ++k ) {
		if( __current.size() >= 1 ) break;
		__current.add(*k);
	    }
	}
    }

} // namespace Tritium
