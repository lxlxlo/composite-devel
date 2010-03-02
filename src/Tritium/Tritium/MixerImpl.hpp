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
		  uint32_t effect_ct = 0);
	virtual ~MixerImpl();

	// AudioPortManager interface

	virtual T<AudioPort>::shared_ptr allocate_port(
	    const QString& name,
	    AudioPort::flow_t in_or_out = AudioPort::OUTPUT,
	    AudioPort::type_t type = AudioPort::MONO,
	    uint32_t size = -1
	    );
	virtual void release_port(T<AudioPort>::shared_ptr port);

	// Mixer interface

	virtual void gain(float gain);
	virtual float gain();
	virtual uint32_t count();
	virtual T<AudioPort>::shared_ptr port(uint32_t n);
	virtual T<Mixer::Channel>::shared_ptr channel(uint32_t n);
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
	 *
	 * \param nframes - The number of frames to process for the
	 * current cycle.
	 *
	 * \param left - Pointer to an array of float's that is at
	 * least nframes large.  The left channel audio data will be
	 * written to this buffer.  Values will be clipped to the
	 * range [-1.0, 1.0].
	 *
	 * \param right - Pointer to an array of float's that is at
	 * least nframes large.  The right channel audio data will be
	 * written to this buffer.  Values will be clipped to the
	 * range [-1.0, 1.0].
	 *
	 * \param peak_left - Pointer to a float where the
	 * left-channel peak value should be stored.  Will return a
	 * number between 0.0f and 1.0f.  If this pointer is null (0),
	 * it will be ignored.
	 *
	 * \param peak_left - Pointer to a float where the
	 * right-channel peak value should be stored.  Will return a
	 * number between 0.0f and 1.0f.  If this pointer is null (0),
	 * it will be ignored.
	 *
	 * \return The parameters left, right, peak_left, and
	 * peak_right are the return values.
	 */
	void mix_down(uint32_t nframes, float* left, float* right,
		      float* peak_left = 0, float* peak_right = 0);

    private:
	MixerImplPrivate *d;
    };

} // namespace Tritium

#endif // TRITIUM_MIXERIMPL_HPP
