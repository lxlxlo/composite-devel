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
#include <Tritium/EngineInterface.hpp>
#include <Tritium/Sample.hpp>
#include <Tritium/Sampler.hpp>
#include <Tritium/Mixer.hpp>
#include <Tritium/Note.hpp>
#include <Tritium/SoundLibrary.hpp>
#include <Tritium/fx/Effects.hpp>
#include <Tritium/memory.hpp>
#include <Tritium/Preferences.hpp>
#include <Tritium/DataPath.hpp>
#include <Tritium/Presets.hpp>
#include "TritiumXml.hpp"
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

Serializer* Serializer::create_standalone(EngineInterface* engine)
{
    return new SerializerStandalone(engine);
}

/*********************************************************************
 * SerializerImpl implementation
 *********************************************************************
 */

SerializerImpl::SerializerImpl(EngineInterface* engine) :
    m_queue( new SerializationQueue(engine) )
{
}

SerializerImpl::~SerializerImpl()
{
}

void SerializerImpl::load_uri(const QString& uri,
			      ObjectBundle& report_to,
			      EngineInterface* engine)
{
    m_queue->load_uri(uri, report_to, engine);
}

void SerializerImpl::save_song(const QString& filename,
                               T<Song>::shared_ptr song,
                               SaveReport& report_t,
                               EngineInterface* engine,
                               bool overwrite)
{
    m_queue->save_song(filename, song, report_t, engine, overwrite);
}

void SerializerImpl::save_drumkit(const QString& dirname,
                                  T<Drumkit>::shared_ptr dk,
                                  SaveReport& report_to,
                                  EngineInterface* engine,
                                  bool overwrite)
{
    m_queue->save_drumkit(dirname, dk, report_to, engine, overwrite);
}

void SerializerImpl::save_pattern(const QString& filename,
                                  T<Pattern>::shared_ptr pattern,
				  const QString& drumkit_name,
                                  SaveReport& report_to,
                                  EngineInterface* engine,
                                  bool overwrite)
{
    m_queue->save_pattern(filename, pattern, drumkit_name, report_to, engine, overwrite);
}

/*********************************************************************
 * SerializerStandalone implementation
 *********************************************************************
 */

SerializerStandalone::SerializerStandalone(EngineInterface* engine) :
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

SerializationQueue::SerializationQueue(EngineInterface* engine) :
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

void SerializationQueue::load_uri(const QString& uri,
				  ObjectBundle& report_to,
				  EngineInterface *engine)
{
    event_data_t event;
    event.ev = LoadUri;
    event.uri = uri;
    event.report_load_to = &report_to;
    event.engine = engine;
    event.overwrite = false;
    m_queue.push_back(event);
}

void SerializationQueue::save_song(const QString& filename,
                                   T<Song>::shared_ptr song,
                                   SaveReport& report_t,
                                   EngineInterface *engine,
                                   bool overwrite)
{
    if(song && engine) {
	song->set_volume( engine->get_mixer()->gain() );

	event_data_t event;
	event.ev = SaveSong;
	event.uri = filename;
	event.report_save_to = &report_t;
	event.engine = engine;
	event.song = song;
	event.overwrite = overwrite;
	m_queue.push_back(event);
    }
}

void SerializationQueue::save_drumkit(const QString& filename,
                                      T<Drumkit>::shared_ptr dk,
                                      SaveReport& report_t,
                                      EngineInterface *engine,
                                      bool overwrite)
{
    if(dk && engine) {
	event_data_t event;
	event.ev = SaveDrumkit;
	event.uri = filename;
	event.report_save_to = &report_t;
	event.engine = engine;
	event.drumkit = dk;
	event.overwrite = overwrite;
	m_queue.push_back(event);
    }
}

void SerializationQueue::save_pattern(const QString& filename,
                                      T<Pattern>::shared_ptr pattern,
				      const QString& drumkit_name,
                                      SaveReport& report_t,
                                      EngineInterface *engine,
                                      bool overwrite)
{
    if(pattern && engine) {
	event_data_t event;
	event.ev = SavePattern;
	event.uri = filename;
	event.drumkit_name = drumkit_name;
	event.report_save_to = &report_t;
	event.engine = engine;
	event.pattern = pattern;
	event.overwrite = overwrite;
	m_queue.push_back(event);
    }
}

