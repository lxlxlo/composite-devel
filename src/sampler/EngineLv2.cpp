/*
 * Copyright(c) 2009 by Gabriel M. Beddingfield <gabriel@teuton.org>
 *
 * This file is part of Composite
 *
 * Composite is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Composite is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY, without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include "EngineLv2.hpp"

#include <Tritium/globals.hpp> // MAX_BUFFER_SIZE
#include <Tritium/MixerImpl.hpp>
#include <Tritium/Sampler.hpp>
#include <Tritium/AudioPort.hpp>
#include <Tritium/SeqScript.hpp>
#include <Tritium/TransportPosition.hpp>
#include <Tritium/Logger.hpp>
#include <Tritium/Instrument.hpp>
#include <Tritium/InstrumentList.hpp>
#include <Tritium/Preferences.hpp>
#include <Tritium/DataPath.hpp>
#include <Tritium/Serialization.hpp>

#include <lv2.h>
#include <event.lv2/event.h>
#include <event.lv2/event-helpers.h>
#include <cstring>

#define PLUGIN_URI "http://gabe.is-a-geek.org/composite/plugins/sampler/1"
#define EVENT_URI "http://lv2plug.in/ns/ext/event"
// Sanity check
#define MAX_URI_LEN 128

static LV2_Descriptor *pluginDescriptor = NULL;

using namespace Composite::Plugin;
using namespace Tritium;

namespace Composite
{
namespace Plugin
{

void EngineLv2::connect_port(LV2_Handle instance,
			     uint32_t port,
			     void* data_location)
{
    ((EngineLv2*)(instance))->_connect_port(port, data_location);
}

void EngineLv2::activate(LV2_Handle instance)
{
    ((EngineLv2*)(instance))->_activate();
}

void EngineLv2::run(LV2_Handle instance,
		    uint32_t sample_count)
{
    ((EngineLv2*)(instance))->_run(sample_count);
}

void EngineLv2::deactivate(LV2_Handle instance)
{
    ((EngineLv2*)(instance))->_deactivate();
}

void EngineLv2::cleanup(LV2_Handle instance)
{
    EngineLv2* i;
    i = static_cast<EngineLv2*>(instance);
    delete i;
    delete Logger::get_instance();
}

EngineLv2::EngineLv2() :
    _sample_rate(48000.0),
    _out_L(0),
    _out_R(0),
    _ev_in(0),
    _event_feature(0)
{
}

EngineLv2::~EngineLv2()
{
    _deactivate();
}

LV2_Handle EngineLv2::instantiate(const LV2_Descriptor * /*descriptor*/,
				  double sample_rate,
				  const char * /*bundle_path*/,
				  const LV2_Feature * const * features)
{
    Logger::create_instance();
    T<EngineLv2>::auto_ptr inst( new EngineLv2 );
    if( inst.get() ) {
	inst->set_sample_rate( sample_rate );
	while(*features) {
	    const LV2_Feature *feat = *features;
	    if( 0 == strncmp(EVENT_URI, feat->URI, strnlen(EVENT_URI, MAX_URI_LEN)) ) {
		inst->_event_feature = static_cast<const LV2_Event_Feature*>(feat->data);
	    }
	    ++features;
	}
	return ((LV2_Handle) inst.release());
    }
    return 0;
}

void EngineLv2::_connect_port(uint32_t port, void* data_location)
{
    switch(port) {
    case 0:
	_out_L = static_cast<float*>(data_location);
	break;
    case 1:
	_out_R = static_cast<float*>(data_location);
	break;
    case 2:
	_ev_in = static_cast<LV2_Event_Buffer*>(data_location);
	break;
    }
}

void EngineLv2::_activate()
{
    _prefs.reset( new Preferences );
    _mixer.reset( new MixerImpl(MAX_BUFFER_SIZE) );
    _sampler.reset( new Sampler(_mixer) );
    _seq.reset( new SeqScript );
    _serializer.reset( Serialization::Serializer::create_standalone(this) );
    _obj_bdl.reset( new Composite::Plugin::ObjectBundle );
    load_drumkit_by_name("GMkit");
}

void EngineLv2::load_drumkit_by_name(const QString& name)
{
    #warning "This is an incomplete implementation"
    QString dk_dir = DataPath::get_data_path() + "/drumkits/" + name + "/drumkit.xml";
    load_drumkit(dk_dir);
}

void EngineLv2::load_drumkit(const QString& drumkit_xml)
{
    bool loading = _obj_bdl->loading();
    if(!loading) {
	ERRORLOG( QString("Unable to acquire loading object to load drumkit %1")
		  .arg(drumkit_xml)
	    );
	return;
    }

    _serializer->load_file(drumkit_xml, *_obj_bdl, this);
}

void EngineLv2::install_drumkit_bundle()
{
    if(_obj_bdl->state() != ObjectBundle::Ready) {
	return;
    }

    _sampler->clear();

    size_t k;
    T<Mixer::Channel>::shared_ptr tmp_chan;
    while( !_obj_bdl->empty() ) {
	switch(_obj_bdl->peek_type()) {
	case ObjectItem::Instrument_t:
	    _sampler->add_instrument( _obj_bdl->pop<Instrument>() );
	    break;
	case ObjectItem::Channel_t:
	    k = _mixer->count();
	    if(k>0) {
		--k;
		tmp_chan = _obj_bdl->pop<Mixer::Channel>();
		_mixer->channel(k)->match_props(*tmp_chan);
		tmp_chan.reset();
	    }
	    break;
	case ObjectItem::Drumkit_t:
	    // Intentionally ignoring
	    _obj_bdl->pop();
	    break;
	default:
	    ERRORLOG("Loading drumkit loaded an unexpected type.");
	    _obj_bdl->pop();
	}
    }

    _obj_bdl->reset();
}

