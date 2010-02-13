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
#ifndef TRITIUM_MIXER_HPP
#define TRITIUM_MIXER_HPP

#include <stdint.h>
#include <Tritium/AudioPortManager.hpp>
#include <Tritium/memory.hpp>
#include <Tritium/globals.hpp>

namespace Tritium
{
    class MixerPrivate;
    class ChannelPrivate;

    /**
     * \brief The master mix device
     *
     */
    class Mixer : public AudioPortManager
    {
    public:

	class Channel
	{
	public:
	    Channel();
	    ~Channel();

	    Channel(const Channel& c);
	    Channel& operator=(const Channel& o);

	    const T<AudioPort>::shared_ptr port() const;
	    T<AudioPort>::shared_ptr& port();

	    float gain() const;
	    void gain(float gain);

	    // Note: pan() and pan_L() are identical.  When dealing
	    // with mono channels, it makes more sense to code pan()
	    // rather than pan_L().

	    float pan() const;
	    void pan(float pan);

	    float pan_L() const;
	    void pan_L(float pan);

	    float pan_R() const;
	    void pan_R(float pan);

	private:
	    ChannelPrivate *d; // Declared in MixerPrivate.hpp
	};

	Mixer(uint32_t max_buffer = MAX_BUFFER_SIZE);
	virtual ~Mixer();

	// AudioPortManager interface

	virtual T<AudioPort>::shared_ptr allocate_port(
	    const QString& name,
	    AudioPort::flow_t in_or_out = AudioPort::OUTPUT,
	    AudioPort::type_t type = AudioPort::MONO,
	    size_t size = -1
	    );
	virtual void release_port(T<AudioPort>::shared_ptr port);

	/**
	 * Prepare for another audio cycle.
	 */
	void pre_process();

	/**
	 * Signals that all channels are written and it's time to render send/returns
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

	/**
	 * Returns the number of audio channels being input into mixer.
	 *
	 * Does not count send/return or FX loops.
	 */
	size_t count();

	/**
	 * Returns the port at index n
	 */
	T<AudioPort>::shared_ptr port(size_t n);

	/**
	 * Returns the port and mixer-specific settings (gain, pan, etc.).
	 */
	T<Channel>::shared_ptr channel(size_t n);

	/**
	 * Convenience class if you already have a port pointer.
	 */
	T<Channel>::shared_ptr channel(const T<AudioPort>::shared_ptr port);

    private:
	MixerPrivate *d;
    };

} // namespace Tritium

#endif // TRITIUM_MIXER_HPP
