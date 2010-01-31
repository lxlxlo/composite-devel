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
#include <Tritium/SoundLibrary.hpp>
#include <Tritium/fx/Effects.hpp>
#include <Tritium/memory.hpp>
#include <Tritium/Preferences.hpp>
#include "version.h"

#include <unistd.h> // usleep()

#include <QFile>
#include <QtXml>
#include <QFileInfo>
#include <QDir>
#include <cassert>

using namespace Tritium;
using namespace Tritium::Serialization;
using std::deque;

/*********************************************************************
 * Serializer implementation
 *********************************************************************
 */

Serializer* Serializer::create_standalone(Engine* engine)
{
    return new SerializerStandalone(engine);
}

/*********************************************************************
 * SerializerImpl implementation
 *********************************************************************
 */

SerializerImpl::SerializerImpl(Engine* engine) :
    m_queue( new SerializationQueue(engine) )
{
}

SerializerImpl::~SerializerImpl()
{
}

void SerializerImpl::load_file(const QString& filename,
                               ObjectBundle& report_to,
                               Engine* engine)
{
    m_queue->load_file(filename, report_to, engine);
}

void SerializerImpl::save_song(const QString& filename,
                               T<Song>::shared_ptr song,
                               SaveReport& report_t,
                               Engine* engine,
                               bool overwrite)
{
    m_queue->save_song(filename, song, report_t, engine, overwrite);
}

void SerializerImpl::save_drumkit(const QString& dirname,
                                  T<Drumkit>::shared_ptr dk,
                                  SaveReport& report_to,
                                  Engine* engine,
                                  bool overwrite)
{
    m_queue->save_drumkit(dirname, dk, report_to, engine, overwrite);
}

void SerializerImpl::save_pattern(const QString& filename,
                                  T<Pattern>::shared_ptr pattern,
				  const QString& drumkit_name,
                                  SaveReport& report_to,
                                  Engine* engine,
                                  bool overwrite)
{
    m_queue->save_pattern(filename, pattern, drumkit_name, report_to, engine, overwrite);
}

/*********************************************************************
 * SerializerStandalone implementation
 *********************************************************************
 */

SerializerStandalone::SerializerStandalone(Engine* engine) :
    SerializerImpl(engine)
{
    m_thread.add_client(m_queue);
    m_thread.start();
}

SerializerStandalone::~SerializerStandalone()
{
    m_thread.shutdown();
    m_thread.wait();
}

/*********************************************************************
 * SerializationQueue implementation
 *********************************************************************
 */

SerializationQueue::SerializationQueue(Engine* engine) :
    m_kill(false),
    m_engine(engine)
{
}

SerializationQueue::~SerializationQueue()
{
    m_engine = 0;
}

bool SerializationQueue::events_waiting()
{
    return ! m_queue.empty();
}

void SerializationQueue::shutdown()
{
    m_kill = true;
}

void SerializationQueue::load_file(const QString& filename,
                                   ObjectBundle& report_to,
                                   Engine *engine)
{
    event_data_t event;
    event.ev = LoadFile;
    event.filename = filename;
    event.report_load_to = &report_to;
    event.engine = engine;
    event.overwrite = false;
    m_queue.push_back(event);
}

void SerializationQueue::save_song(const QString& filename,
                                   T<Song>::shared_ptr song,
                                   SaveReport& report_t,
                                   Engine *engine,
                                   bool overwrite)
{
    event_data_t event;
    event.ev = SaveSong;
    event.filename = filename;
    event.report_save_to = &report_t;
    event.engine = engine;
    event.song = song;
    event.overwrite = overwrite;
    m_queue.push_back(event);
}

void SerializationQueue::save_drumkit(const QString& filename,
                                      T<Drumkit>::shared_ptr dk,
                                      SaveReport& report_t,
                                      Engine *engine,
                                      bool overwrite)
{
    event_data_t event;
    event.ev = SaveDrumkit;
    event.filename = filename;
    event.report_save_to = &report_t;
    event.engine = engine;
    event.drumkit = dk;
    event.overwrite = overwrite;
    m_queue.push_back(event);
}

void SerializationQueue::save_pattern(const QString& filename,
                                      T<Pattern>::shared_ptr pattern,
				      const QString& drumkit_name,
                                      SaveReport& report_t,
                                      Engine *engine,
                                      bool overwrite)
{
    event_data_t event;
    event.ev = SavePattern;
    event.filename = filename;
    event.drumkit_name = drumkit_name;
    event.report_save_to = &report_t;
    event.engine = engine;
    event.pattern = pattern;
    event.overwrite = overwrite;
    m_queue.push_back(event);
}

int SerializationQueue::process()
{
    queue_t::iterator it;

    it = m_queue.begin();
    while( it != m_queue.end() && !m_kill ) {
        switch(it->ev) {
        case LoadFile:
            handle_load_file(*it);
            break;
        case SaveSong:
            handle_save_song(*it);
            break;
        case SaveDrumkit:
            handle_save_drumkit(*it);
            break;
        case SavePattern:
            handle_save_pattern(*it);
            break;
        }
        ++it;
        m_queue.pop_front();
    }
    return 0;
}

void SerializationQueue::handle_load_file(SerializationQueue::event_data_t& ev)
{
    QFile file(ev.filename);
    if( QFile(ev.filename).exists()) {
        if(ev.filename.endsWith(".h2song")) {
            handle_load_song(ev);
        } else if (ev.filename.endsWith(".h2pattern")) {
            handle_load_pattern(ev);
        } else if (ev.filename.endsWith("drumkit.xml")) {
            handle_load_drumkit(ev);
        } else {
            ObjectBundle& bdl = *ev.report_load_to;
            bdl.error = true;
            bdl.error_message = QString("File '%1' is not in a valid format")
                .arg(ev.filename);
            bdl();
        }
    } else {
        ObjectBundle& bdl = *ev.report_load_to;
        bdl.error = true;
        bdl.error_message = QString("File '%1' does not exist")
            .arg(ev.filename);
        bdl();
    }
}

