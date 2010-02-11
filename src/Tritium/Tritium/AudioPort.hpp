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
#ifndef TRITIUM_AUDIOPORT_HPP
#define TRITIUM_AUDIOPORT_HPP

#include <stdint.h>
#include <Tritium/memory.hpp>

class QString;

namespace Tritium
{
    /**
     * An audio port.
     */
    class AudioPort
    {
    public:
	typedef float Float;

	typedef enum {
	    OUTPUT = 0,
	    INPUT = 1
	} flow_t;

	typedef enum {
	    MONO = 0,
	    STEREO = 1
	} type_t;

	virtual ~AudioPort() {}

	/**
	 * Sets a human-readable name for the port.
	 */
	virtual void set_name(const QString& name) = 0;

	/**
	 * Retrieve human-readable name for this port.
	 */
	virtual const QString& get_name() const = 0;

	/**
	 * Returns a pointer to the port's buffer.
	 *
	 * May return a different pointer on every invocation.  See
	 * size() for the size of the buffer.  For mono buffers, there
	 * is only one channel (0).  For stereo buffers, channel 1 is
	 * the Right channel.
	 *
	 * Side effect: Calling this function WILL clear the zero
	 * flag.
	 *
	 * \param chan 0 for the Left (or Mono) buffer, 1 for the
	 * Right buffer.
	 */
	virtual Float* get_buffer(unsigned chan = 0) = 0;

	/**
	 * Returns the size of the buffer returned by get_buffer().
	 */
	virtual uint32_t size() = 0;

	/**
	 * Returns the type of the port (whether mono or stereo).
	 */
	virtual type_t type() = 0;

	/**
	 * If true, ignore buffer data and presume all zeros.
	 *
	 * Rather than write zeros and then /mix/ zeros, this enables
	 * an optimization that says, "skip this port, I'm all zeros."
	 */
	virtual bool zero_flag() = 0;

	/**
	 * Set the zero_flag() state.
	 */
	virtual void set_zero_flag(bool zero_is_true) = 0;

	/**
	 * Writes zeros to the beginning of the buffer.  Does not set zero().
	 *
	 * Writes zeros to every data point in the buffer from 0 to
	 * nframes-1.  If nframes is -1, it will write the entire
	 * buffer.
	 */
	virtual void write_zeros(uint32_t nframes = -1) = 0;

    };

} // namespace Tritium

#endif // TRITIUM_AUDIOPORT_HPP
