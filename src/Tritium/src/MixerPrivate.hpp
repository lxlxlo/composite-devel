/*
 * Copyright(c) 2010 by Gabriel M. Beddingfield <gabriel@teuton.org>
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
#ifndef TRITIUM_MIXERPRIVATE_HPP
#define TRITIUM_MIXERPRIVATE_HPP

#include <Tritium/AudioPort.hpp>
#include <Tritium/memory.hpp>
#include "AudioPortImpl.hpp"
#include <deque>
#include <functional>
#include <QMutex>

namespace Tritium
{
    class MixerPrivate
    {
    public:
	typedef AudioPortImpl act_port_t;
	typedef T<AudioPort>::shared_ptr port_ref_t;
	typedef std::deque< Mixer::channel_t > port_list_t;

	uint32_t _max_buf;
	port_list_t _in_ports;
	QMutex _in_ports_mutex;

	port_ref_t new_stereo_port();
	port_ref_t new_mono_port();
	void delete_port(port_ref_t port);

	static void eval_pan(float gain, float pan, float& left, float& right);
	static void copy_buffer_no_gain(float* dst, float* src, uint32_t nframes);
	static void copy_buffer_with_gain(float* dst, float* src, uint32_t nframes, float gain);
	static void mix_buffer_no_gain(float* dst, float* src, uint32_t nframes);
	static void mix_buffer_with_gain(float* dst, float* src, uint32_t nframes, float gain);

	struct mult_gain : public std::unary_function<float, float>
	{
	    float gain;
	    float operator()(float t) {
		return gain * t;
	    }
	};

	struct add_with_gain : public std::unary_function<float, float>
	{
	    float gain;
	    float operator()(float _new, float orig) {
		return orig + _new * gain;
	    }
	};
    };

    bool operator==(const Mixer::channel_t& chan, const T<AudioPort>::shared_ptr port) {
	return chan.port == port;
    }

    bool operator==(const T<AudioPort>::shared_ptr port, const Mixer::channel_t& chan) {
	return chan.port == port;
    }



} // namespace Tritium

#endif // TRITIUM_MIXERPRIVATE_HPP