void SerializationQueue::handle_save_song(SerializationQueue::event_data_t& ev)
{
    assert(false);
}

/**
 * Saves a drumkit to a folder.
 *
 * ev.filename should point to a specific folder where the drumkit
 * should be saved.  It should \em not point to the drumkit.xml
 * manifest.  It \should point to the exact directory to store the kit
 * (i.e. the last part of the path is typically the name of the
 * drumkit).  If the folder does not exist, it will be created.
 */
void SerializationQueue::handle_save_drumkit(SerializationQueue::event_data_t& ev)
{
    T<Drumkit>::shared_ptr drumkit = ev.drumkit;

    if( Logger::get_log_level() & Logger::Info ) {
	drumkit->dump();
    }

    QVector<QString> tempVector(16);

    QString sDrumkitDir = ev.filename;

    // check if the directory exists
    QDir dir( sDrumkitDir );
    if( !dir.exists() ) {
	dir.mkpath( "." );
    } else {
	WARNINGLOG("Saving drumkit on top of an older one");
	// We don't clean out the directory, in case we accidentally
	// delete some old, valuable sample.
    }

    if( !dir.exists() ) {
	ev.report_save_to->status = SaveReport::SaveFailed;
	ev.report_save_to->message = QString("Could create folder '%1'")
	    .arg( sDrumkitDir );
	(*ev.report_save_to)();
    }

    // create the drumkit.xml file
    QString sDrumkitXmlFilename = sDrumkitDir + QString( "/drumkit.xml" );

    QDomDocument doc;
    QDomProcessingInstruction header = doc.createProcessingInstruction( "xml", "version=\"1.0\" encoding=\"UTF-8\"");
    doc.appendChild( header );

    QDomElement rootNode = doc.createElement( "drumkit_info" );

    LocalFileMng::writeXmlString( rootNode, "name", drumkit->getName() );    // name
    LocalFileMng::writeXmlString( rootNode, "author", drumkit->getAuthor() );        // author
    LocalFileMng::writeXmlString( rootNode, "info", drumkit->getInfo() );    // info
    LocalFileMng::writeXmlString( rootNode, "license", drumkit->getLicense() );      // license

    //QDomNode instrumentListNode( "instrumentList" );              // instrument list
    QDomElement instrumentListNode = doc.createElement( "instrumentList" );

    unsigned nInstrument = drumkit->getInstrumentList()->get_size();
    // INSTRUMENT NODE
    for ( unsigned i = 0; i < nInstrument; i++ ) {
	T<Instrument>::shared_ptr instr = drumkit->getInstrumentList()->get( i );

	for ( unsigned nLayer = 0; nLayer < MAX_LAYERS; nLayer++ ) {
	    InstrumentLayer *pLayer = instr->get_layer( nLayer );
	    if ( pLayer ) {
		T<Sample>::shared_ptr pSample = pLayer->get_sample();
		QString sOrigFilename = pSample->get_filename();

		QString sDestFilename = sOrigFilename;

		/*
		  Till rev. 743, the samples got copied into the
		  root of the drumkit folder.

		  Now the sample gets only copied to the folder
		  if it doesn't reside in a subfolder of the drumkit dir.
		*/

		if( sOrigFilename.startsWith( sDrumkitDir ) ){
		    INFOLOG("sample is already in drumkit dir");
		    tempVector[ nLayer ] = sDestFilename.remove( sDrumkitDir + "/" );
		} else {
		    int nPos = sDestFilename.lastIndexOf( '/' );
		    sDestFilename = sDestFilename.mid( nPos + 1, sDestFilename.size() - nPos - 1 );
		    sDestFilename = sDrumkitDir + "/" + sDestFilename;

		    QFile::copy( sOrigFilename, sDestFilename );
		    tempVector[ nLayer ] = sDestFilename.remove( sDrumkitDir + "/" );
		}
	    }
	}

	QDomNode instrumentNode = doc.createElement( "instrument" );

	LocalFileMng::writeXmlString( instrumentNode, "id", instr->get_id() );
	LocalFileMng::writeXmlString( instrumentNode, "name", instr->get_name() );
	LocalFileMng::writeXmlString( instrumentNode, "volume", QString("%1").arg( instr->get_volume() ) );
	LocalFileMng::writeXmlBool( instrumentNode, "isMuted", instr->is_muted() );
	LocalFileMng::writeXmlString( instrumentNode, "pan_L", QString("%1").arg( instr->get_pan_l() ) );
	LocalFileMng::writeXmlString( instrumentNode, "pan_R", QString("%1").arg( instr->get_pan_r() ) );
	LocalFileMng::writeXmlString( instrumentNode, "randomPitchFactor", QString("%1").arg( instr->get_random_pitch_factor() ) );
	LocalFileMng::writeXmlString( instrumentNode, "gain", QString("%1").arg( instr->get_gain() ) );

	LocalFileMng::writeXmlBool( instrumentNode, "filterActive", instr->is_filter_active() );
	LocalFileMng::writeXmlString( instrumentNode, "filterCutoff", QString("%1").arg( instr->get_filter_cutoff() ) );
	LocalFileMng::writeXmlString( instrumentNode, "filterResonance", QString("%1").arg( instr->get_filter_resonance() ) );

	LocalFileMng::writeXmlString( instrumentNode, "Attack", QString("%1").arg( instr->get_adsr()->__attack ) );
	LocalFileMng::writeXmlString( instrumentNode, "Decay", QString("%1").arg( instr->get_adsr()->__decay ) );
	LocalFileMng::writeXmlString( instrumentNode, "Sustain", QString("%1").arg( instr->get_adsr()->__sustain ) );
	LocalFileMng::writeXmlString( instrumentNode, "Release", QString("%1").arg( instr->get_adsr()->__release ) );

	LocalFileMng::writeXmlString( instrumentNode, "muteGroup", QString("%1").arg( instr->get_mute_group() ) );

	for ( unsigned nLayer = 0; nLayer < MAX_LAYERS; nLayer++ ) {
	    InstrumentLayer *pLayer = instr->get_layer( nLayer );
	    if ( pLayer == NULL ) continue;
	    // Sample *pSample = pLayer->get_sample();

	    QDomNode layerNode = doc.createElement( "layer" );
	    LocalFileMng::writeXmlString( layerNode, "filename", tempVector[ nLayer ] );
	    LocalFileMng::writeXmlString( layerNode, "min", QString("%1").arg( pLayer->get_min_velocity() ) );
	    LocalFileMng::writeXmlString( layerNode, "max", QString("%1").arg( pLayer->get_max_velocity() ) );
	    LocalFileMng::writeXmlString( layerNode, "gain", QString("%1").arg( pLayer->get_gain() ) );
	    LocalFileMng::writeXmlString( layerNode, "pitch", QString("%1").arg( pLayer->get_pitch() ) );

	    instrumentNode.appendChild( layerNode );
	}

	instrumentListNode.appendChild( instrumentNode );
    }

    rootNode.appendChild( instrumentListNode );

    doc.appendChild( rootNode );

    QFile file( sDrumkitXmlFilename );
    if ( !file.open(QIODevice::WriteOnly) ) {
	ev.report_save_to->status = SaveReport::SaveFailed;
	ev.report_save_to->message = QString("Could not open file '%1' to write")
	    .arg(sDrumkitXmlFilename);
	(*ev.report_save_to)();
    }

    QTextStream TextStream( &file );
    doc.save( TextStream, 1 );

    file.close();

    ev.report_save_to->status = SaveReport::SaveSuccess;
    (*ev.report_save_to)();
}

