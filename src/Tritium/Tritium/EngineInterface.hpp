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
#ifndef TRITIUM_ENGINEINTERFACE_HPP
#define TRITIUM_ENGINEINTERFACE_HPP

#include <Tritium/memory.hpp>

namespace Tritium
{
    class Preferences;
    class Sampler;
    class Mixer;
    class Effects;

    /**
     * \brief Abstract interface for a basic audio engine
     *
     */
    class EngineInterface
    {
    public:
	virtual ~EngineInterface() {}

	////////////////////////////////////////////////
	// These must always return a valid instance
	////////////////////////////////////////////////
	virtual T<Preferences>::shared_ptr get_preferences() = 0;
	virtual T<Sampler>::shared_ptr get_sampler() = 0;
	virtual T<Mixer>::shared_ptr get_mixer() = 0;

	////////////////////////////////////////////////
	// These may return null pointers
	////////////////////////////////////////////////
	virtual T<Effects>::shared_ptr get_effects() = 0;

    };

} // namespace Tritium

#endif // TRITIUM_ENGINEINTERFACE_HPP
