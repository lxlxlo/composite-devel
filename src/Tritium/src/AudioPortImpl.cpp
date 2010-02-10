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

#include "AudioPortImpl.hpp"
#include <algorithm>

using namespace Tritium;

AudioPortImpl::AudioPortImpl(
    AudioPort::type_t type,
    uint32_t max_size
    ) :
    _left(max_size),
    _right(0),
    _zero(true)
{
    if( type == AudioPort::STEREO ) {
	_right.resize(max_size);
    }
}

AudioPortImpl::~AudioPortImpl()
{
}

AudioPort::Float* AudioPortImpl::get_buffer(unsigned chan)
{
    if(chan == 0) {
	return &_left.front();
    } else if (chan == 1) {
	return &_right.front();
    } else {
	return 0;
    }
}

uint32_t AudioPortImpl::size()
{
    return _left.size();
}

AudioPort::type_t AudioPortImpl::type()
{
    if(_right.size() != 0) {
	return AudioPort::STEREO;
    } else {
	return AudioPort::MONO;
    }
}

bool AudioPortImpl::zero_flag()
{
    return _zero;
}

void AudioPortImpl::set_zero_flag(bool zero_is_true)
{
    _zero = zero_is_true;
}

void AudioPortImpl::write_zeros(uint32_t nframes)
{
    if(nframes == -1 || nframes > _left.size()) {
	std::fill(_left.begin(), _left.end(), 0.0f);
	std::fill(_right.begin(), _right.end(), 0.0f);
    } else {
	std::fill(&_left[0], &_left[nframes], 0.0f);
	if(!_right.empty()) {
	    std::fill(&_right[0], &_right[nframes], 0.0f);
	}
    }
}
