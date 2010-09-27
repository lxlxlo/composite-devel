/*
 * Copyright (c) 2009 by Gabriel M. Beddingfield <gabriel@teuton.org>
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
#ifndef TRITIUM_MEMORY_HPP
#define TRITIUM_MEMORY_HPP

#include <memory>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <stdint.h>

/**
 * \brief Header file with smart pointers, etc.
 */

namespace Tritium
{
    template <typename X>
    struct T
    {
	typedef std::auto_ptr<X> auto_ptr;
	typedef boost::shared_ptr<X> shared_ptr;
	typedef boost::weak_ptr<X> weak_ptr;
    };

    inline bool not_power_of_2(unsigned val) {
	return ((val-1)&val);
    }

    inline bool is_power_of_2(unsigned val) {
	return ! not_power_of_2(val);
    }

    inline bool not_aligned_N(void *ptr, int N) {
	return (((intptr_t)ptr) % N);
    }

    inline bool is_aligned_N(void *ptr, int N) {
	return ! not_aligned_N(ptr, N);
    }

    inline bool not_aligned_16(void *ptr) {
	return not_aligned_N(ptr, 16);
    }

    inline bool is_aligned_16(void *ptr) {
	return ! not_aligned_16(ptr);
    }

} // namespace Tritium

#endif // TRITIUM_MEMORY_HPP