int SerializationQueue::process()
{
    queue_t::iterator it;

    it = m_queue.begin();
    while( it != m_queue.end() && !m_kill ) {
        switch(it->ev) {
        case LoadUri:
            handle_load_uri(*it);
            break;
        case SaveSong:
            handle_save_song(*it, (*it).uri);
            break;
        case SaveDrumkit:
            handle_save_drumkit(*it, (*it).uri);
            break;
        case SavePattern:
            handle_save_pattern(*it, (*it).uri);
            break;
        }
        ++it;
        m_queue.pop_front();
    }
    return 0;
}

/**
 * If the defaults required by the URI doesn't exist... make sure it does
 *
 */
bool SerializationQueue::ensure_default_exists(const QUrl& uri)
{
    if( uri.scheme() != "tritium" ) {
	return false;
    }

    if( ! uri.path().startsWith("default/") ) {
	return false;
    }

    // Only presets are handled at this time.
    if( ! uri.path().startsWith("default/presets") ) {
	return false;
    }

    QString user = m_engine->get_preferences()->getDataDirectory();
    QString path = uri.path().replace("default/presets", "presets/default") + ".xml";
    QString def = "presets/default.xml";

    QFileInfo f_info(user + "/" + path);

    if( !f_info.exists() ) {
	// Use default
	f_info.setFile(user + "/" + def);
    }

    if( !f_info.exists() ) {
	if( ! f_info.absoluteDir().exists() ) {
	    QDir dir = f_info.absoluteDir();
	    if( ! dir.mkpath(".") ) {
		ERRORLOG(QString("Unable to create directory '%1'")
			 .arg(dir.absolutePath())
		    );
		return false;
	    }
	}
	T<Presets>::shared_ptr presets(new Presets);
	presets->generate_default_presets(m_engine->get_preferences());
	TritiumXml writer;
	writer.push(presets);
	QString data;
	if( ! writer.writeContent(data) ) {
	    ERRORLOG(QString("Error generating default presets: %1")
		     .arg(writer.error())
		);
	    return false;
	}
	QFile fp(f_info.absoluteFilePath());
	if( !fp.open(QIODevice::ReadWrite) ) {
	    ERRORLOG(QString("Unable to open file '%1' for writing")
		     .arg(f_info.absoluteFilePath())
		);
	    return false;
	}
	if( !fp.write(data.toUtf8()) ) {
	    ERRORLOG(QString("Unable to write data to file '%1'")
		     .arg(f_info.absoluteFilePath())
		);
	    return false;
	}
	fp.close();
    }
    if( f_info.exists() && f_info.isFile() ) {
	return true;
    }
    return false;
}

/**
 * Resolves a URI to a filename and then calls handle_load_file()
 */
void SerializationQueue::handle_load_uri(SerializationQueue::event_data_t& ev)
{
    QUrl uri(ev.uri);
    QString filename;

    if( uri.scheme() == "" ) {
	filename = ev.uri;
    } else if ( uri.scheme() == "file" ) {
	if( ! uri.authority().isEmpty() ) {
	    ERRORLOG(QString("URI authority '%1' unhandled, assuming to be localhost")
		     .arg(uri.authority())
		);
	}
	filename = uri.path();
    } else if ( uri.scheme() == "tritium" ) {
	/* XXX TODO This is really ugly.  Need a class that
	 * handles tritium: URI mappings.  It was done like
	 * this because it's quick and dirty.... and ugly. :-)
	 */
	QString user = m_engine->get_preferences()->getDataDirectory();
	QString syst(DataPath::get_data_path());
	QString path( uri.path() );

	if(path.startsWith("drumkits/")) {
	    path += "/drumkit.xml";
	}

	if(path.startsWith("default/")) {
	    // Redefine path based on URI rules.
	    if( ensure_default_exists(uri) ) {
		// Only supporting presets at this time...
		assert(path.startsWith("default/presets"));
		path = path.replace("default/presets", "presets/default") + ".xml";
		QFileInfo finf(user + "/" + path);
		if( ! finf.exists() ) {
		    path = "presets/default.xml";
		}
	    } else {
		// not much we can do here.
	    }
	}

	user += "/" + path;
	syst += "/" + path;
	QFileInfo f_user(user);
	QFileInfo f_syst(syst);

	if( f_user.exists() ) {
	    filename = user;
	} else if (f_syst.exists() ) {
	    filename = syst;
	}
    } else {
	ERRORLOG(QString("URI scheme '%1' not understood").arg(uri.scheme()));
    }
    handle_load_file(ev, filename);
}

