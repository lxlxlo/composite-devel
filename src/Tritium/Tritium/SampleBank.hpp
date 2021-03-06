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

#ifndef TRITIUM_SAMPLEBANK_HPP
#define TRITIUM_SAMPLEBANK_HPP

#include <Tritium/globals.hpp>
#include <Tritium/memory.hpp>
#include <QtCore/QMutex>
#include <QtCore/QAtomicInt>
#include <map>
#include <stdexcept>

namespace Tritium
{

class Sample;

/**
 * \brief Central container for all audio samples.
 *
 * This is a central clearing-house for audio samples.  The intention
 * is that references to a sample can be passed around reliably as an
 * integer.  However, end-users of the samples need to maintain a copy
 * of the actual sample pointer to keep it from being garbage
 * collected.
 */
class SampleBank
{
public:
    typedef T<Sample>::shared_ptr value_t;
    typedef int key_t;
    typedef std::map<key_t, value_t> container_t;
    typedef container_t::iterator iterator;
    typedef container_t::const_iterator const_iterator;
    typedef container_t::value_type iterator_value_type;

    SampleBank();
    virtual ~SampleBank();

    // Misc manip
    size_t size() const;
    value_t pop(key_t key);
    key_t push(value_t sample);
    value_t get(key_t key); // throws std::out_of_range
    value_t find(key_t key); // no throw
    key_t find_key(value_t sample);
    void clear();

    // Iterators
    iterator begin();
    const_iterator begin() const;
    iterator end();
    const_iterator end() const;

    void garbage_collect();

private:

    enum { NO_KEY = 0 };

    mutable QAtomicInt _next_key;
    mutable QMutex _data_mutex;
    container_t _data;

}; // class Sample

} // namespace Tritium

#endif // TRITIUM_SAMPLEBANK_HPP
