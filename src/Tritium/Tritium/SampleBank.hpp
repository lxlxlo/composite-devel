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
    typedef container_t::value_type iterator_value_type;

    SampleBank();
    virtual ~SampleBank();

    /**
     * \brief Returns the number of elements in the continaer
     *
     * Complexity: O(N)
     * RT-Safe: yes
     *
     * \return Number of elements in container
     */
    size_t size() const;

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
    value_t pop(key_t key);

    /**
     * \brief Adds a sample to the bank.
     *
     * Complexity: O(1)
     * RT-Safe: no
     *
     * \param sample The sample to add to the bank.
     * \return On success, returns the key for the sample.  On
     *         failure returns 0.
     */
    key_t push(value_t sample);

    /**
     * \brief Retrieves the sample by key.
     *
     * Complexity: O(1)
     * RT-Safe: yes
     *
     * \param key The key that was assigned to the sample.
     * \return Pointer to the sample assigned to key.
     * \throw std::out_of_range() if key doesn't match any.
     */
    value_t get(key_t key);

    /**
     * \brief Finds the key for the given sample
     *
     * Complexity: O(N)
     * RT-Safe: yes
     *
     * \param sample The sample whose key we would like to find.
     * \return Either the key, or 0 if the sample isn't found.
     */
    key_t find_key(value_t sample);

    /**
     * \brief Erases everything from the bank.
     *
     * Complexity: O(N)
     * RT-Safe: no
     */
    void clear();

    /* \brief Returns read-write iterator that points to the first sample.
     *
     * RT-Safe: yes
     *
     * \returns iterator with value type std::pair<key_t, value_t>
     */
    iterator begin();

    /* \brief Returns read-only iterator that points to the first sample.
     *
     * RT-Safe: yes
     *
     * \returns iterator with value type const std::pair<key_t, value_t>
     */
    const_iterator begin() const;

    /* \brief Returns read-write iterator that points to one past the last sample.
     *
     * RT-Safe: yes
     *
     * \returns iterator with value type std::pair<key_t, value_t>
     */
    iterator end();

    /* \brief Returns read-only iterator that points to one past the last sample.
     *
     * RT-Safe: yes
     *
     * \returns iterator with value type std::pair<key_t, value_t>
     */
    const_iterator end() const;

private:

    enum { NO_KEY = 0 };

    mutable QAtomicInt _next_key;
    mutable QMutex _data_mutex;
    container_t _data;

}; // class Sample

} // namespace Tritium

#endif // TRITIUM_SAMPLEBANK_HPP
