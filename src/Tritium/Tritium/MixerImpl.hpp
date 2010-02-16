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
#ifndef TRITIUM_MIXERIMPL_HPP
#define TRITIUM_MIXERIMPL_HPP

#include <stdint.h>
#include <Tritium/Mixer.hpp>
#include <Tritium/AudioPortManager.hpp>
#include <Tritium/memory.hpp>
#include <Tritium/globals.hpp>

namespace Tritium
{
    class MixerImplPrivate;
    class ChannelPrivate;
    class Effects;

    /**
     * \brief The master mix device
     *
     */
    class MixerImpl : public Mixer, public AudioPortManager
    {
    public:

	MixerImpl(uint32_t max_buffer = MAX_BUFFER_SIZE,
		  T<Effects>::shared_ptr fx_man = T<Effects>::shared_ptr(),
		  size_t effect_ct = 0);
	virtual ~MixerImpl();

	// AudioPortManager interface

	virtual T<AudioPort>::shared_ptr allocate_port(
	    const QString& name,
	    AudioPort::flow_t in_or_out = AudioPort::OUTPUT,
	    AudioPort::type_t type = AudioPort::MONO,
	    size_t size = -1
	    );
	virtual void release_port(T<AudioPort>::shared_ptr port);

	// Mixer interface

	virtual size_t count();
	virtual T<AudioPort>::shared_ptr port(size_t n);
	virtual T<Mixer::Channel>::shared_ptr channel(size_t n);
	virtual T<Mixer::Channel>::shared_ptr channel(const T<AudioPort>::shared_ptr port);

	/**
	 * Prepare DSP for another audio cycle.
	 */
	void pre_process(uint32_t nframes);

	/**
	 * Process all send/return channels.
	 *
	 * i.e. Sends to effects.
	 */
	void mix_send_return(uint32_t nframes);

	/**
	 * Mix to output buffers.
	 *
	 * This function is an intermediate API.  In the future, it's
	 * intended to have a more flexible output system, and
	 * possible manage the audio drivers internally.
	 */
	void mix_down(uint32_t nframes, float* left, float* right);

    private:
	MixerImplPrivate *d;
    };

} // namespace Tritium

#endif // TRITIUM_MIXERIMPL_HPP
