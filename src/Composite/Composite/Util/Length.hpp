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
#ifndef COMPOSITE_UTIL_LENGTH_HPP
#define COMPOSITE_UTIL_LENGTH_HPP

#include <cassert>

namespace Composite
{
namespace Looks
{
    /**
     * \brief Manages a unit of length, with conversions.
     */
    class Length
    {
    public:
	Length(float inches = 0.0, float dpi = 96.0) :
	    _inches(inches),
	    _dpi(dpi)
	    {}

	float inches() {
	    return _inches;
	}

	void inches(float val) {
	    _inches = val;
	}

	float dpi() {
	    return _dpi;
	}
	void dpi(float val) {
	    assert(val >= 1e-5f);
	    _dpi = val;
	}

	float pixels() {
	    return _inches * _dpi;
	}

	void pixels(float val) {
	    assert(_dpi >= 1e-5f);
	    _inches = val / _dpi;
	}

	float mm() {
	    return inches() * 25.4f;
	}

	void mm(float val) {
	    inches( val / 25.4f );
	}

    private:
	float _inches;
	float _dpi;
    };

} // namespace Looks
} // namespace Composite

#endif // COMPOSITE_UTIL_LENGTH_HPP