void SerializationQueue::handle_save_pattern(SerializationQueue::event_data_t& ev)
{
    //int mode = 1 save, int mode = 2 save as
    // INSTRUMENT NODE


    // Requirements:  Must have ev.pattern and ev.drumkit set.
    // ev.filename must be a full path.
    assert(ev.ev == SavePattern);
    assert(ev.pattern);
    T<Pattern>::shared_ptr pat = ev.pattern;
    QString drumkit_name = ev.drumkit_name;
    QString sPatternXmlFilename = ev.filename;
    if( ! sPatternXmlFilename.endsWith(".h2pattern") ) {
	sPatternXmlFilename += ".h2pattern";
    }

    // check if the directory exists
    QFileInfo pat_file_info(sPatternXmlFilename);
    QDir dir = pat_file_info.dir();
    if( !dir.exists() ) {
	dir.mkdir( dir.path() );
    }
    if( !dir.exists() ) {
	ev.report_save_to->filename = ev.filename;
	ev.report_save_to->message = QString("Could not create directory '%1' for save.")
	    .arg( dir.path() );
	ev.report_save_to->status = SaveReport::SaveFailed;
	(*ev.report_save_to)();
	return;
    }

    QDomDocument doc;
    QDomProcessingInstruction header = doc.createProcessingInstruction( "xml", "version=\"1.0\" encoding=\"UTF-8\"");
    doc.appendChild( header );

    QDomNode rootNode = doc.createElement( "drumkit_pattern" );
    //LIB_ID just in work to get better usability
    //writeXmlString( &rootNode, "LIB_ID", "in_work" );
    LocalFileMng::writeXmlString( rootNode, "pattern_for_drumkit", drumkit_name );


    // pattern
    QDomNode patternNode = doc.createElement( "pattern" );
    LocalFileMng::writeXmlString( patternNode, "pattern_name", ev.pattern->get_name() );
    LocalFileMng::writeXmlString( patternNode, "category", pat->get_category() );
    LocalFileMng::writeXmlString( patternNode, "size", QString("%1").arg( pat->get_length() ) );

    QDomNode noteListNode = doc.createElement( "noteList" );
    Pattern::note_map_t::iterator pos;
    for ( pos = pat->note_map.begin(); pos != pat->note_map.end(); ++pos ) {
	Note *pNote = pos->second;
	assert( pNote );

	QDomNode noteNode = doc.createElement( "note" );
	LocalFileMng::writeXmlString( noteNode, "position", QString("%1").arg( pos->first ) );
	LocalFileMng::writeXmlString( noteNode, "leadlag", QString("%1").arg( pNote->get_leadlag() ) );
	LocalFileMng::writeXmlString( noteNode, "velocity", QString("%1").arg( pNote->get_velocity() ) );
	LocalFileMng::writeXmlString( noteNode, "pan_L", QString("%1").arg( pNote->get_pan_l() ) );
	LocalFileMng::writeXmlString( noteNode, "pan_R", QString("%1").arg( pNote->get_pan_r() ) );
	LocalFileMng::writeXmlString( noteNode, "pitch", QString("%1").arg( pNote->get_pitch() ) );

	LocalFileMng::writeXmlString( noteNode, "key", Note::keyToString( pNote->m_noteKey ) );

	LocalFileMng::writeXmlString( noteNode, "length", QString("%1").arg( pNote->get_length() ) );
	LocalFileMng::writeXmlString( noteNode, "instrument", pNote->get_instrument()->get_id() );
	noteListNode.appendChild( noteNode );
    }
    patternNode.appendChild( noteListNode );

    rootNode.appendChild( patternNode );

    doc.appendChild( rootNode );

    QFile file( sPatternXmlFilename );
    if ( !file.open(QIODevice::WriteOnly) ) {
	ev.report_save_to->filename = ev.filename;
	ev.report_save_to->message = QString("Could not create file '%1' for save.")
	    .arg( ev.filename );
	ev.report_save_to->status = SaveReport::SaveFailed;
	(*ev.report_save_to)();
	return;
    }

    QTextStream TextStream( &file );
    doc.save( TextStream, 1 );

    file.close();


    QFileInfo check_file_written( sPatternXmlFilename );
    if ( !check_file_written.exists() ) {
	ev.report_save_to->filename = ev.filename;
	ev.report_save_to->message = QString("Could not create directory '%1' for save.")
	    .arg( dir.path() );
	ev.report_save_to->status = SaveReport::SaveFailed;
	(*ev.report_save_to)();
	return;
    }

    ev.report_save_to->filename = ev.filename;
    ev.report_save_to->message = QString();
    ev.report_save_to->status = SaveReport::SaveSuccess;
    (*ev.report_save_to)();
}

