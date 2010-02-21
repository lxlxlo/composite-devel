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
#ifndef COMPOSITE_PLUGIN_ENGINELV2_HPP
#define COMPOSITE_PLUGIN_ENGINELV2_HPP

#include <lv2.h>
#include <event.lv2/event.h>

#include <Tritium/memory.hpp>

namespace Tritium
{
    class MixerImpl;
    class Sampler;
    class SeqScript;
    class AudioPortImpl;
} // namespace Tritium

namespace Composite
{
    namespace Plugin
    {
	/**
	 * \brief The main engine for the LV2 plugin version of the sampler.
	 *
	 */
	class EngineLv2
	{
	public:
	    EngineLv2();
	    virtual ~EngineLv2();

	    // Callbacks for LV2 interface
	    static LV2_Handle instantiate(const LV2_Descriptor *descriptor,
					  double sample_rate,
					  const char * bundle_path,
					  const LV2_Feature * const * features);
	    static void connect_port(LV2_Handle instance,
				     uint32_t port,
				     void *data_location);
	    static void activate(LV2_Handle instance);
	    static void run(LV2_Handle instance,
			    uint32_t sample_count);
	    static void deactivate(LV2_Handle instance);
	    static void cleanup(LV2_Handle instance);
	    static const void* extension_data(const char * uri);

	    Tritium::T<Tritium::Sampler>::shared_ptr sampler();
	    Tritium::T<Tritium::MixerImpl>::shared_ptr mixer();

	    // The real versions of the callbacks above.
	    void _connect_port(uint32_t port, void* data_location);
	    void _activate();
	    void _run(uint32_t sample_count);
	    void _deactivate();

	    void set_sample_rate(double sample_rate) {
		_sample_rate = sample_rate;
	    }

	    double get_sample_rate() {
		return _sample_rate;
	    }

	protected:
	    void process_events(uint32_t sample_count);

	private:
	    double _sample_rate;
	    float *_out_L; // Port 0, extern
	    float *_out_R; // Port 1, extern
	    LV2_Event_Buffer *_ev_in; // Port 2, extern
	    const LV2_Event_Feature *_event_feature; // Host's Event callbacks.
	    Tritium::T<Tritium::MixerImpl>::shared_ptr _mixer;
	    Tritium::T<Tritium::Sampler>::shared_ptr _sampler;
	    Tritium::T<Tritium::SeqScript>::auto_ptr _seq;
	};

    } // namespace Plugin
} // namespace Composite

#endif // COMPOSITE_PLUGIN_ENGINELV2_HPP
