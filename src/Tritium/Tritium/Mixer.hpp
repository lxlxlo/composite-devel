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
    class ChannelPrivate;

    /**
     * \brief Abstract "public" interface for a mixer device
     *
     */
    class Mixer
    {
    public:

	class Channel
	{
	public:
	    Channel();
	    Channel(uint32_t sends);
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

	    /**
	     * Return maximum number of "send" gains.
	     */
	    uint32_t send_count() const;

	    float send_gain(uint32_t index) const;
	    void send_gain(uint32_t index, float gain);

	    /**
	     * Make everything except port() match the parameter.
	     */
	    void match_props(const Channel& other);

	private:
	    ChannelPrivate *d; // Declared in MixerImplPrivate.hpp
	};

	virtual ~Mixer() {}

	/**
	 * Set the master gain for output (master volume).
	 */
	virtual void gain(float gain) = 0;

	/**
	 * Get the master gain (volume) for output.  Default should always be 1.0.
	 */
	virtual float gain() = 0;

	/**
	 * Returns the number of audio channels being input into mixer.
	 *
	 * Does not count send/return or FX loops.
	 */
	virtual uint32_t count() = 0;

	/**
	 * Returns the port at index n
	 */
	virtual T<AudioPort>::shared_ptr port(uint32_t n) = 0;

	/**
	 * Returns the port and mixer-specific settings (gain, pan, etc.).
	 */
	virtual T<Channel>::shared_ptr channel(uint32_t n) = 0;

	/**
	 * Convenience class if you already have a port pointer.
	 */
	virtual T<Channel>::shared_ptr channel(const T<AudioPort>::shared_ptr port) = 0;

    };

} // namespace Tritium

#endif // TRITIUM_MIXER_HPP