void SerializationQueue::handle_load_song(SerializationQueue::event_data_t& ev)
{
    QDomDocument song_doc = LocalFileMng::openXmlDocument(ev.filename);
    QDomElement song_node = song_doc.documentElement();
    QStringList errors;

    if( song_node.tagName() != "song" ) {
        ERRORLOG( QString("Error loading %1 -- not a valid .h2song")
                  .arg(ev.filename) );
        ev.report_load_to->error = true;
        ev.report_load_to->error_message = "Not a valid .h2song.";
        (*ev.report_load_to)();
        return;
    }
    QDomElement instrumentList_node =
        song_node.firstChildElement("instrumentList");
    if( instrumentList_node.isNull() ) {
        ERRORLOG( QString("Error loading %1 -- no instrumentList node"
                          " in .h2song")
                  .arg(ev.filename) );
        ev.report_load_to->error = true;
        ev.report_load_to->error_message =
            ".h2song missing instrumentList section.";
        (*ev.report_load_to)();
        return;
    }
    QDomElement patternList_node = song_node.firstChildElement("patternList");
    if( patternList_node.isNull() ) {
        ERRORLOG( QString("Error loading %1 -- no patternList node in .h2song")
                  .arg(ev.filename) );
        ev.report_load_to->error = true;
        ev.report_load_to->error_message =
            ".h2song missing patternList section.";
        (*ev.report_load_to)();
        return;
    }
    QDomElement patternSequence_node =
        song_node.firstChildElement("patternSequence");
    if( patternList_node.isNull() ) {
        ERRORLOG( QString("Error loading %1 -- no patternSequence node "
                          "in .h2song")
                  .arg(ev.filename) );
        ev.report_load_to->error = true;
        ev.report_load_to->error_message =
            ".h2song missing patternSequence section.";
        (*ev.report_load_to)();
        return;
    }
    // Null is OK.
    QDomElement ladspa_node = song_node.firstChildElement("ladspa");

    // LOAD SONG-SPECIFIC DATA
    T<Song>::shared_ptr song = handle_load_song_node(song_node, errors);
    song->set_filename(ev.filename);

    // LOAD INSTRUMENTS
    deque< T<Instrument>::shared_ptr > instrument_ra;
    handle_load_instrumentlist_node(instrument_ra, "-", instrumentList_node, errors);

    // LOAD PATTERNS
    deque< T<Pattern>::shared_ptr > pattern_ra;
    handle_load_patternlist_node(pattern_ra, patternList_node,
                                 instrument_ra, errors);

    // LOAD PATTERN SEQUENCE
    deque< QStringList > pattern_seq_ra;
    handle_load_patternsequence_node(pattern_seq_ra, patternSequence_node,
                                     errors);

    // LOAD LADSPA SETTINGS
    deque< T<LadspaFX>::shared_ptr > fx_ra;
    if( ! ladspa_node.isNull() ) {
        handle_load_ladspa_node(fx_ra, ladspa_node, errors);
    }

    #warning "TODO: NEED TO HANDLE ERRORS"
    #warning "TODO: TO VALIDATE OBJECTS"

    /***********************
     * DISPATCH OBJECTS
     ***********************
     */

    ObjectBundle& bdl = *ev.report_load_to;

    bdl.push(song);

    deque< T<Instrument>::shared_ptr >::iterator i_it;
    for(i_it = instrument_ra.begin() ; i_it != instrument_ra.end() ; ++i_it ) {
        bdl.push( *i_it );
    }

    T<PatternList>::auto_ptr pattern_list( new PatternList );
    deque< T<Pattern>::shared_ptr >::iterator p_it;
    for( p_it = pattern_ra.begin() ; p_it != pattern_ra.end() ; ++p_it ) {
        pattern_list->add( *p_it );
    }

    // Join the pattern sequence names with the patterns
    // that we've already loaded.  (Similar to an SQL
    // LEFT JOIN).  Then put both into the song reference.
    deque< QStringList >::iterator ps_it;
    T<Song::pattern_group_t>::shared_ptr groups(new Song::pattern_group_t);
    for( ps_it = pattern_seq_ra.begin() ; ps_it != pattern_seq_ra.end() ; ++ps_it ) {
        T<PatternList>::shared_ptr tmp( new PatternList );
        QStringList::Iterator pid_it;
        for( pid_it = ps_it->begin() ; pid_it != ps_it->end() ; ++pid_it ) {
            // Find the pattern whose name matches *pid_it
            int j;
            for(j=0 ; j < pattern_list->get_size() ; ++j) {
                #warning "TODO: Detech if there is no pattern ID match."
                T<Pattern>::shared_ptr p_tmp;
                p_tmp = pattern_list->get(j);
                if( (*pid_it) == p_tmp->get_name() ) {
                    tmp->add(p_tmp);
                    j = pattern_list->get_size();
                }
            }
        }
        groups->push_back(tmp);
    }

    song->set_pattern_list( pattern_list.release() );
    song->set_pattern_group_vector( groups );

    deque< T<LadspaFX>::shared_ptr >::iterator fx_it;
    for(fx_it = fx_ra.begin() ; fx_it != fx_ra.end() ; ++fx_it ) {
        bdl.push( *fx_it );
    }

    bdl();
}

