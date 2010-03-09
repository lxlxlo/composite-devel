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

/**
 * t_Serialization.cpp
 *
 * This is just the template for a test.  If the template is not
 * executable, it will not be kept up-to-date.
 */

#include <Tritium/Serialization.hpp>
#include <Tritium/ObjectBundle.hpp>
#include <Tritium/memory.hpp>
#include <Tritium/Logger.hpp>
#include <Tritium/Engine.hpp>
#include <Tritium/Song.hpp>
#include <Tritium/Pattern.hpp>
#include <Tritium/Instrument.hpp>
#include <Tritium/InstrumentList.hpp>
#include <Tritium/InstrumentLayer.hpp>
#include <Tritium/Mixer.hpp>
#include <Tritium/Sample.hpp>
#include <Tritium/Sampler.hpp>
#include <Tritium/SoundLibrary.hpp>
#include <Tritium/Preferences.hpp>
#include <Tritium/fx/Effects.hpp>
#include <Tritium/fx/LadspaFX.hpp>
#include <Tritium/Note.hpp>
#include <Tritium/ADSR.hpp>
#include <cstdio>
#include <cstdlib>
#include <deque>
#include <QString>
#include <QDir>
#include <sys/stat.h> // for mkdir()
#include <sys/types.h> // for mkdir()

// CHANGE THIS TO MATCH YOUR FILE:
#define THIS_NAMESPACE t_Serialization
#include "test_macros.hpp"
#include "test_config.hpp"

using namespace Tritium;
using namespace Tritium::Serialization;

namespace THIS_NAMESPACE
{
    class SyncBundle : public ObjectBundle
    {
    public:
        bool done;

        SyncBundle() : done(false) {}
        void operator()() { done = true; }
    };

    class SyncSaveReport : public SaveReport
    {
    public:
	bool done;

	SyncSaveReport() : done(false) {}
	void operator()() { done = true; }
    };

    const char temp_dir[] = "t_Serialization_tmp";
    const char song_file_name[] = TEST_DATA_DIR "/t_Serialization.h2song";
    const char song_master_default_name[] = TEST_BIN_DIR "/t_Serialization-default.h2song";
    const char pattern_file_name[] = TEST_DATA_DIR "/t_Serialization.h2pattern";
    const char drumkit_manifest_file_name[] = TEST_DATA_DIR "/t_Serialization-drumkit/drumkit.xml";
    /**
     * Recursively removes files and folders.
     *
     * Thanks to:
     * http://lists.trolltech.com/qt-interest/2008-02/thread00720-0.html
     */
    bool RemoveDirectory(QDir& aDir)
    {
	bool has_err = false;
	if( aDir.exists() ) {
	    QFileInfoList entries = aDir.entryInfoList(
		QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files
		);
	    int count = entries.size();
	    for (int idx = 0; ((idx < count) && (false == has_err)); idx++)
	    {
		QFileInfo entryInfo = entries[idx];
		QString path = entryInfo.absoluteFilePath();
		if (entryInfo.isDir())
		{
		    QDir sub(path);
		    has_err = RemoveDirectory(sub);
		}
		else
		{
		    QFile file(path);
		    if (!file.remove())
			has_err = true;
		}
	    }
	    if (!aDir.rmdir(aDir.absolutePath()))
		has_err = true;
	}
	return has_err;
    }

    struct Fixture
    {
        // SETUP AND TEARDOWN OBJECTS FOR YOUR TESTS.
        T<Serializer>::auto_ptr s;
        T<Engine>::auto_ptr engine;
        Fixture() : s(0) {
            Logger::create_instance();
            T<Preferences>::shared_ptr prefs(new Preferences );
            engine.reset( new Engine(prefs) );
            s.reset( Serializer::create_standalone(engine.get()) );
            int rv;
	    ::remove(temp_dir);
            rv = ::mkdir(temp_dir, 0700);
            BOOST_REQUIRE( rv == 0 );
        }
        ~Fixture() {
	    bool has_error;
	    QDir td(temp_dir);
	    has_error = RemoveDirectory(td);
            BOOST_REQUIRE( ! has_error );
	    BOOST_REQUIRE( ! td.exists() );
            s.reset();
            engine.reset();
            delete Logger::get_instance();
        }
    };

} // namespace THIS_NAMESPACE

TEST_BEGIN( Fixture );

TEST_CASE( 000_load_invalid_file_name )
{
    const char fn[] = TEST_DATA_DIR "/really_unlikely_filename.txt";
    SyncBundle bdl;

    s->load_uri(fn, bdl, engine.get());

    while( ! bdl.done ) {
        sleep(1);
    }

    CK( bdl.error );
    CK( bdl.empty() );
}

