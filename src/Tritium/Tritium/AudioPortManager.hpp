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
#ifndef TRITIUM_AUDIOPORTMANAGER_HPP
#define TRITIUM_AUDIOPORTMANAGER_HPP

#include <Tritium/AudioPort.hpp>
#include <Tritium/memory.hpp>

namespace Tritium
{
    /**
     * \brief Abstract interface for allocating/deallocating ports.
     *
     */
    class AudioPortManager
    {
    public:
	virtual ~AudioPortManager() {}

	/**
	 * Creates a port that is managed by the AudioPortManager.
	 *
	 * Typical use case is that the AudioPortManager would be a
	 * mixer or a connection manager, whereas the Sampler or a
	 * Synth just needs a port and does not care about
	 * connections.
	 *
	 * This function is not realtime safe.
	 *
	 * \param name Name of the port, human-readable.
	 *
	 * \param in_or_out Whether port is an input (readable) or
	 * output (writeable) port.
	 *
	 * \param type The type of data that the port takes.  E.g. a
	 * Port::AUDIO port will have a buffer of float's.
	 *
	 * \param size A requested size for the buffer.  If -1, it
	 * will be a size suitable for the current audio settings.
	 *
	 * \return Either a null pointer, or a pointer to the
	 * allocated port.
	 */
	virtual T<AudioPort>::shared_ptr allocate_port(
	    const QString& name,
	    AudioPort::flow_t in_or_out = AudioPort::OUTPUT,
	    AudioPort::type_t type = AudioPort::MONO,
	    size_t size = -1
	    ) = 0;

	/**
	 * Officially declare the port de-allocated.
	 */
	virtual void release_port(T<AudioPort>::shared_ptr port) = 0;
    };

} // namespace Tritium

#endif // TRITIUM_AUDIOPORTMANAGER_HPP
