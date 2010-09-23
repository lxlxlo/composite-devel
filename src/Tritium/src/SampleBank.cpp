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

#include <Tritium/SampleBank.hpp>
#include <QtCore/QMutexLocker>
#include <stdexcept>

namespace Tritium
{
    SampleBank::SampleBank() :
	_next_key(1)
    {
    }

    SampleBank::~SampleBank()
    {
    }

    size_t SampleBank::size() const
    {
	return _data.size();
    }

    SampleBank::value_t SampleBank::pop(SampleBank::key_t key)
    {
	iterator pos;
	QMutexLocker lk( &_data_mutex );
	pos = _data.find(key);
	if( pos == _data.end() ) {
	    throw std::out_of_range("Invalid key to SampleBank::pop()");
	}
	SampleBank::value_t val = _data[key];
	_data.erase(pos);
	return val;
    }

    SampleBank::key_t SampleBank::push(SampleBank::value_t sample)
    {
	iterator pos;
	QMutexLocker lk( &_data_mutex );
	for( pos = _data.begin() ; pos != _data.end() ; ++pos ) {
	    if( sample == pos->second ) break;
	}
	if( pos == _data.end() ) {
	    // New element
	    key_t key = _next_key.fetchAndAddAcquire(1);
	    assert( key != NO_KEY );
	    _data[key] = sample;
	    return key;
	}
	// Existing element
	return pos->first;
    }

    SampleBank::value_t SampleBank::get(SampleBank::key_t key)
    {
	SampleBank::value_t rv;
	SampleBank::iterator pos;
	QMutexLocker lk( &_data_mutex );
	pos = _data.find(key);
	if( pos == _data.end() ) {
	    throw std::out_of_range("Invalid key to SampleBank::get()");
	} else {
	    rv = pos->second;
	}
	return rv;
    }

    SampleBank::value_t SampleBank::find(SampleBank::key_t key)
    {
	// Same code as get(), but no exception.
	SampleBank::value_t rv;
	SampleBank::iterator pos;
	QMutexLocker lk( &_data_mutex );
	pos = _data.find(key);
	if( pos != _data.end() ) rv = pos->second;
	return rv;
    }

    SampleBank::key_t SampleBank::find_key(SampleBank::value_t sample)
    {
	iterator pos;
	QMutexLocker lk( &_data_mutex );
	for( pos = _data.begin() ; pos != _data.end() ; ++pos ) {
	    if( sample == pos->second ) break;
	}
	if( pos == _data.end() ) {
	    return 0;
	}
	return pos->first;
    }

    void SampleBank::clear()
    {
	QMutexLocker lk( &_data_mutex );
	_data.clear();
    }

    SampleBank::iterator SampleBank::begin()
    {
	iterator pos;
	QMutexLocker lk( &_data_mutex );
	pos = _data.begin();
	return pos;
    }

    SampleBank::const_iterator SampleBank::begin() const
    {
	const_iterator pos;
	QMutexLocker lk( &_data_mutex );
	pos = _data.begin();
	return pos;
    }

    SampleBank::iterator SampleBank::end()
    {
	iterator pos;
	QMutexLocker lk( &_data_mutex );
	pos = _data.end();
	return pos;
    }

    SampleBank::const_iterator SampleBank::end() const
    {
	const_iterator pos;
	QMutexLocker lk( &_data_mutex );
	pos = _data.end();
	return pos;
    }

} // namespace Tritium
