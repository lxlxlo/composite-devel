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

namespace Tritium
{

class Sample;

/**
 * Central container for all audio samples.
 *
 */
class SampleBank
{
public:
    typedef T<Sample>::shared_ptr value_t;
    typedef int key_t;
    typedef std::map<key_t, value_t> container_t;
    typedef container_t::iterator iterator;
    typedef container_t::const_iterator const_iterator;

    SampleBank();
    virtual ~SampleBank();

    /* Misc access */
    size_t size() const;
    value_t pop(key_t key);
    key_t push(value_t sample); // key_t == 0 is an error
    value_t get(key_t key);
    key_t find_key(value_t sample);
    void clear();

    /* Iterator access
     *
     * Note that iterators are std::map<key_t, value_t>, which means
     * that the values iterated over are std::pair<key_t, value_t>.
     */
    iterator begin();
    const_iterator begin() const;
    iterator end();
    const_iterator end() const;

private:
    mutable QAtomicInt _next_key;
    mutable QMutex _data_mutex;
    container_t _data;

}; // class Sample

} // namespace Tritium

#endif // TRITIUM_SAMPLEBANK_HPP