void EngineLv2::_run(uint32_t nframes)
{
    if( ! _out_L ) return;
    if( ! _out_R ) return;

    // Check if we need to install a new drumkit.
    if( _obj_bdl->state() == ObjectBundle::Ready ) {
	install_drumkit_bundle();
    }

    // Sanity checks
    assert(_mixer);
    assert(_sampler);
    assert(_seq.get());

    TransportPosition pos;

    _mixer->pre_process(nframes);

    // Sampler needs a TransportPosition, but only uses it for
    // the frame rate.
    pos.frame_rate = _sample_rate;
    process_events(nframes);
    _sampler->process(_seq->begin_const(),
		      _seq->end_const(nframes),
		      pos,
		      nframes);

    _mixer->mix_send_return(nframes);
    _mixer->mix_down(nframes, _out_L, _out_R, 0, 0);

    _seq->consumed(nframes);
}

void EngineLv2::_deactivate()
{
    _out_L = 0;
    _out_R = 0;
    _obj_bdl.reset();
    _serializer.reset();
    _seq.reset();
    _sampler.reset();
    _mixer.reset();
    _prefs.reset();
}

void EngineLv2::process_events(uint32_t nframes)
{
    if( ! _ev_in ) return;

    LV2_Event_Iterator it;
    lv2_event_begin(&it, _ev_in);
    for( ; lv2_event_is_valid(&it) ; lv2_event_increment(&it) ) {
	uint8_t *data;
	LV2_Event& ev = *lv2_event_get(&it, &data);
	SeqEvent sev;

	ERRORLOG("Got event");
	sev.frame = ev.frames;
	// ev.subframes ignored
	if(0 == ev.type) {
	    ERRORLOG("Type 0??");
	    // Data is non-POD, and we don't support any such
	    // data.
	    _event_feature->lv2_event_unref(
		_event_feature->callback_data,
		&ev
		);
	} else {
	    #warning "This is not a real MIDI implementation"
	    // Just trigger a note to play
	    ERRORLOG( QString("Data = %1").arg( QString::number(data[0], 16) ) );
	    if( (data[0] & 0xF0) == 0x90 ) {
		ERRORLOG("Note Event");
		sev.type = SeqEvent::NOTE_ON;
		sev.quantize = false;
		sev.note.set_velocity(float(data[2]) / 127.0f);

		int k = data[1] - 36;
		T<InstrumentList>::shared_ptr i_list = _sampler->get_instrument_list();
		T<Instrument>::shared_ptr inst;
		if(k>=0 && k < i_list->get_size()) {
		    sev.note.set_instrument( i_list->get(k) );
		} else {
		    ERRORLOG("No instruments");
		}
		if(sev.note.get_instrument()) {
		    ERRORLOG("Scheduled note");
		    _seq->insert(sev);
		}
	    }
	}	
    }
}

const void* EngineLv2::extension_data(const char * /*uri*/)
{
    return 0;
}

T<Tritium::Preferences>::shared_ptr EngineLv2::get_preferences()
{
    return _prefs;
}

T<Tritium::Sampler>::shared_ptr EngineLv2::get_sampler()
{
    return _sampler;
}

T<Tritium::Mixer>::shared_ptr EngineLv2::get_mixer()
{
    return boost::dynamic_pointer_cast<Tritium::Mixer>(_mixer);
}

/**
 * Always returns a null pointer
 */
T<Tritium::Effects>::shared_ptr EngineLv2::get_effects()
{
    return T<Tritium::Effects>::shared_ptr();
}

void ObjectBundle::operator()()
{
    QMutexLocker lk(&_mutex);
    switch(_state) {
    case Loading:
	_state = Ready;
	break;
    case Empty:
    case Ready:
	break;
    }
}

void ObjectBundle::reset()
{
    QMutexLocker lk(&_mutex);
    switch(_state) {
    case Ready:
	_state = Empty;
	break;
    case Loading:
    case Empty:
	break;
    }
}

bool ObjectBundle::loading()
{
    QMutexLocker lk(&_mutex);
    bool rv = false;
    switch(_state) {
    case Empty:
	_state = Loading;
	rv = true;
	break;
    case Ready:
    case Loading:
	break;
    }
    return rv;
}

} // namespace Composite::Plugin

} // namespace Composite

static void plugin_init()
{
    LV2_Descriptor *d;
    typedef EngineLv2 p;

    pluginDescriptor = new LV2_Descriptor;
    d = pluginDescriptor;

    d->URI = PLUGIN_URI;
    d->activate = p::activate;
    d->cleanup = p::cleanup;
    d->connect_port = p::connect_port;
    d->deactivate = p::deactivate;
    d->instantiate = p::instantiate;
    d->run = p::run;
    d->extension_data = p::extension_data;
}

LV2_SYMBOL_EXPORT
const LV2_Descriptor *lv2_descriptor(uint32_t index)
{
    if (!pluginDescriptor) plugin_init();

    if( index == 0 ) {
	return pluginDescriptor;
    }
    return 0;
}