TEST_CASE( 010_load_song_check_song )
{
    SyncBundle bdl;

    s->load_uri(song_file_name, bdl, engine.get());

    while( ! bdl.done ) {
        sleep(1);
    }

    BOOST_REQUIRE( ! bdl.error );

    // Sort out all the components:
    std::deque< T<Song>::shared_ptr > songs;
    std::deque< T<Pattern>::shared_ptr > patterns;
    std::deque< T<Instrument>::shared_ptr > instruments;
    std::deque< T<Mixer::Channel>::shared_ptr > channels;
    std::deque< T<LadspaFX>::shared_ptr > effects;

    while( ! bdl.empty() ) {
        switch(bdl.peek_type()) {
        case ObjectItem::Song_t:
            songs.push_back( bdl.pop<Song>() );
            break;
        case ObjectItem::Pattern_t:
            patterns.push_back( bdl.pop<Pattern>() );
            break;
        case ObjectItem::Instrument_t:
            instruments.push_back( bdl.pop<Instrument>() );
            break;
        case ObjectItem::LadspaFX_t:
            effects.push_back( bdl.pop<LadspaFX>() );
            break;
	case ObjectItem::Channel_t:
	    channels.push_back( bdl.pop<Mixer::Channel>() );
	    break;
        default:
            BOOST_REQUIRE(false); // should not reach this.
        }
    }

    CK( songs.size() == 1 );
    CK( patterns.size() == 0 );
    CK( instruments.size() == 32 );
    CK( channels.size() == 32 );
    CK( effects.size() == 0 );

    /********************************************
     * SONG Object
     ********************************************
     */
    T<Song>::shared_ptr s = songs.front();

    // Metadata
    CK( s->get_name() == "Jazzy" );
    CK( s->get_volume() == 0.5 );
    CK( s->get_metronome_volume() == 0.5 );
    CK( s->get_mute() == false );
    CK( s->get_resolution() == 48 );
    CK( s->get_bpm() == 100.0f );
    CK( s->get_modified() == false );
    CK( s->get_author() == "Emiliano Grilli" );
    CK( s->get_license() == "Unknown license" );
    CK( s->get_filename() == song_file_name );

    // Pattern and Instrument
    CK( s->get_pattern_list()->get_size() == 3 );
    CK( s->get_pattern_group_vector()->size() == 8 );
    // Serializer doesn't load instruments into Song,
    // since that's about to be wrong.
    CK( s->get_notes() == "Jazzy..." );
    CK( s->is_loop_enabled() == true );
    CK( s->get_humanize_time_value() == 0.23f );
    CK( s->get_humanize_velocity_value() == 0.23f );
    CK( s->get_swing_factor() == 0.44f );
    CK( s->get_mode() == Song::SONG_MODE );

    // "songhelper" methods
    CK( s->song_bar_count() == 8 );
    CK( s->song_tick_count() == 1536 );
    CK( s->pattern_group_index_for_bar(1) == 0 );
    CK( s->bar_for_absolute_tick(0) == 1 );
    CK( s->bar_start_tick(1) == 0 );
    CK( s->ticks_in_bar(1) == 192 );

    // Pattern Mode methods
    CK( s->get_pattern_mode_type() == Song::SINGLE );
    Tritium::PatternList pl;
    s->get_playing_patterns(pl);
    CK( pl.get_size() == 0 );

    /********************************************
     * Instrument Objects
     ********************************************
     */
    // Instead of testing that _every_ instrument
    // loaded properly... we'll check 0, 1, 13,
    // 30, and 31.
    int k;
    std::deque< T<Instrument>::shared_ptr >::iterator inst;
    std::deque< T<Mixer::Channel>::shared_ptr >::iterator ch;
    for( k=0, inst=instruments.begin(), ch=channels.begin()
	     ; inst != instruments.end(), ch != channels.end()
	     ; ++k, ++inst, ++ch ) {
        Instrument& in = *(*inst);
        InstrumentLayer *lay = 0;
	Mixer::Channel& chan = **ch;

        T<Sample>::shared_ptr samp;
        switch(k) {
        case 0:
            CK( in.get_id() == "0" );
            CK( in.get_drumkit_name() == "GMkit" );
            CK( in.get_name() == "Kick" );
	    CK( chan.gain() == 1.0f );
            CK( in.is_muted() == false );
            CK( in.get_pan_l() == 1.0f );
            CK( in.get_pan_r() == 1.0f );
            CK( chan.send_gain(0) == 0.0f );
            CK( chan.send_gain(1) == 0.0f );
            CK( chan.send_gain(2) == 0.0f );
            CK( chan.send_gain(3) == 0.0f );
            lay = in.get_layer(0);
            CK( lay );
            CK( lay->get_min_velocity() == 0.0f );
            CK( lay->get_max_velocity() == 1.0f );
            CK( lay->get_gain() == 1.0 );
            CK( lay->get_pitch() == 0.0 );
            samp = lay->get_sample();
            CK( samp );
            CK( samp->get_filename().endsWith( "kick_Dry_b.flac" ) );
            CK( samp->get_sample_rate() == 44100U );
            CK( in.get_layer(1) == 0 );
            break;
        case 1:
            CK( in.get_id() == "1" );
            CK( in.get_drumkit_name() == "GMkit" );
            CK( in.get_name() == "Stick" );
            CK( chan.gain() == 0.69f );
            CK( in.is_muted() == false );
            CK( in.get_pan_l() == 1.0f );
            CK( in.get_pan_r() == 1.0f );
            CK( chan.send_gain(0) == 0.0f );
            CK( chan.send_gain(1) == 0.0f );
            CK( chan.send_gain(2) == 0.0f );
            CK( chan.send_gain(3) == 0.0f );
            lay = in.get_layer(0);
            CK( lay );
            CK( lay->get_min_velocity() == 0.0f );
            CK( lay->get_max_velocity() == 1.0f );
            CK( lay->get_gain() == 1.0 );
            CK( lay->get_pitch() == 0.0 );
            samp = lay->get_sample();
            CK( samp );
            CK( samp->get_filename().endsWith( "stick_Woody.flac" ) );
            CK( samp->get_sample_rate() == 44100U );
            CK( in.get_layer(1) == 0 );
            break;
        case 13:
            CK( in.get_id() == "13" );
            CK( in.get_drumkit_name() == "GMkit" );
            CK( in.get_name() == "Crash" );
            CK( chan.gain() == 0.69f );
            CK( in.is_muted() == false );
            CK( in.get_pan_l() == 1.0f );
            CK( in.get_pan_r() == 0.88f );
            CK( chan.send_gain(0) == 0.0f );
            CK( chan.send_gain(1) == 0.0f );
            CK( chan.send_gain(2) == 0.0f );
            CK( chan.send_gain(3) == 0.0f );
            lay = in.get_layer(0);
            CK( lay );
            CK( lay->get_min_velocity() == 0.0f );
            CK( lay->get_max_velocity() == 1.0f );
            CK( lay->get_gain() == 1.0 );
            CK( lay->get_pitch() == 0.0 );
            samp = lay->get_sample();
            CK( samp );
            CK( samp->get_filename().endsWith( "cra_Rock_a.flac" ) );
            CK( samp->get_sample_rate() == 44100U );
            CK( in.get_layer(1) == 0 );
            break;
        case 30:
            CK( in.get_id() == "30" );
            CK( in.get_drumkit_name() == "GMkit" );
            CK( in.get_name() == "31" );
            CK( chan.gain() == 0.8f );
            CK( in.is_muted() == false );
            CK( in.get_pan_l() == 1.0f );
            CK( in.get_pan_r() == 1.0f );
            CK( chan.send_gain(0) == 0.0f );
            CK( chan.send_gain(1) == 0.0f );
            CK( chan.send_gain(2) == 0.0f );
            CK( chan.send_gain(3) == 0.0f );
            lay = in.get_layer(0);
            CK( lay );
            CK( lay->get_min_velocity() == 0.0f );
            CK( lay->get_max_velocity() == 1.0f );
            CK( lay->get_gain() == 1.0 );
            CK( lay->get_pitch() == 0.0 );
            samp = lay->get_sample();
            CK( samp );
            CK( samp->get_filename().endsWith( "emptySample.flac" ) );
            CK( samp->get_sample_rate() == 44100U );
            CK( in.get_layer(1) == 0 );
            break;
        case 31:
            CK( in.get_id() == "31" );
            CK( in.get_drumkit_name() == "GMkit" );
            CK( in.get_name() == "32" );
            CK( chan.gain() == 0.8f );
            CK( in.is_muted() == false );
            CK( in.get_pan_l() == 1.0f );
            CK( in.get_pan_r() == 1.0f );
            CK( chan.send_gain(0) == 0.0f );
            CK( chan.send_gain(1) == 0.0f );
            CK( chan.send_gain(2) == 0.0f );
            CK( chan.send_gain(3) == 0.0f );
            lay = in.get_layer(0);
            CK( lay );
            CK( lay->get_min_velocity() == 0.0f );
            CK( lay->get_max_velocity() == 1.0f );
            CK( lay->get_gain() == 1.0 );
            CK( lay->get_pitch() == 0.0 );
            samp = lay->get_sample();
            CK( samp );
            CK( samp->get_filename().endsWith( "emptySample.flac" ) );
            CK( samp->get_sample_rate() == 44100U );
            CK( in.get_layer(1) == 0 );
            break;
        case 32:
            // Should not be 33 insts.
            BOOST_ERROR("Too many instruments loaded");
            break;
        }
    }

    /********************************************
     * Pattern Sequence
     ********************************************
     */
    T<Song::pattern_group_t>::shared_ptr seq;
    seq = s->get_pattern_group_vector();
    CK( seq->size() == 8 );
    for( k=0; unsigned(k)<seq->size() ; ++k ) {
	BOOST_REQUIRE( (*seq)[k] );
	BOOST_REQUIRE( (*seq)[k]->get_size() == 1 );
	BOOST_REQUIRE( (*seq)[k]->get(0) );
	switch(k) {
	case 0:
	case 1:
	case 2:
	case 5:
	case 6:
	    // patternID == 1
	    CK( (*seq)[k]->get(0)->get_name() == "1" );
	    break;
	case 3:
	case 7:
	    // patternID == 2
	    CK( (*seq)[k]->get(0)->get_name() == "2" );
	    break;
	case 4:
	    // patternID == 3
	    CK( (*seq)[k]->get(0)->get_name() == "3" );
	    break;
	default:
	    BOOST_ERROR("Invalid song sequence.");
	}
    }

}

