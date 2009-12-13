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
#include <Tritium/Engine.hpp>

#include <QDomDocument>

#include "SerializationPrivate.hpp"

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
    Song* Song::load( Engine* engine, const QString& filename )
    {
	Song *song = NULL;

	SongReader reader;
	song = reader.readSong( engine, filename );

	return song;
    }



    /// Save a song to file
    bool Song::save( Engine* engine, const QString& filename )
    {
	SongWriter writer;
	int err;
	err = writer.writeSong( engine, this, filename );

	if( err ) {
	    return false;
	}
	return QFile::exists( filename );
    }


    /// Create default song
    Song* Song::get_default_song(Engine* engine){
	Song *song = new Song( "empty", "Tritium", 120, 0.5 );

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
	engine->renameJackPorts();
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
    Song* Song::get_empty_song(Engine* engine)
    {
	QString dataDir = DataPath::get_data_path();	
	QString filename = dataDir + "/DefaultSong.h2song";

	if( ! QFile::exists( filename ) ){
	    ERRORLOG("File " + filename + " exists not. Failed to load default song.");
	    filename = dataDir + "/DefaultSong.h2song";
	}
	
	Song *song = Song::load( engine, filename );
	
	/* if file DefaultSong.h2song not accessible
	 * create a simple default song.
	 */
	if(!song){
	    song = Song::get_default_song(engine);
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
