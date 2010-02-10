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
#ifndef TRITIUM_AUDIOPORTIMPL_HPP
#define TRITIUM_AUDIOPORTIMPL_HPP

#include <Tritium/AudioPort.hpp>
#include <Tritium/memory.hpp>
#include <Tritium/globals.hpp>
#include <vector>

namespace Tritium
{
    /**
     * An audio port.
     */
    class AudioPortImpl : public AudioPort
    {
    public:
	AudioPortImpl(
	    AudioPort::type_t type = AudioPort::MONO,
	    uint32_t max_size = MAX_BUFFER_SIZE
	    );
	virtual ~AudioPortImpl();

	virtual Float* get_buffer(unsigned chan = 0);
	virtual uint32_t size();
	virtual AudioPort::type_t type();
	virtual bool zero_flag();
	virtual void set_zero_flag(bool zero_is_true);
	virtual void write_zeros(uint32_t nframes);

    private:
	std::vector<Float> _left;
	std::vector<Float> _right;
	bool _zero;
    };

} // namespace Tritium

#endif // TRITIUM_AUDIOPORTIMPL_HPP