void SerializationQueue::handle_load_drumkit(
    SerializationQueue::event_data_t& ev
    )
{
    // Path information
    QFileInfo fn_info(ev.filename);
    QString drumkit_dir = fn_info.absolutePath();

    if( ! fn_info.exists() ) {
        ERRORLOG( QString("Error loading %1 -- file not found.")
                  .arg(fn_info.absoluteFilePath()) );
        ev.report_load_to->error = true;
        ev.report_load_to->error_message = "File not found.";
        (*ev.report_load_to)();
        return;
    }

    QDomDocument drumkit_doc = LocalFileMng::openXmlDocument(ev.filename);

    if( drumkit_doc.isNull() ) {
        ERRORLOG( QString("Error loading %1 -- not an XML file.")
                  .arg(fn_info.absoluteFilePath()) );
        ev.report_load_to->error = true;
        ev.report_load_to->error_message = "Not an XML file.";
        (*ev.report_load_to)();
        return;
    }

    QDomElement drumkit_info_node = drumkit_doc.documentElement();
    QStringList errors;

    if( drumkit_info_node.tagName() != "drumkit_info" ) {
        ERRORLOG( QString("Error loading %1 -- not a valid drumkit.xml file")
                  .arg(ev.filename) );
        ev.report_load_to->error = true;
        ev.report_load_to->error_message = "Not a valid drumkit.xml file.";
        (*ev.report_load_to)();
        return;
    }
    // TODO: Do this with a handle_load_drumkit_info_node
    // instead of inlining it like this
    T<Drumkit>::shared_ptr drumkit(new Drumkit);
    QString dk_name = LocalFileMng::readXmlString(drumkit_info_node, "name", "");
    QString dk_author = LocalFileMng::readXmlString(drumkit_info_node, "author", "");
    QString dk_info = LocalFileMng::readXmlString(drumkit_info_node, "info", "");
    QString dk_license = LocalFileMng::readXmlString(drumkit_info_node, "license", "");

    drumkit->setName( dk_name );
    drumkit->setAuthor( dk_author );
    drumkit->setInfo( dk_info );
    drumkit->setLicense( dk_license );

    QDomElement instrumentList_node =
        drumkit_info_node.firstChildElement("instrumentList");
    if( instrumentList_node.isNull() ) {
        ERRORLOG( QString("Error loading %1 -- no instrumentList node "
                          "in drumkit.xml")
                  .arg(ev.filename) );
        ev.report_load_to->error = true;
        ev.report_load_to->error_message =
            "drumkit.xml missing instrumentList section.";
        (*ev.report_load_to)();
        return;
    }
    deque< T<Instrument>::shared_ptr > instrument_ra;
    handle_load_instrumentlist_node(instrument_ra,
				    drumkit_dir,
				    instrumentList_node,
				    errors);

    #warning "TODO: NEED TO HANDLE ERRORS"
    #warning "TODO: NEED TO VALIDATE OBJECTS"

    /***********************
     * DISPATCH OBJECTS
     ***********************
     */

    ObjectBundle& bdl = (*ev.report_load_to);

    bdl.push( drumkit );
    deque< T<Instrument>::shared_ptr >::iterator i_it;
    for(i_it = instrument_ra.begin() ; i_it != instrument_ra.end() ; ++i_it ) {
        bdl.push( *i_it );
    }

    bdl();
}

void SerializationQueue::handle_load_pattern(
    SerializationQueue::event_data_t& ev
    )
{
    QDomDocument pattern_doc = LocalFileMng::openXmlDocument(ev.filename);
    QDomElement dk_pattern_node = pattern_doc.documentElement();
    QStringList errors;

    if( dk_pattern_node.tagName() != "drumkit_pattern" ) {
        ERRORLOG( QString("Error loading %1 -- not a valid .h2pattern file")
                  .arg(ev.filename) );
        ev.report_load_to->error = true;
        ev.report_load_to->error_message = "Not a valid .h2pattern file.";
        (*ev.report_load_to)();
        return;
    }
    QDomElement pat_node = dk_pattern_node.firstChildElement("pattern");
    if( pat_node.isNull() ) {
        ERRORLOG( QString("Error loading %1 -- no pattern node in drumkit.xml")
                  .arg(ev.filename) );
        ev.report_load_to->error = true;
        ev.report_load_to->error_message = ".h2pattern missing pattern section.";
        (*ev.report_load_to)();
        return;
    }

    #warning "TODO: Converting InstrumentList to std::deque<>... not a great practice"
    deque< T<Instrument>::shared_ptr > insts;
    InstrumentList* ilist = ev.engine->getSong()->get_instrument_list();
    for( unsigned k=0 ; k < ilist->get_size() ; ++k ) {
        insts.push_back( ilist->get(k) );
    }

    T<Pattern>::shared_ptr pat = handle_load_pattern_node(pat_node, insts,
                                                          errors);

    #warning "TODO: Need to handle errors!!"

    ev.report_load_to->push(pat);

    (*ev.report_load_to)();
}