TEST_CASE( 020_load_pattern_check_pattern )
{
    SyncBundle bdl;

    // Set up some fake instruments
    T<InstrumentList>::auto_ptr inst_list( new InstrumentList );
    T<Sampler>::shared_ptr sampler = engine->get_sampler();
    sampler->get_instrument_list()->clear();
    int k;
    for( k=0 ; k<32 ; ++k ) {
	T<Instrument>::shared_ptr i(
	    new Instrument(
		QString::number(k),
		QString::number(k),
		new ADSR
		)
	    );
	sampler->add_instrument(i);
    }

    s->load_uri(pattern_file_name, bdl, engine.get());

    while( ! bdl.done ) {
        sleep(1);
    }

    BOOST_REQUIRE( ! bdl.error );

    // Sort out all the components:
    std::deque< T<Song>::shared_ptr > songs;
    std::deque< T<Pattern>::shared_ptr > patterns;
    std::deque< T<Instrument>::shared_ptr > instruments;
    std::deque< T<LadspaFX>::shared_ptr > effects;

    while( ! bdl.empty() ) {
        switch(bdl.peek_type()) {
        case ObjectItem::Song_t:
            songs.push_back( bdl.pop<Song>() );
            break;
        case ObjectItem::Pattern_t:
            patterns.push_back( bdl.pop<Pattern>() );
            break;
        case ObjectItem::Instrument_t:
            instruments.push_back( bdl.pop<Instrument>() );
            break;
        case ObjectItem::LadspaFX_t:
            effects.push_back( bdl.pop<LadspaFX>() );
            break;
        default:
            BOOST_REQUIRE(false); // should not reach this.
        }
    }

    CK( songs.size() == 0 );
    CK( patterns.size() == 1 );
    CK( instruments.size() == 0 );
    CK( effects.size() == 0 );

    /********************************************
     * Pattern Object
     ********************************************
     */
    T<Pattern>::shared_ptr pat = patterns.front();

    CK(pat->get_length() == 192);
    CK(pat->get_name() == "floor-tom");
    CK(pat->get_category() == "" );

    // Not going to validate the entire file...
    // just a few specific beats @ 0, 72, 84,
    // 168, and 192.
    typedef Pattern::note_map_t::iterator iter_t;
    std::pair<iter_t, iter_t> range;
    iter_t it;
    int ctr;
    T<InstrumentList>::shared_ptr i_list = engine->get_sampler()->get_instrument_list();

    range = pat->note_map.equal_range(0);
    ctr = 0;
    for(it=range.first ; it!=range.second ; ++it) {
	++ctr;
	Note *N = it->second;
	T<Instrument>::shared_ptr inst = N->get_instrument();
	if(inst == i_list->get(0)) {
	    CK(N->get_velocity() == 0.8f);
	    CK(N->get_leadlag() == 0.0f);
	    CK(N->get_pan_l() == 0.5f);
	    CK(N->get_pan_r() == 0.5f);
	    CK(N->get_pitch() == 0.0f);
	    CK(Note::keyToString(N->m_noteKey) == "C0");
	    CK(N->get_length() == -1);
	} else if (inst == i_list->get(5)) {
	    CK(N->get_velocity() == 0.71f);
	    CK(N->get_leadlag() == 0.0f);
	    CK(N->get_pan_l() == 0.5f);
	    CK(N->get_pan_r() == 0.5f);
	    CK(N->get_pitch() == 0.0f);
	    CK(Note::keyToString(N->m_noteKey) == "C0");
	    CK(N->get_length() == -1);
	} else if (inst == i_list->get(13)) {
	    CK(N->get_velocity() == 0.8f);
	    CK(N->get_leadlag() == 0.0f);
	    CK(N->get_pan_l() == 0.5f);
	    CK(N->get_pan_r() == 0.5f);
	    CK(N->get_pitch() == 0.0f);
	    CK(Note::keyToString(N->m_noteKey) == "C0");
	    CK(N->get_length() == -1);
	} else {
	    BOOST_ERROR("Pattern references invalid instrument");
	}
    }
    CK(ctr == 3);

    range = pat->note_map.equal_range(72);
    ctr = 0;
    for(it=range.first ; it!=range.second ; ++it) {
	++ctr;
	Note *N = it->second;
	T<Instrument>::shared_ptr inst = N->get_instrument();
	if(inst == i_list->get(5)) {
	    CK(N->get_velocity() == 0.3f);
	    CK(N->get_leadlag() == 0.0f);
	    CK(N->get_pan_l() == 0.5f);
	    CK(N->get_pan_r() == 0.5f);
	    CK(N->get_pitch() == 0.0f);
	    CK(Note::keyToString(N->m_noteKey) == "C0");
	    CK(N->get_length() == -1);
	} else if (inst == i_list->get(8)) {
	    CK(N->get_velocity() == 0.46f);
	    CK(N->get_leadlag() == 0.0f);
	    CK(N->get_pan_l() == 0.5f);
	    CK(N->get_pan_r() == 0.5f);
	    CK(N->get_pitch() == 0.0f);
	    CK(Note::keyToString(N->m_noteKey) == "C0");
	    CK(N->get_length() == -1);
	} else {
	    BOOST_ERROR("Pattern references invalid instrument");
	}
    }
    CK(ctr == 2);

    range = pat->note_map.equal_range(84);
    ctr = 0;
    for(it=range.first ; it!=range.second ; ++it) {
	++ctr;
	Note *N = it->second;
	T<Instrument>::shared_ptr inst = N->get_instrument();
	if(inst == i_list->get(7)) {
	    CK(N->get_velocity() == 0.5f);
	    CK(N->get_leadlag() == 0.0f);
	    CK(N->get_pan_l() == 0.5f);
	    CK(N->get_pan_r() == 0.5f);
	    CK(N->get_pitch() == 0.0f);
	    CK(Note::keyToString(N->m_noteKey) == "C0");
	    CK(N->get_length() == -1);
	} else {
	    BOOST_ERROR("Pattern references invalid instrument");
	}
    }
    CK(ctr == 1);

    range = pat->note_map.equal_range(168);
    ctr = 0;
    for(it=range.first ; it!=range.second ; ++it) {
	++ctr;
	Note *N = it->second;
	T<Instrument>::shared_ptr inst = N->get_instrument();
	if(inst == i_list->get(5)) {
	    CK(N->get_velocity() == 0.8f);
	    CK(N->get_leadlag() == 0.0f);
	    CK(N->get_pan_l() == 0.5f);
	    CK(N->get_pan_r() == 0.5f);
	    CK(N->get_pitch() == 0.0f);
	    CK(Note::keyToString(N->m_noteKey) == "C0");
	    CK(N->get_length() == -1);
	} else if (inst == i_list->get(8)) {
	    CK(N->get_velocity() == 0.54f);
	    CK(N->get_leadlag() == 0.0f);
	    CK(N->get_pan_l() == 0.5f);
	    CK(N->get_pan_r() == 0.5f);
	    CK(N->get_pitch() == 0.0f);
	    CK(Note::keyToString(N->m_noteKey) == "C0");
	    CK(N->get_length() == -1);
	} else {
	    BOOST_ERROR("Pattern references invalid instrument");
	}
    }
    CK(ctr == 2);

    // There should be nothing @ 192.
    range = pat->note_map.equal_range(192);
    CK(range.first == range.second);

}

