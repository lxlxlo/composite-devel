/*
 * Copyright(c) 2002-2008 by Alex >Comix< Cominu [comix@users.sourceforge.net]
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

#include <Tritium/SimpleSample.hpp>
#include <Tritium/memory.hpp>
#include <stdexcept>
#include <cassert>

using namespace std;

namespace Tritium
{
    SimpleSample::SimpleSample(
	int channels,
	unsigned frames,
	float sample_rate,
	const QUrl& url
	) :
	_size(frames),
	_sample_rate(sample_rate),
	_memory( channels * (1 + _size), 0.0f ),
	_url(url)
    {
	if(channels <= 0)
	    throw std::range_error("Zero or negative channel count given to SimpleSample");

	float *ptr = &_memory[0];
	while( channels-- ) {
	    _chan_ptrs.push_back(ptr);
	    ptr += frames;
	}
    }

    SimpleSample::~SimpleSample()
    {
	std::vector<float*>::iterator it;
	for( it = _chan_ptrs.begin() ; it != _chan_ptrs.end() ; ++it ) {
	    (*it) = 0;
	}
    }

    T<SimpleSample>::shared_ptr SimpleSample::empty_sample(int chans) {
	T<SimpleSample>::shared_ptr ptr( new SimpleSample(chans, 0, 44100.0, QUrl()) );
	return ptr;
    }

} // namespace Tritium