T<Song>::shared_ptr SerializationQueue::handle_load_song_node(
    QDomElement songNode,
    QStringList& errors
    )
{
    QString m_sSongVersion = LocalFileMng::readXmlString( songNode , "version", "Unknown version" );

    if ( m_sSongVersion != QString( get_version().c_str() ) ) {
        WARNINGLOG( "Trying to load a song created with a different "
                    "version of Hydrogen/Tritium/Composite." );
        WARNINGLOG( "Song was saved with version " + m_sSongVersion );
    }

    float fBpm = LocalFileMng::readXmlFloat( songNode, "bpm", 120 );
    float fVolume = LocalFileMng::readXmlFloat( songNode, "volume", 0.5 );
    float fMetronomeVolume = LocalFileMng::readXmlFloat( songNode, "metronomeVolume", 0.5 );
    QString sName( LocalFileMng::readXmlString( songNode, "name", "Untitled Song" ) );
    QString sAuthor( LocalFileMng::readXmlString( songNode, "author", "Unknown Author" ) );
    QString sNotes( LocalFileMng::readXmlString( songNode, "notes", "..." ) );
    QString sLicense( LocalFileMng::readXmlString( songNode, "license", "Unknown license" ) );
    bool bLoopEnabled = LocalFileMng::readXmlBool( songNode, "loopEnabled", false );

    Song::SongMode nMode = Song::PATTERN_MODE;  // Mode (song/pattern)
    QString sMode = LocalFileMng::readXmlString( songNode, "mode", "pattern" );
    if ( sMode == "song" ) {
        nMode = Song::SONG_MODE;
    }

    float fHumanizeTimeValue = LocalFileMng::readXmlFloat( songNode, "humanize_time", 0.0 );
    float fHumanizeVelocityValue = LocalFileMng::readXmlFloat( songNode, "humanize_velocity", 0.0 );
    float fSwingFactor = LocalFileMng::readXmlFloat( songNode, "swing_factor", 0.0 );

    T<Song>::shared_ptr song( new Song( sName, sAuthor, fBpm, fVolume ) );
    song->set_metronome_volume( fMetronomeVolume );
    song->set_notes( sNotes );
    song->set_license( sLicense );
    song->set_loop_enabled( bLoopEnabled );
    song->set_mode( nMode );
    song->set_humanize_time_value( fHumanizeTimeValue );
    song->set_humanize_velocity_value( fHumanizeVelocityValue );
    song->set_swing_factor( fSwingFactor );

    return song;
}

void SerializationQueue::handle_load_instrumentlist_node(
    deque< T<Instrument>::shared_ptr >& dest,
    const QString& drumkit_path,
    QDomElement& instrumentList_node,
    QStringList& errors)
{
    QDomElement inst_node;
    T<Instrument>::shared_ptr i;
    inst_node = instrumentList_node.firstChildElement("instrument");
    while( ! inst_node.isNull() ) {
        i = handle_load_instrument_node(inst_node, drumkit_path, errors);
        if(i) dest.push_back(i);
        inst_node = inst_node.nextSiblingElement("instrument");
    }
}

T<Instrument>::shared_ptr SerializationQueue::handle_load_instrument_node(
    QDomElement& instrumentNode,
    const QString& drumkit_path,
    QStringList& errors)
{
    QString sId = LocalFileMng::readXmlString( instrumentNode, "id", "" );                      // instrument id
    QString sDrumkit = LocalFileMng::readXmlString( instrumentNode, "drumkit", "" );    // drumkit
    QString sName = LocalFileMng::readXmlString( instrumentNode, "name", "" );          // name
    float fVolume = LocalFileMng::readXmlFloat( instrumentNode, "volume", 1.0 );        // volume
    bool bIsMuted = LocalFileMng::readXmlBool( instrumentNode, "isMuted", false );      // is muted
    float fPan_L = LocalFileMng::readXmlFloat( instrumentNode, "pan_L", 0.5 );  // pan L
    float fPan_R = LocalFileMng::readXmlFloat( instrumentNode, "pan_R", 0.5 );  // pan R
    float fFX1Level = LocalFileMng::readXmlFloat( instrumentNode, "FX1Level", 0.0 );    // FX level
    float fFX2Level = LocalFileMng::readXmlFloat( instrumentNode, "FX2Level", 0.0 );    // FX level
    float fFX3Level = LocalFileMng::readXmlFloat( instrumentNode, "FX3Level", 0.0 );    // FX level
    float fFX4Level = LocalFileMng::readXmlFloat( instrumentNode, "FX4Level", 0.0 );    // FX level
    float fGain = LocalFileMng::readXmlFloat( instrumentNode, "gain", 1.0, false, false );      // instrument gain

    int fAttack = LocalFileMng::readXmlInt( instrumentNode, "Attack", 0, false, false );                // Attack
    int fDecay = LocalFileMng::readXmlInt( instrumentNode, "Decay", 0, false, false );          // Decay
    float fSustain = LocalFileMng::readXmlFloat( instrumentNode, "Sustain", 1.0, false, false );        // Sustain
    int fRelease = LocalFileMng::readXmlInt( instrumentNode, "Release", 1000, false, false );   // Release

    float fRandomPitchFactor = LocalFileMng::readXmlFloat( instrumentNode, "randomPitchFactor", 0.0f, false, false );

    bool bFilterActive = LocalFileMng::readXmlBool( instrumentNode, "filterActive", false );
    float fFilterCutoff = LocalFileMng::readXmlFloat( instrumentNode, "filterCutoff", 1.0f, false );
    float fFilterResonance = LocalFileMng::readXmlFloat( instrumentNode, "filterResonance", 0.0f, false );
    QString sMuteGroup = LocalFileMng::readXmlString( instrumentNode, "muteGroup", "-1", false );
    int nMuteGroup = sMuteGroup.toInt();

    if ( sId.isEmpty() ) {
        errors << QString("Empty ID for instrument %1... skipping").arg(sName);
        return T<Instrument>::shared_ptr();
    }

    // create a new instrument
    T<Instrument>::shared_ptr pInstrument(
        new Instrument(
            sId,
            sName,
            new ADSR( fAttack, fDecay, fSustain, fRelease )
            )
        );
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

    // Load the samples (layers)
    LocalFileMng localFileMng(m_engine);
    QString drumkitPath = drumkit_path;
    if ( ( !sDrumkit.isEmpty() ) && ( sDrumkit != "-" ) ) {
        drumkitPath = localFileMng.getDrumkitDirectory( sDrumkit ) + sDrumkit;
    }

    QDomNode filenameNode = instrumentNode.firstChildElement( "filename" );

    if( !filenameNode.isNull() ) {
        // Backward compatability mode (Hydrogen <= 0.9.0)
        // Only one layer.
        QString sFilename = LocalFileMng::readXmlString( instrumentNode, "filename", "" );

        if ( !drumkitPath.isEmpty() ) {
            sFilename = drumkitPath + "/" + sFilename;
        }
        T<Sample>::shared_ptr pSample = Sample::load( sFilename );
        if ( ! pSample ) {
            // When switching between 0.8.2 and 0.9.0 the default
            // drumkit was changed.  If loading the sample fails, try
            // again by adding ".flac" to the file name.
            sFilename = sFilename.left( sFilename.length() - 4 );
            sFilename += ".flac";
            pSample = Sample::load( sFilename );
        }
        if ( ! pSample ) {
            ERRORLOG( "Error loading sample: " + sFilename + " not found" );
            pInstrument->set_muted( true );
        }
        T<InstrumentLayer>::auto_ptr pLayer( new InstrumentLayer( pSample ) );
        pInstrument->set_layer( pLayer.release(), 0 );
    } else {
        // After 0.9.0, all drumkits have at least one <layer>
        // element for loading samples.
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
            T<Sample>::shared_ptr pSample = Sample::load( sFilename );
            if ( ! pSample ) {
                ERRORLOG( "Error loading sample: " + sFilename + " not found" );
                pInstrument->set_muted( true );
            }
            T<InstrumentLayer>::auto_ptr pLayer( new InstrumentLayer( pSample ) );
            pLayer->set_velocity_range( fMin, fMax );
            pLayer->set_gain( fGain );
            pLayer->set_pitch( fPitch );
            pInstrument->set_layer( pLayer.release(), nLayer );
            nLayer++;

            layerNode = ( QDomNode ) layerNode.nextSiblingElement( "layer" );

        }
    }

    #warning "TODO: NEED TO VALIDATE INSTRUMENT BEFORE PASSING IT BACK"

    return pInstrument;
}

