/*
 * Copyright(c) 2010 by Gabriel M. Beddingfield <gabriel@teuton.org>
 *
 * This file is part of Composite
 *
 * Composite is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Composite is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY, without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */
#ifndef COMPOSITE_UTIL_PROPERTY_HPP
#define COMPOSITE_UTIL_PROPERTY_HPP

#include <cassert>

namespace Composite
{
namespace Looks
{
    /**
     * \brief Provides a Read/Write class property
     */
    template <typename T>
    class Property
    {
    public:
	Property(T v = T()) :
	    _d(v)
	    {}

	const T& operator()() const { return _d; }
	void operator()(const T& v) { _d = v; }

    private:
	T _d;
    };

} // namespace Util
} // namespace Composite

#endif // COMPOSITE_UTIL_PROPERTY_HPP