TEST_CASE( 030_load_drumkit_check_drumkit )
{
    SyncBundle bdl;

    s->load_uri(drumkit_manifest_file_name, bdl, engine.get());

    while( ! bdl.done ) {
        sleep(1);
    }

    BOOST_REQUIRE( ! bdl.error );

    // Sort out all the components:
    std::deque< T<Song>::shared_ptr > songs;
    std::deque< T<Pattern>::shared_ptr > patterns;
    std::deque< T<Instrument>::shared_ptr > instruments;
    std::deque< T<Mixer::Channel>::shared_ptr > channels;
    std::deque< T<LadspaFX>::shared_ptr > effects;
    std::deque< T<Drumkit>::shared_ptr > drumkits;

    while( ! bdl.empty() ) {
        switch(bdl.peek_type()) {
        case ObjectItem::Song_t:
            songs.push_back( bdl.pop<Song>() );
            break;
        case ObjectItem::Pattern_t:
            patterns.push_back( bdl.pop<Pattern>() );
            break;
        case ObjectItem::Instrument_t:
            instruments.push_back( bdl.pop<Instrument>() );
            break;
        case ObjectItem::LadspaFX_t:
            effects.push_back( bdl.pop<LadspaFX>() );
            break;
	case ObjectItem::Drumkit_t:
	    drumkits.push_back( bdl.pop<Drumkit>() );
	    break;
	case ObjectItem::Channel_t:
	    channels.push_back( bdl.pop<Mixer::Channel>() );
	    break;
        default:
            BOOST_REQUIRE(false); // should not reach this.
        }
    }

    CK( songs.size() == 0 );
    CK( patterns.size() == 0 );
    CK( instruments.size() == 32 );
    CK( channels.size() == 32 );
    CK( effects.size() == 0 );
    CK( drumkits.size() == 1 );

    /********************************************
     * Check drumkit metadata
     ********************************************
     */
    T<Drumkit>::shared_ptr dk;
    dk = drumkits.front();
    CK( dk->getName() == "test_kit" );
    CK( dk->getAuthor() == "Artemio <artemio@artemio.net>" );
    CK( dk->getInfo() == "GeneralMIDI acoustic drum set made of samples from "
	"Roland XV-5080 synth module. Thanks to L.-E. Johansson for samples. "
	"Based on the stock Hydrogen/Composite GMkit, used for unit testing." );
    CK( dk->getLicense() == "" );

    /********************************************
     * Check instruments
     ********************************************
     */

    // Checking 0, 7, 13, 15
    BOOST_REQUIRE( instruments.size() == 32 );

    T<Instrument>::shared_ptr inst;
    T<Mixer::Channel>::shared_ptr chan;
    InstrumentLayer *layer;
    
    inst = instruments[0];
    chan = channels[0];
    CK( inst->get_id() == "0" );
    CK( inst->get_name() == "Kick" );
    CK( chan->gain() == 1.0f );
    CK( inst->is_muted() == false );
    CK( inst->get_pan_l() == 1.0f );
    CK( inst->get_pan_r() == 1.0f );
    CK( inst->get_mute_group() == -1 );
    layer = inst->get_layer(0);
    BOOST_REQUIRE( layer != 0 );
    CK( layer->get_sample()->get_filename().endsWith("kick_Dry_b.flac") );
    CK( inst->get_layer(1) == 0 );

    inst = instruments[7];
    chan = channels[7];
    CK( inst->get_id() == "7" );
    CK( inst->get_name() == "Tom Mid" );
    CK( chan->gain() == 1.0f );
    CK( inst->is_muted() == false );
    CK( inst->get_pan_l() == 0.8f );
    CK( inst->get_pan_r() == 1.0f );
    CK( inst->get_mute_group() == -1 );

    inst = instruments[13];
    chan = channels[13];
    CK( inst->get_id() == "13" );
    CK( inst->get_name() == "Crash" );
    CK( chan->gain() == 0.69f );
    CK( inst->is_muted() == false );
    CK( inst->get_pan_l() == 1.0f );
    CK( inst->get_pan_r() == 0.88f );
    CK( inst->get_mute_group() == -1 );

    inst = instruments[15];
    chan = channels[15];
    CK( inst->get_id() == "15" );
    CK( inst->get_name() == "Crash Jazz" );
    CK( chan->gain() == 0.77f );
    CK( inst->is_muted() == false );
    CK( inst->get_pan_l() == 1.0f );
    CK( inst->get_pan_r() == 0.78f );
    CK( inst->get_mute_group() == -1 );

}