void SerializationQueue::handle_load_patternlist_node(
    deque< T<Pattern>::shared_ptr >& dest,
    QDomElement& patternList_node,
    const deque< T<Instrument>::shared_ptr >& insts,
    QStringList& errors)
{
    QDomElement pat_node;
    T<Pattern>::shared_ptr p;
    pat_node = patternList_node.firstChildElement("pattern");
    while( ! pat_node.isNull() ) {
        p = handle_load_pattern_node(pat_node, insts, errors);
        if(p) dest.push_back(p);
        pat_node = pat_node.nextSiblingElement("pattern");
    }
}

T<Pattern>::shared_ptr SerializationQueue::handle_load_pattern_node(
    QDomElement& pat_node,
    const deque< T<Instrument>::shared_ptr >& insts,
    QStringList& errors)
{
    // There are 3 different <pattern> schemas.  This is a
    // switch to choose the correct one.  See Documentation/Xml_Schemas.txt
    QDomNode test = pat_node.firstChildElement("noteList");
    if( test.isNull() ) {
        return handle_load_pattern_node_pre094(pat_node, insts, errors);
    } else {
        // Handles both .h2song and .h2pattern
        return handle_load_pattern_node_094(pat_node, insts, errors);
    }
}

T<Pattern>::shared_ptr SerializationQueue::handle_load_pattern_node_pre094(
    QDomElement& pat_node,
    const deque< T<Instrument>::shared_ptr >& insts,
    QStringList& errors)
{
    T<Pattern>::shared_ptr pPattern;

    QString sName;      // name
    sName = LocalFileMng::readXmlString( pat_node, "name", sName );

    QString sCategory = ""; // category
    sCategory = LocalFileMng::readXmlString( pat_node, "category", sCategory );
    int nSize = -1;
    nSize = LocalFileMng::readXmlInt( pat_node, "size", nSize, false, false );

    pPattern.reset( new Pattern( sName, sCategory, nSize ) );

    // Back compatibility code. Version < 0.9.4
    QDomNode sequenceListNode = pat_node.firstChildElement( "sequenceList" );

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

            T<Instrument>::shared_ptr instrRef;
            deque< T<Instrument>::shared_ptr >::const_iterator it;
            unsigned i;
            for( i=0, it=insts.begin() ; it != insts.end() ; ++it ) {
                if( instrId == (*it)->get_id() ) {
                    instrRef = (*it);
                    break;
                }
            }

            if ( !instrRef ) {
                ERRORLOG( "Instrument with ID: '" + instrId + "' not found. Note skipped." );
                continue;
            }

            pNote = new Note( instrRef, fVelocity, fPan_L, fPan_R, nLength, nPitch );
            pNote->set_leadlag(fLeadLag);

            //infoLog( "new note!! pos: " + toString( pNote->m_nPosition ) + "\t instr: " + instrId );
            pPattern->note_map.insert( std::make_pair( nPosition, pNote ) );

            noteNode = ( QDomNode ) noteNode.nextSiblingElement( "note" );


        }
        sequenceNode = ( QDomNode ) sequenceNode.nextSiblingElement( "sequence" );
    }

    return pPattern;
}