void SerializationQueue::handle_load_file(SerializationQueue::event_data_t& ev, const QString& filename)
{
    QFile file(filename);
    if( QFile(filename).exists()) {
        if(filename.endsWith(".h2song")) {
            handle_load_song(ev, filename);
        } else if (filename.endsWith(".h2pattern")) {
            handle_load_pattern(ev, filename);
        } else if (filename.endsWith("drumkit.xml")) {
            handle_load_drumkit(ev, filename);
	} else if (filename.endsWith(".xml")) {
	    #warning "XXX TODO: Was supposed to classify based on /content/, not file name"
	    handle_load_tritium(ev, filename);
        } else {
	    handle_callback(
		ev,
		filename,
		true,
		QString("File '%1' is not in a valid format (uri=%2)")
		.arg(filename)
		.arg(ev.uri)
		);
        }
    } else {
	handle_callback(
	    ev,
	    filename,
	    true,
	    QString("File '%1' does not exist (uri=%2)")
	    .arg(filename)
	    .arg(ev.uri)
	    );
    }
}

void SerializationQueue::handle_save_song(SerializationQueue::event_data_t& ev, const QString& filename)
{
    EngineInterface *engine = m_engine;
    Song& song = *(ev.song);

    DEBUGLOG( "Saving song " + filename );
    int rv = 0; // return value

    // FIXME: has the file write-permssion?
    // FIXME: verificare che il file non sia gia' esistente
    // FIXME: effettuare copia di backup per il file gia' esistente


    QDomDocument doc;
    QDomProcessingInstruction header = doc.createProcessingInstruction( "xml", "version=\"1.0\" encoding=\"UTF-8\"");
    doc.appendChild( header );

    QDomNode songNode = doc.createElement( "song" );

    LocalFileMng::writeXmlString( songNode, "version", QString( get_version().c_str() ) );
    LocalFileMng::writeXmlString( songNode, "bpm", QString("%1").arg( song.get_bpm() ) );
    LocalFileMng::writeXmlString( songNode, "volume", QString("%1").arg( song.get_volume() ) );
    LocalFileMng::writeXmlString( songNode, "metronomeVolume", QString("%1").arg( song.get_metronome_volume() ) );
    LocalFileMng::writeXmlString( songNode, "name", song.get_name() );
    LocalFileMng::writeXmlString( songNode, "author", song.get_author() );
    LocalFileMng::writeXmlString( songNode, "notes", song.get_notes() );
    LocalFileMng::writeXmlString( songNode, "license", song.get_license() );
    LocalFileMng::writeXmlBool( songNode, "loopEnabled", song.is_loop_enabled() );

    if ( song.get_mode() == Song::SONG_MODE ) {
	LocalFileMng::writeXmlString( songNode, "mode", QString( "song" ) );
    } else {
	LocalFileMng::writeXmlString( songNode, "mode", QString( "pattern" ) );
    }

    LocalFileMng::writeXmlString( songNode, "humanize_time", QString("%1").arg( song.get_humanize_time_value() ) );
    LocalFileMng::writeXmlString( songNode, "humanize_velocity", QString("%1").arg( song.get_humanize_velocity_value() ) );
    LocalFileMng::writeXmlString( songNode, "swing_factor", QString("%1").arg( song.get_swing_factor() ) );

    /*      LocalFileMng::writeXmlBool( &songNode, "delayFXEnabled", song.m_bDelayFXEnabled );
	    LocalFileMng::writeXmlString( &songNode, "delayFXWetLevel", QString("%1").arg( song.m_fDelayFXWetLevel ) );
	    LocalFileMng::writeXmlString( &songNode, "delayFXFeedback", QString("%1").arg( song.m_fDelayFXFeedback ) );
	    LocalFileMng::writeXmlString( &songNode, "delayFXTime", QString("%1").arg( song.m_nDelayFXTime ) );
    */

    // instrument list
    QDomNode instrumentListNode = doc.createElement( "instrumentList" );
    T<InstrumentList>::shared_ptr instrument_list = m_engine->get_sampler()->get_instrument_list();
    unsigned nInstrument = instrument_list->get_size();

    // INSTRUMENT NODE
    for ( unsigned i = 0; i < nInstrument; i++ ) {
	T<Instrument>::shared_ptr instr = instrument_list->get( i );
	T<Mixer::Channel>::shared_ptr chan = m_engine->get_mixer()->channel( i );

	assert( instr );

	QDomNode instrumentNode = doc.createElement( "instrument" );

	LocalFileMng::writeXmlString( instrumentNode, "id", instr->get_id() );
	LocalFileMng::writeXmlString( instrumentNode, "drumkit", instr->get_drumkit_name() );
	LocalFileMng::writeXmlString( instrumentNode, "name", instr->get_name() );
	LocalFileMng::writeXmlString( instrumentNode, "volume", QString("%1").arg( chan->gain() ) );
	LocalFileMng::writeXmlBool( instrumentNode, "isMuted", instr->is_muted() );
	LocalFileMng::writeXmlString( instrumentNode, "pan_L", QString("%1").arg( instr->get_pan_l() ) );
	LocalFileMng::writeXmlString( instrumentNode, "pan_R", QString("%1").arg( instr->get_pan_r() ) );
	LocalFileMng::writeXmlString( instrumentNode, "gain", QString("%1").arg( instr->get_gain() ) );

	LocalFileMng::writeXmlBool( instrumentNode, "filterActive", instr->is_filter_active() );
	LocalFileMng::writeXmlString( instrumentNode, "filterCutoff", QString("%1").arg( instr->get_filter_cutoff() ) );
	LocalFileMng::writeXmlString( instrumentNode, "filterResonance", QString("%1").arg( instr->get_filter_resonance() ) );

	LocalFileMng::writeXmlString( instrumentNode, "FX1Level", QString("%1").arg( chan->send_gain(0) ) );
	LocalFileMng::writeXmlString( instrumentNode, "FX2Level", QString("%1").arg( chan->send_gain(1) ) );
	LocalFileMng::writeXmlString( instrumentNode, "FX3Level", QString("%1").arg( chan->send_gain(2) ) );
	LocalFileMng::writeXmlString( instrumentNode, "FX4Level", QString("%1").arg( chan->send_gain(3) ) );

	assert( instr->get_adsr() );
	LocalFileMng::writeXmlString( instrumentNode, "Attack", QString("%1").arg( instr->get_adsr()->__attack ) );
	LocalFileMng::writeXmlString( instrumentNode, "Decay", QString("%1").arg( instr->get_adsr()->__decay ) );
	LocalFileMng::writeXmlString( instrumentNode, "Sustain", QString("%1").arg( instr->get_adsr()->__sustain ) );
	LocalFileMng::writeXmlString( instrumentNode, "Release", QString("%1").arg( instr->get_adsr()->__release ) );

	LocalFileMng::writeXmlString( instrumentNode, "randomPitchFactor", QString("%1").arg( instr->get_random_pitch_factor() ) );

	LocalFileMng::writeXmlString( instrumentNode, "muteGroup", QString("%1").arg( instr->get_mute_group() ) );

	for ( unsigned nLayer = 0; nLayer < MAX_LAYERS; nLayer++ ) {
	    InstrumentLayer *pLayer = instr->get_layer( nLayer );
	    if ( pLayer == NULL ) continue;
	    T<Sample>::shared_ptr pSample = pLayer->get_sample();
	    if ( pSample == NULL ) continue;

	    QString sFilename = pSample->get_filename();
	    if ( !instr->get_drumkit_name().isEmpty() ) {
		// se e' specificato un drumkit, considero solo il nome del file senza il path
		int nPos = sFilename.lastIndexOf( "/" );
		sFilename = sFilename.mid( nPos + 1, sFilename.length() );
	    }
	    QDomNode layerNode = doc.createElement( "layer" );
	    LocalFileMng::writeXmlString( layerNode, "filename", sFilename );
	    LocalFileMng::writeXmlString( layerNode, "min", QString("%1").arg( pLayer->get_min_velocity() ) );
	    LocalFileMng::writeXmlString( layerNode, "max", QString("%1").arg( pLayer->get_max_velocity() ) );
	    LocalFileMng::writeXmlString( layerNode, "gain", QString("%1").arg( pLayer->get_gain() ) );
	    LocalFileMng::writeXmlString( layerNode, "pitch", QString("%1").arg( pLayer->get_pitch() ) );

	    instrumentNode.appendChild( layerNode );
	}

	instrumentListNode.appendChild( instrumentNode );
    }
    songNode.appendChild( instrumentListNode );


    // pattern list
    QDomNode patternListNode = doc.createElement( "patternList" );

    unsigned nPatterns = song.get_pattern_list()->get_size();
    for ( unsigned i = 0; i < nPatterns; i++ ) {
	T<Pattern>::shared_ptr pat = song.get_pattern_list()->get( i );

	// pattern
	QDomNode patternNode = doc.createElement( "pattern" );
	LocalFileMng::writeXmlString( patternNode, "name", pat->get_name() );
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

	patternListNode.appendChild( patternNode );
    }
    songNode.appendChild( patternListNode );


    // pattern sequence
    QDomNode patternSequenceNode = doc.createElement( "patternSequence" );

    unsigned nPatternGroups = song.get_pattern_group_vector()->size();
    for ( unsigned i = 0; i < nPatternGroups; i++ ) {
	QDomNode groupNode = doc.createElement( "group" );

	T<PatternList>::shared_ptr pList = ( *song.get_pattern_group_vector() )[i];
	for ( unsigned j = 0; j < pList->get_size(); j++ ) {
	    T<Pattern>::shared_ptr pPattern = pList->get( j );
	    LocalFileMng::writeXmlString( groupNode, "patternID", pPattern->get_name() );
	}
	patternSequenceNode.appendChild( groupNode );
    }

    songNode.appendChild( patternSequenceNode );


    // LADSPA FX
    QDomNode ladspaFxNode = doc.createElement( "ladspa" );

    for ( unsigned nFX = 0; nFX < MAX_FX; nFX++ ) {
	QDomNode fxNode = doc.createElement( "fx" );

#ifdef LADSPA_SUPPORT
	T<Effects>::shared_ptr effects = engine->get_effects();
	T<LadspaFX>::shared_ptr pFX;
	if(effects) {
	    pFX = effects->getLadspaFX( nFX );
	}
	if ( pFX ) {
	    LocalFileMng::writeXmlString( fxNode, "name", pFX->getPluginLabel() );
	    LocalFileMng::writeXmlString( fxNode, "filename", pFX->getLibraryPath() );
	    LocalFileMng::writeXmlBool( fxNode, "enabled", pFX->isEnabled() );
	    LocalFileMng::writeXmlString( fxNode, "volume", QString("%1").arg( pFX->getVolume() ) );
	    for ( unsigned nControl = 0; nControl < pFX->inputControlPorts.size(); nControl++ ) {
		LadspaControlPort *pControlPort = pFX->inputControlPorts[ nControl ];
		QDomNode controlPortNode = doc.createElement( "inputControlPort" );
		LocalFileMng::writeXmlString( controlPortNode, "name", pControlPort->sName );
		LocalFileMng::writeXmlString( controlPortNode, "value", QString("%1").arg( pControlPort->fControlValue ) );
		fxNode.appendChild( controlPortNode );
	    }
	    for ( unsigned nControl = 0; nControl < pFX->outputControlPorts.size(); nControl++ ) {
		LadspaControlPort *pControlPort = pFX->inputControlPorts[ nControl ];
		QDomNode controlPortNode = doc.createElement( "outputControlPort" );
		LocalFileMng::writeXmlString( controlPortNode, "name", pControlPort->sName );
		LocalFileMng::writeXmlString( controlPortNode, "value", QString("%1").arg( pControlPort->fControlValue ) );
		fxNode.appendChild( controlPortNode );
	    }
	}
#else
	if ( false ) {
	}
#endif
	else {
	    LocalFileMng::writeXmlString( fxNode, "name", QString( "no plugin" ) );
	    LocalFileMng::writeXmlString( fxNode, "filename", QString( "-" ) );
	    LocalFileMng::writeXmlBool( fxNode, "enabled", false );
	    LocalFileMng::writeXmlString( fxNode, "volume", "0.0" );
	}
	ladspaFxNode.appendChild( fxNode );
    }

    songNode.appendChild( ladspaFxNode );
    doc.appendChild( songNode );

    QFile file(filename);
    if ( !file.open(QIODevice::WriteOnly) )
	rv = 1;

    QTextStream TextStream( &file );
    doc.save( TextStream, 1 );

    file.close();


    song.set_filename( filename );
    if( rv ) {
	handle_callback(
	    ev,
	    filename,
	    true,
	    "There was an error in saving the file."
	    );
    } else {
	song.set_modified(false);
	handle_callback(ev, filename);
    }
}

