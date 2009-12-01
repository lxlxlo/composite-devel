/*
 * Copyright(c) 2009 by Gabriel M. Beddingfield <gabriel@teuton.org>
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
#ifndef TRITIUM_PATTERNMODELIST_HPP
#define TRITIUM_PATTERNMODELIST_HPP

#include <Tritium/Song.hpp>
#include <vector>
#include <QMutex>

namespace Tritium
{
    /**
     * /brief Internal class for Tritium::Song.
     *
     * Container for a vector that is thread-safe and
     * contains unique values.
     */
    class PatternModeList
    {
    public:
	typedef int value_type;
	typedef std::vector<value_type> list_type;
	typedef list_type::iterator iterator;

	PatternModeList();

	void reserve(size_t size);
	size_t size();
	void add(value_type d);
	void remove(value_type d);
	void clear();

	// Iterator access is inherently not thread-safe.
	// To work around this, lock the mutex to ensure that
	// the vector doesn't change while using the iterator.
	QMutex& get_mutex();  // Warning: Only use this for begin()/end().
                              // If you call any of the other methods, with
	                      // the mutex locked, you will get a deadlock.
	iterator begin();
	iterator end();

    private:
	QMutex __mutex;
	list_type __vec;
    };

} // namespace Tritium

#endif // TRITIUM_PATTERNMODELIST_HPP