T<Pattern>::shared_ptr SerializationQueue::handle_load_pattern_node_094(
    QDomElement& pat_node,
    const deque< T<Instrument>::shared_ptr >& insts,
    QStringList& errors)
{
    T<Pattern>::shared_ptr pPattern;

    QDomNode test;
    QString sName;      // name
    test = pat_node.firstChildElement("name");
    if( ! test.isNull() ) {
        sName = LocalFileMng::readXmlString( pat_node, "name", sName );
    } else {
        sName = LocalFileMng::readXmlString( pat_node, "pattern_name", sName );
    }

    QString sCategory = ""; // category
    sCategory = LocalFileMng::readXmlString( pat_node, "category", sCategory );
    int nSize = -1;
    nSize = LocalFileMng::readXmlInt( pat_node, "size", nSize, false, false );

    pPattern.reset( new Pattern( sName, sCategory, nSize ) );

    QDomNode pNoteListNode = pat_node.firstChildElement( "noteList" );

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

        T<Instrument>::shared_ptr instrRef;
        deque< T<Instrument>::shared_ptr >::const_iterator it;
        unsigned i;
        for( i=0, it=insts.begin() ; it != insts.end() ; ++it ) {
            if( instrId == (*it)->get_id() ) {
                instrRef = (*it);
                break;
            }
        }

        if ( !instrRef ) {
            ERRORLOG( "Instrument with ID: '" + instrId + "' not found. Note skipped." );
        }

        pNote = new Note( instrRef, fVelocity, fPan_L, fPan_R, nLength, nPitch, Note::stringToKey( sKey ) );
        pNote->set_leadlag(fLeadLag);
        pPattern->note_map.insert( std::make_pair( nPosition, pNote ) );

        noteNode = ( QDomNode ) noteNode.nextSiblingElement( "note" );
    }

    return pPattern;
}

void SerializationQueue::handle_load_patternsequence_node(
    deque< QStringList >& pattern_seq_ra,
    QDomElement& patternSequence_node,
    QStringList& errors )
{
    QDomElement group = patternSequence_node.firstChildElement("group");
    QLocale c_locale = QLocale::c();

    while( !group.isNull() ) {
        QStringList pats;
        QDomElement pid = group.firstChildElement("patternID");
        while( !pid.isNull() ) {
            pats << pid.text();
            pid = pid.nextSiblingElement("patternID");
        }
        pattern_seq_ra.push_back(pats);
        group = group.nextSiblingElement("group");
    }
}

void SerializationQueue::handle_load_ladspa_node(
    deque< T<LadspaFX>::shared_ptr >& dest,
    QDomElement& ladspaNode,
    QStringList& errors)
{
    int nFX = 0;
    QDomElement fxNode = ladspaNode.firstChildElement( "fx" );
    T<LadspaFX>::shared_ptr fx;
    while (  !fxNode.isNull()  ) {
        fx = handle_load_fx_node(fxNode, errors);
        if(fx) dest.push_back(fx);
        fxNode = fxNode.nextSiblingElement("fx");
    }
}

T<LadspaFX>::shared_ptr SerializationQueue::handle_load_fx_node(
    QDomElement& fxNode,
    QStringList& errors)
{
    QString sName = LocalFileMng::readXmlString( fxNode, "name", "" );
    QString sFilename = LocalFileMng::readXmlString( fxNode, "filename", "" );
    bool bEnabled = LocalFileMng::readXmlBool( fxNode, "enabled", false );
    float fVolume = LocalFileMng::readXmlFloat( fxNode, "volume", 1.0 );

    T<LadspaFX>::shared_ptr pFX;

    if ( sName != "no plugin" ) {
        // FIXME: il caricamento va fatto fare all'engine, solo lui sa il samplerate esatto
#ifdef LADSPA_SUPPORT
        pFX = LadspaFX::load( sFilename, sName, 44100 );
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
        }
#endif
    }
    return pFX;
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
T<Song>::shared_ptr SongReader::readSong( Engine* engine, const QString& filename )
{
    SynchronousObjectBundle bdl;
    T<Serializer>::auto_ptr serial( Serializer::create_standalone(engine) );
    T<Song>::shared_ptr song;

    // Give command to load and wait until it's complete.
    serial->load_file(filename, bdl, engine);
    while( ! bdl.ready ) {
        usleep(250);
    }

    if( ! bdl.error ) {
        /* TODO: For now, while refactoring, a song load bundle
         * will carry individual instruments.  However, we must
         * splice these into the song manually.
         *
         * The objects in the bundle can come in any order.
         */
        T<InstrumentList>::auto_ptr insts( new InstrumentList );
        deque< T<LadspaFX>::shared_ptr > fx;

        while( ! bdl.empty() ) {
            switch(bdl.peek_type()) {
            case ObjectItem::Song_t:
                // Should only load one song.
                assert( ! song );
                song = bdl.pop<Song>();
                break;
            case ObjectItem::Instrument_t:
                insts->add( bdl.pop<Instrument>() );
                break;
            case ObjectItem::LadspaFX_t:
                fx.push_back( bdl.pop<LadspaFX>() );
                break;
            default:
                // Should not have loaded anything else.
                assert(false);
            }
        }

        // Insert the instruments.
        song->set_instrument_list( insts.release() );

        // Set up the effects.
#ifdef LADSPA_SUPPORT
        // reset FX
        int k;
        for( k=0 ; k < MAX_FX ; ++k ) {
            engine->get_effects()->getLadspaFX(k).reset();
        }
#endif
        deque< T<LadspaFX>::shared_ptr >::iterator i_fx;
        for( k=0, i_fx = fx.begin() ; i_fx != fx.end() && k<MAX_FX ; ++k, ++i_fx ) {
            engine->get_effects()->setLadspaFX( *i_fx, k );
        }

    } else {
        INFOLOG( QString("Error with file '%1'").arg(filename) );
        INFOLOG( bdl.error_message );
        return song;
    }

    song->set_modified(false);
    song->set_filename(filename);

    return song;
}