/**
 * Saves a drumkit to a folder.
 *
 * filename should point to a specific folder where the drumkit
 * should be saved.  It should \em not point to the drumkit.xml
 * manifest.  It \should point to the exact directory to store the kit
 * (i.e. the last part of the path is typically the name of the
 * drumkit).  If the folder does not exist, it will be created.
 */
void SerializationQueue::handle_save_drumkit(SerializationQueue::event_data_t& ev, const QString& filename)
{
    T<Drumkit>::shared_ptr drumkit = ev.drumkit;

    if( Logger::get_log_level() & Logger::Info ) {
	drumkit->dump();
    }

    QVector<QString> tempVector(16);

    QString sDrumkitDir = filename;

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
	handle_callback(
	    ev,
	    filename,
	    true,
	    QString("Could create folder '%1'").arg( sDrumkitDir )
	    );
	return;
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
	T<Mixer::Channel>::shared_ptr chan = drumkit->channels()[i];

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
		    DEBUGLOG("sample is already in drumkit dir");
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
	LocalFileMng::writeXmlString( instrumentNode, "volume", QString("%1").arg( chan->gain() ) );
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
	handle_callback(
	    ev,
	    filename,
	    true,
	    QString("Could not open file '%1' to write").arg(sDrumkitXmlFilename)
	    );
	return;
    }

    QTextStream TextStream( &file );
    doc.save( TextStream, 1 );

    file.close();

    handle_callback(ev, filename);
}