TEST_CASE( 040_save_song )
{
    T<Song>::shared_ptr song = Song::get_default_song(engine.get());
    SyncSaveReport ssr;
    QString save_name = QString("%1/%2").arg(temp_dir).arg("test_song.h2song");

    engine->get_mixer()->gain( song->get_volume() );
    s->save_song(save_name, song, ssr, engine.get(), false);

    while(!ssr.done) {
	sleep(1);
    }

    if(ssr.status != SaveReport::SaveSuccess) {
	BOOST_ERROR(ssr.message.toStdString());
    }

    BOOST_REQUIRE(ssr.status == SaveReport::SaveSuccess);

    song.reset();

    SyncBundle bdl;
    s->load_uri(save_name, bdl, engine.get());

    while( !bdl.done ) {
	sleep(1);
    }

    if(bdl.error) {
	BOOST_ERROR(bdl.error_message.toStdString());
    }

    BOOST_REQUIRE( ! bdl.error );

    /**
     * There is a limitation in boost unit test.  Whenver we make a
     * call to system(), the execution monitor will segfault if the
     * return value is non-zero because we receive a SIGCHLD.  This
     * can be disabled by setting the environment variable
     * BOOST_TEST_CATCH_SYSTEM_ERRORS to "no" before running the test.
     *
     * The library currently plans to add BOOST_TEST_IGNORE_SIGCHLD
     * environment variable.  I know of no programmatic solutions.  I
     * tried setting the environment variable on-the-fly... but it has
     * to be avail. for the start-up code.
     *
     * References:
     * - Boost Test documentation on the Program Execution Monitor
     * - http://lists.boost.org/boost-users/2008/09/40603.php
     * - http://lists.boost.org/boost-users/2009/05/48043.php
     */

    #warning "TODO If this test fails, Boost might segfault."
    QString check_cmd = QString("diff -w \"%1\" \"%2\"")
	.arg(song_master_default_name)
	.arg(save_name);
    int rv = system(check_cmd.toLocal8Bit());
    CK(rv == 0);
}

