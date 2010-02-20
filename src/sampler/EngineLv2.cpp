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

#include <lv2.h>
#include <cstdlib>

#define PLUGIN_URI "http://gabe.is-a-geek.org/composite/plugins/sampler/1"

static LV2_Descriptor *pluginDescriptor = NULL;

using namespace Composite::Plugin;
using namespace Tritium;

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
}

EngineLv2::EngineLv2() :
    _sample_rate(48000.0),
    _out_L(0),
    _out_R(0)
{
}

EngineLv2::~EngineLv2()
{
}

LV2_Handle EngineLv2::instantiate(const LV2_Descriptor * /*descriptor*/,
				  double sample_rate,
				  const char * /*bundle_path*/,
				  const LV2_Feature * const * /*features*/)
{
    Logger::create_instance();
    T<EngineLv2>::auto_ptr inst( new EngineLv2 );
    if( inst.get() ) {
	inst->set_sample_rate( sample_rate );
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
    }
}

void EngineLv2::_activate()
{
    _mixer.reset( new MixerImpl(MAX_BUFFER_SIZE) );
    _sampler.reset( new Sampler(_mixer) );
    _seq.reset( new SeqScript );
}

static float noise()
{
    return 2.0f * drand48() - 1.0f;
}

void EngineLv2::_run(uint32_t nframes)
{
    if( ! _out_L ) return;
    if( ! _out_R ) return;

    // Sanity checks
    assert(_mixer);
    assert(_sampler);
    assert(_seq.get());

    TransportPosition pos;

    _mixer->pre_process(nframes);

    // Sampler needs a TransportPosition, but only uses it for
    // the frame rate.
    pos.frame_rate = _sample_rate;
    _sampler->process(_seq->begin_const(),
		      _seq->end_const(nframes),
		      pos,
		      nframes);

    _mixer->mix_send_return(nframes);
    _mixer->mix_down(nframes, _out_L, _out_R, 0, 0);

    // XXX TODO DEBUG Output white noise as a test
    uint32_t ctr = nframes;
    while(ctr--) {
	_out_L[ctr] = noise();
	_out_R[ctr] = noise();
    }

    _seq->consumed(nframes);
}

void EngineLv2::_deactivate()
{
    _mixer.reset();
    _sampler.reset();
    _seq.reset();
}

const void* EngineLv2::extension_data(const char * /*uri*/)
{
    return 0;
}

T<Tritium::Sampler>::shared_ptr EngineLv2::sampler()
{
    return _sampler;
}

T<Tritium::MixerImpl>::shared_ptr EngineLv2::mixer()
{
    return _mixer;
}

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
