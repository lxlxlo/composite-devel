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
#include <deque>
#include <stdexcept>

namespace Tritium
{
    /**
     * \brief Constructs an empty SampleBank.
     */
    SampleBank::SampleBank() :
	_next_key(1)
    {
    }

    /**
     * \brief Destroys SampleBank.
     *
     * It unrefs all of the samples.  Whether or not they are deleted
     * depends on their internal reference counts.
     */

    SampleBank::~SampleBank()
    {
    }

    /**
     * \brief Returns the number of elements in the continaer
     *
     * Complexity: O(N)
     * RT-Safe: yes
     *
     * \return Number of elements in container
     */
    size_t SampleBank::size() const
    {
	return _data.size();
    }

    /**
     * \brief Removes a sample from the container.
     *
     * Complexity: O(log(N))
     * RT-Safe: no
     *
     * \param key The key for the sample that should be removed.
     * \return Pointer to the sample removed.
     * \throw std::out_of_range() if key doesn't match any.
     */
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

    /**
     * \brief Adds a sample to the bank.
     *
     * Complexity: O(log(N))
     * RT-Safe: no
     *
     * \param sample The sample to add to the bank.
     * \return On success, returns the key for the sample.  On
     *         failure returns 0.
     */
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

    /**
     * \brief Retrieves the sample by key.
     *
     * Complexity: O(log(N))
     * RT-Safe: yes
     *
     * \param key The key that was assigned to the sample.
     * \return Pointer to the sample assigned to key.
     * \throw std::out_of_range() if key doesn't match any.
     */
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

    /**
     * \brief Retrieves the sample by key, if it exists.
     *
     * Complexity: O(log(N))
     * RT-Safe: yes
     *
     * \param key The key to search for.
     * \return Pointer to sample assigned to key, or null pointer.
     * \throw none
     */
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

    /**
     * \brief Finds the key for the given sample
     *
     * Complexity: O(N)
     * RT-Safe: yes
     *
     * \param sample The sample whose key we would like to find.
     * \return Either the key, or 0 if the sample isn't found.
     */
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

    /**
     * \brief Erases everything from the bank.
     *
     * Complexity: O(N)
     * RT-Safe: no
     */
    void SampleBank::clear()
    {
	QMutexLocker lk( &_data_mutex );
	_data.clear();
    }

    /**
     * \brief Returns read-write iterator that points to the first sample.
     *
     * RT-Safe: yes
     *
     * \returns iterator with value type std::pair<key_t, value_t>
     */
    SampleBank::iterator SampleBank::begin()
    {
	iterator pos;
	QMutexLocker lk( &_data_mutex );
	pos = _data.begin();
	return pos;
    }

    /**
     * \brief Returns read-only iterator that points to the first sample.
     *
     * RT-Safe: yes
     *
     * \returns iterator with value type const std::pair<key_t, value_t>
     */
    SampleBank::const_iterator SampleBank::begin() const
    {
	const_iterator pos;
	QMutexLocker lk( &_data_mutex );
	pos = _data.begin();
	return pos;
    }

    /**
     * \brief Returns read-write iterator that points to one past the last sample.
     *
     * RT-Safe: yes
     *
     * \returns iterator with value type std::pair<key_t, value_t>
     */
    SampleBank::iterator SampleBank::end()
    {
	iterator pos;
	QMutexLocker lk( &_data_mutex );
	pos = _data.end();
	return pos;
    }

    /**
     * \brief Returns read-only iterator that points to one past the last sample.
     *
     * RT-Safe: yes
     *
     * \returns iterator with value type std::pair<key_t, value_t>
     */
    SampleBank::const_iterator SampleBank::end() const
    {
	const_iterator pos;
	QMutexLocker lk( &_data_mutex );
	pos = _data.end();
	return pos;
    }

    /**
     * \brief Delete all objects not being used
     *
     * RT-Safe: no
     */
    void SampleBank::garbage_collect()
    {
	std::deque< value_t > death_row;
	iterator it, tmp;

	it = begin();
	while( it != end() ) {
	    if( (*it).second.unique() ) {
		death_row.push_back( (*it).second );
		tmp = it;
		++it;
		_data.erase(tmp);
	    } else {
		++it;
	    }
	}
    }

} // namespace Tritium