TEST_CASE( 050_save_pattern )
{
    // Will load an existing pattern... and then save it.

    SyncBundle bdl;

    // Set up some fake instruments
    T<InstrumentList>::auto_ptr inst_list( new InstrumentList );
    T<Sampler>::shared_ptr sampler = engine->get_sampler();
    int k;
    for( k=0 ; k<32 ; ++k ) {
	T<Instrument>::shared_ptr i(
	    new Instrument(
		QString::number(k),
		QString::number(k),
		new ADSR
		)
	    );
	sampler->add_instrument(i);
    }

    s->load_uri(pattern_file_name, bdl, engine.get());

    while( ! bdl.done ) {
        sleep(1);
    }

    BOOST_REQUIRE( ! bdl.error );

    // Sort out all the components:
    std::deque< T<Song>::shared_ptr > songs;
    std::deque< T<Pattern>::shared_ptr > patterns;
    std::deque< T<Instrument>::shared_ptr > instruments;
    std::deque< T<Mixer::Channel>::shared_ptr > channels;
    std::deque< T<LadspaFX>::shared_ptr > effects;

    while( ! bdl.empty() ) {
        switch(bdl.peek_type()) {
        case ObjectItem::Song_t:
            songs.push_back( bdl.pop<Song>() );
            break;
        case ObjectItem::Pattern_t:
            patterns.push_back( bdl.pop<Pattern>() );
            break;
        case ObjectItem::Instrument_t:
            instruments.push_back( bdl.pop<Instrument>() );
            break;
        case ObjectItem::LadspaFX_t:
            effects.push_back( bdl.pop<LadspaFX>() );
            break;
	case ObjectItem::Channel_t:
	    channels.push_back( bdl.pop<Mixer::Channel>() );
	    break;
        default:
            BOOST_REQUIRE(false); // should not reach this.
        }
    }

    CK( songs.size() == 0 );
    CK( patterns.size() == 1 );
    CK( instruments.size() == 0 );
    CK( channels.size() == 0 );
    CK( effects.size() == 0 );

    SyncSaveReport ssr;
    QString save_pattern_file_name = QString("%1/pat.h2pattern")
	.arg(temp_dir);
    s->save_pattern( save_pattern_file_name,
		     patterns.front(),
		     "GMkit",
		     ssr,
		     engine.get(),
		     false );

    while( ! ssr.done ) {
	sleep(1);
    }

    BOOST_REQUIRE(ssr.status == SaveReport::SaveSuccess);

    /**
     * There is a limitation in boost unit test.  Whenver we make a
     * call to system(), the execution monitor will segfault if the
     * return value is non-zero because we receive a SIGCHLD.  This
     * can be disabled by setting the environment variable
     * BOOST_TEST_CATCH_SYSTEM_ERRORS to "no" before running the test.
     *
     * The library currently plans to add BOOST_TEST_IGNORE_SIGCHLD
     * environment variable.  I know of no programmatic solutions.  I
     * tried setting the environment variable on-the-fly... but it has
     * to be avail. for the start-up code.
     *
     * References:
     * - Boost Test documentation on the Program Execution Monitor
     * - http://lists.boost.org/boost-users/2008/09/40603.php
     * - http://lists.boost.org/boost-users/2009/05/48043.php
     */

    #warning "TODO If this test fails, Boost might segfault."
    QString check_cmd = QString("diff -w \"%1\" \"%2\"")
	.arg(pattern_file_name)
	.arg(save_pattern_file_name);
    int rv = system(check_cmd.toLocal8Bit());
    CK(rv == 0);
    ::remove(save_pattern_file_name.toLocal8Bit());
}