void SerializationQueue::handle_save_pattern(SerializationQueue::event_data_t& ev, const QString& filename)
{
    //int mode = 1 save, int mode = 2 save as
    // INSTRUMENT NODE


    // Requirements:  Must have ev.pattern and ev.drumkit set.
    // filename must be a full path.
    assert(ev.ev == SavePattern);
    assert(ev.pattern);
    T<Pattern>::shared_ptr pat = ev.pattern;
    QString drumkit_name = ev.drumkit_name;
    QString sPatternXmlFilename = filename;
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
	handle_callback(
	    ev,
	    filename,
	    true,
	    QString("Could not create directory '%1' for save.").arg( dir.path() )
	    );
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
	handle_callback(
	    ev,
	    filename,
	    true,
	    QString("Could not create file '%1' for save.").arg( filename )
	    );
	return;
    }

    QTextStream TextStream( &file );
    doc.save( TextStream, 1 );

    file.close();


    QFileInfo check_file_written( sPatternXmlFilename );
    if ( !check_file_written.exists() ) {
	handle_callback(
	    ev,
	    filename,
	    true,
	    QString("Could not create directory '%1' for save.").arg( dir.path() )
	    );
	return;
    }

    handle_callback(ev, filename);
}

void SerializationQueue::handle_load_song(SerializationQueue::event_data_t& ev, const QString& filename)
{
    QDomDocument song_doc = LocalFileMng::openXmlDocument(filename);
    QDomElement song_node = song_doc.documentElement();
    QStringList errors;

    if( song_node.tagName() != "song" ) {
	handle_callback(
	    ev,
	    filename,
	    true,
	    "Not a valid .h2song."
	    );
        return;
    }
    QDomElement instrumentList_node =
        song_node.firstChildElement("instrumentList");
    if( instrumentList_node.isNull() ) {
	handle_callback(
	    ev,
	    filename,
	    true,
            ".h2song missing instrumentList section."
	    );
        return;
    }
    QDomElement patternList_node = song_node.firstChildElement("patternList");
    if( patternList_node.isNull() ) {
	handle_callback(
	    ev,
	    filename,
	    true,
            ".h2song missing patternList section."
	    );
        return;
    }
    QDomElement patternSequence_node =
        song_node.firstChildElement("patternSequence");
    if( patternList_node.isNull() ) {
	handle_callback(
	    ev,
	    filename,
	    true,
            ".h2song missing patternSequence section."
	    );
        return;
    }
    // Null is OK.
    QDomElement ladspa_node = song_node.firstChildElement("ladspa");

    // LOAD SONG-SPECIFIC DATA
    T<Song>::shared_ptr song = handle_load_song_node(song_node, errors);
    song->set_filename(filename);

    // LOAD INSTRUMENTS
    deque< T<Instrument>::shared_ptr > instrument_ra;
    deque< T<Mixer::Channel>::shared_ptr > channel_ra;
    handle_load_instrumentlist_node(instrument_ra, channel_ra, "-", instrumentList_node, errors);

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
    size_t k;
    for(k=0 ; k<instrument_ra.size() && k<channel_ra.size() ; ++k) {
	bdl.push( instrument_ra[k] );
	bdl.push( channel_ra[k] );
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

    handle_callback(ev, filename);
}

void SerializationQueue::handle_load_drumkit(
    SerializationQueue::event_data_t& ev,
    const QString& filename
    )
{
    // Path information
    QFileInfo fn_info(filename);
    QString drumkit_dir = fn_info.absolutePath();

    if( ! fn_info.exists() ) {
	handle_callback(
	    ev,
	    filename,
	    true,
	    "File not found."
	    );
        return;
    }

    QDomDocument drumkit_doc = LocalFileMng::openXmlDocument(filename);

    if( drumkit_doc.isNull() ) {
	handle_callback(
	    ev,
	    filename,
	    true,
	    "Not an XML file."
	    );
        return;
    }

    QDomElement drumkit_info_node = drumkit_doc.documentElement();
    QStringList errors;

    if( drumkit_info_node.tagName() != "drumkit_info" ) {
	handle_callback(
	    ev,
	    filename,
	    true,
	    "Not a valid drumkit.xml file."
	    );
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
	handle_callback(
	    ev,
	    filename,
	    true,
            "drumkit.xml missing instrumentList section."
	    );
        return;
    }
    deque< T<Instrument>::shared_ptr > instrument_ra;
    deque< T<Mixer::Channel>::shared_ptr > channel_ra;
    handle_load_instrumentlist_node(instrument_ra,
				    channel_ra,
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
    size_t k;
    for(k=0 ; k<instrument_ra.size() && k<channel_ra.size() ; ++k) {
	bdl.push( instrument_ra[k] );
	bdl.push( channel_ra[k] );
    }

    handle_callback(ev, filename);
}

void SerializationQueue::handle_load_pattern(
    SerializationQueue::event_data_t& ev,
    const QString& filename
    )
{
    QDomDocument pattern_doc = LocalFileMng::openXmlDocument(filename);
    QDomElement dk_pattern_node = pattern_doc.documentElement();
    QStringList errors;

    if( dk_pattern_node.tagName() != "drumkit_pattern" ) {
	handle_callback(
	    ev,
	    filename,
	    true,
	    "Not a valid .h2pattern file."
	    );
        return;
    }
    QDomElement pat_node = dk_pattern_node.firstChildElement("pattern");
    if( pat_node.isNull() ) {
	handle_callback(
	    ev,
	    filename,
	    true,
	    ".h2pattern missing pattern section."
	    );
        return;
    }

    #warning "TODO: Converting InstrumentList to std::deque<>... not a great practice"
    deque< T<Instrument>::shared_ptr > insts;
    T<InstrumentList>::shared_ptr ilist = ev.engine->get_sampler()->get_instrument_list();
    for( unsigned k=0 ; k < ilist->get_size() ; ++k ) {
        insts.push_back( ilist->get(k) );
    }

    T<Pattern>::shared_ptr pat = handle_load_pattern_node(pat_node, insts,
                                                          errors);

    #warning "TODO: Need to handle errors!!"

    ev.report_load_to->push(pat);

    handle_callback(ev, filename);
}

void SerializationQueue::handle_load_tritium(
    SerializationQueue::event_data_t& ev,
    const QString& filename
    )
{
    TritiumXml reader;
    QFile file(filename);

    file.open(QIODevice::ReadOnly);
    reader.readContent( &file );
    file.close();

    ev.report_load_to->objects = reader.objects;
    ev.report_load_to->error = reader.error();
    ev.report_load_to->error_message = reader.error_message();

    handle_callback(ev, filename);
}

/**
 * Convenience function to handle the functors.
 *
 */
void SerializationQueue::handle_callback(
    event_data_t& ev,
    QString filename,
    bool error,
    QString error_message
    )
{
    switch(ev.ev) {
    case LoadUri:
	ev.report_load_to->error = error;
	ev.report_load_to->error_message = (error) ? error_message : QString();
	(*ev.report_load_to)();
	break;
    case SaveSong:
    case SaveDrumkit:
    case SavePattern:
	ev.report_save_to->filename = filename;
	if(error) {
	    ev.report_save_to->status = SaveReport::SaveFailed;
	    ev.report_save_to->message = error_message;
	} else {
	    ev.report_save_to->status = SaveReport::SaveSuccess;
	    ev.report_save_to->message = QString();
	}
	(*ev.report_save_to)();
	break;
    default:
	assert(false);
    }
}


T<Song>::shared_ptr SerializationQueue::handle_load_song_node(
    QDomElement songNode,
    QStringList& errors
    )
{
    QString m_sSongVersion = LocalFileMng::readXmlString( songNode , "version", "Unknown version" );

    if ( m_sSongVersion != QString( get_version().c_str() ) ) {
        DEBUGLOG( "Trying to load a song created with a different "
		  "version of Hydrogen/Tritium/Composite." );
        DEBUGLOG( "Song was saved with version " + m_sSongVersion );
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
    deque< T<Instrument>::shared_ptr >& inst_dest,
    deque< T<Mixer::Channel>::shared_ptr >& chan_dest,
    const QString& drumkit_path,
    QDomElement& instrumentList_node,
    QStringList& errors)
{
    QDomElement inst_node;
    T<Instrument>::shared_ptr i;
    T<Mixer::Channel>::shared_ptr c;
    inst_node = instrumentList_node.firstChildElement("instrument");
    while( ! inst_node.isNull() ) {
        handle_load_instrument_node(inst_node, drumkit_path, i, c, errors);
        if(i) inst_dest.push_back(i);
	if(c) chan_dest.push_back(c);
        inst_node = inst_node.nextSiblingElement("instrument");
    }
}

void SerializationQueue::handle_load_instrument_node(
    QDomElement& instrumentNode,
    const QString& drumkit_path,
    T<Instrument>::shared_ptr& inst_rv,
    T<Mixer::Channel>::shared_ptr& chan_rv,
    QStringList& errors
    )
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
        return;
    }

    // create a new instrument
    T<Instrument>::shared_ptr pInstrument(
        new Instrument(
            sId,
            sName,
            new ADSR( fAttack, fDecay, fSustain, fRelease )
            )
        );
    T<Mixer::Channel>::shared_ptr channel( new Mixer::Channel(4) );

    channel->gain( fVolume );
    pInstrument->set_muted( bIsMuted );
    pInstrument->set_pan_l( fPan_L );
    pInstrument->set_pan_r( fPan_R );
    pInstrument->set_drumkit_name( sDrumkit );
    channel->send_gain(0, fFX1Level);
    channel->send_gain(1, fFX2Level);
    channel->send_gain(2, fFX3Level);
    channel->send_gain(3, fFX4Level);
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

    inst_rv = pInstrument;
    chan_rv = channel;
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