TEST_CASE( 060_save_drumkit )
{
    SyncBundle bdl;

    s->load_uri(drumkit_manifest_file_name, bdl, engine.get());

    while( ! bdl.done ) {
        sleep(1);
    }

    BOOST_REQUIRE( ! bdl.error );

    // Sort out all the components:
    std::deque< T<Song>::shared_ptr > songs;
    std::deque< T<Pattern>::shared_ptr > patterns;
    std::deque< T<Instrument>::shared_ptr > instruments;
    std::deque< T<Mixer::Channel>::shared_ptr > channels;
    std::deque< T<LadspaFX>::shared_ptr > effects;
    std::deque< T<Drumkit>::shared_ptr > drumkits;

    while( ! bdl.empty() ) {
        switch(bdl.peek_type()) {
        case ObjectItem::Song_t:
            songs.push_back( bdl.pop<Song>() );
            break;
        case ObjectItem::Pattern_t:
            patterns.push_back( bdl.pop<Pattern>() );
            break;
        case ObjectItem::Instrument_t:
            instruments.push_back( bdl.pop<Instrument>() );
            break;
        case ObjectItem::LadspaFX_t:
            effects.push_back( bdl.pop<LadspaFX>() );
            break;
	case ObjectItem::Drumkit_t:
	    drumkits.push_back( bdl.pop<Drumkit>() );
	    break;
	case ObjectItem::Channel_t:
	    channels.push_back( bdl.pop<Mixer::Channel>() );
	    break;
        default:
            BOOST_REQUIRE(false); // should not reach this.
        }
    }

    CK( songs.size() == 0 );
    CK( patterns.size() == 0 );
    CK( instruments.size() == 32 );
    CK( channels.size() == 32 );
    CK( effects.size() == 0 );
    CK( drumkits.size() == 1 );

    std::deque< T<Instrument>::shared_ptr >::iterator it;
    T<InstrumentList>::shared_ptr instrument_list(new InstrumentList);
    for( it = instruments.begin() ; it != instruments.end() ; ++it ) {
	instrument_list->add( *it );
    }
    drumkits.front()->setInstrumentList( instrument_list );
    drumkits.front()->channels().clear();
    drumkits.front()->channels().insert( drumkits.front()->channels().end(),
					 channels.begin(),
					 channels.end()
	);

    SyncSaveReport ssr;
    QString save_drumkit_folder_name = QString("%1/test_kit")
	.arg(temp_dir);
    s->save_drumkit( save_drumkit_folder_name,
		     drumkits.front(),
		     ssr,
		     engine.get(),
		     false );

    while( ! ssr.done ) {
	sleep(1);
    }

    BOOST_REQUIRE(ssr.status == SaveReport::SaveSuccess);

    BOOST_REQUIRE( bdl.empty() );
    bdl.done = false;

    s->load_uri( save_drumkit_folder_name + "/drumkit.xml",
		  bdl,
		  engine.get() );

    while( ! bdl.done ) {
        sleep(1);
    }

    if( bdl.error ) {
	BOOST_ERROR( bdl.error_message.toStdString() );
    }
    BOOST_REQUIRE( ! bdl.error );

    // Sort out all the components:
    songs.clear();
    patterns.clear();
    instruments.clear();
    channels.clear();
    effects.clear();
    drumkits.clear();

    while( ! bdl.empty() ) {
        switch(bdl.peek_type()) {
        case ObjectItem::Song_t:
            songs.push_back( bdl.pop<Song>() );
            break;
        case ObjectItem::Pattern_t:
            patterns.push_back( bdl.pop<Pattern>() );
            break;
        case ObjectItem::Instrument_t:
            instruments.push_back( bdl.pop<Instrument>() );
            break;
        case ObjectItem::LadspaFX_t:
            effects.push_back( bdl.pop<LadspaFX>() );
            break;
	case ObjectItem::Drumkit_t:
	    drumkits.push_back( bdl.pop<Drumkit>() );
	    break;
	case ObjectItem::Channel_t:
	    channels.push_back( bdl.pop<Mixer::Channel>() );
	    break;
        default:
            BOOST_REQUIRE(false); // should not reach this.
        }
    }

    CK( songs.size() == 0 );
    CK( patterns.size() == 0 );
    CK( instruments.size() == 32 );
    CK( channels.size() == 32 );
    CK( effects.size() == 0 );
    CK( drumkits.size() == 1 );

    /********************************************
     * Check drumkit metadata
     ********************************************
     */
    T<Drumkit>::shared_ptr dk;
    dk = drumkits.front();
    CK( dk->getName() == "test_kit" );
    CK( dk->getAuthor() == "Artemio <artemio@artemio.net>" );
    CK( dk->getInfo() == "GeneralMIDI acoustic drum set made of samples from "
	"Roland XV-5080 synth module. Thanks to L.-E. Johansson for samples. "
	"Based on the stock Hydrogen/Composite GMkit, used for unit testing." );
    CK( dk->getLicense() == "" );

    /********************************************
     * Check instruments
     ********************************************
     */

    // Checking 0, 7, 13, 15
    BOOST_REQUIRE( instruments.size() == 32 );

    T<Instrument>::shared_ptr inst;
    T<Mixer::Channel>::shared_ptr chan;
    InstrumentLayer *layer;

    inst = instruments[0];
    chan = channels[0];
    CK( inst->get_id() == "0" );
    CK( inst->get_name() == "Kick" );
    CK( chan->gain() == 1.0f );
    CK( inst->is_muted() == false );
    CK( inst->get_pan_l() == 1.0f );
    CK( inst->get_pan_r() == 1.0f );
    CK( inst->get_mute_group() == -1 );
    layer = inst->get_layer(0);
    BOOST_REQUIRE( layer != 0 );
    CK( layer->get_sample()->get_filename().endsWith("kick_Dry_b.flac") );
    CK( inst->get_layer(1) == 0 );

    inst = instruments[7];
    chan = channels[7];
    CK( inst->get_id() == "7" );
    CK( inst->get_name() == "Tom Mid" );
    CK( chan->gain() == 1.0f );
    CK( inst->is_muted() == false );
    CK( inst->get_pan_l() == 0.8f );
    CK( inst->get_pan_r() == 1.0f );
    CK( inst->get_mute_group() == -1 );

    inst = instruments[13];
    chan = channels[13];
    CK( inst->get_id() == "13" );
    CK( inst->get_name() == "Crash" );
    CK( chan->gain() == 0.69f );
    CK( inst->is_muted() == false );
    CK( inst->get_pan_l() == 1.0f );
    CK( inst->get_pan_r() == 0.88f );
    CK( inst->get_mute_group() == -1 );

    inst = instruments[15];
    chan = channels[15];
    CK( inst->get_id() == "15" );
    CK( inst->get_name() == "Crash Jazz" );
    CK( chan->gain() == 0.77f );
    CK( inst->is_muted() == false );
    CK( inst->get_pan_l() == 1.0f );
    CK( inst->get_pan_r() == 0.78f );
    CK( inst->get_mute_group() == -1 );
}

TEST_END()
