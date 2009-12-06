/*
 * Copyright(c) 2002-2008 by Alex >Comix< Cominu [comix@users.sourceforge.net]
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

#include <Tritium/AudioEngine.hpp>

#include <Tritium/fx/Effects.hpp>
#include <Tritium/Sampler.hpp>
#include <Tritium/Logger.hpp>

#include <cassert>

namespace Tritium
{

AudioEngine::AudioEngine()
		: __sampler( NULL )
{
	INFOLOG( "INIT" );

	__sampler = new Sampler;
}



AudioEngine::~AudioEngine()
{
	INFOLOG( "DESTROY" );
	delete __sampler;
}



Sampler* AudioEngine::get_sampler()
{
	assert(__sampler);
	return __sampler;
}



void AudioEngine::lock( const char* file, unsigned int line, const char* function )
{
	__engine_mutex.lock();
	__locker.file = file;
	__locker.line = line;
	__locker.function = function;
}



bool AudioEngine::try_lock( const char* file, unsigned int line, const char* function )
{
	bool locked = __engine_mutex.tryLock();
	if ( ! locked ) {
		// Lock not obtained
		return false;
	}
	__locker.file = file;
	__locker.line = line;
	__locker.function = function;
	return true;
}



void AudioEngine::unlock()
{
	// Leave "__locker" dirty.
	__engine_mutex.unlock();
}


}; // namespace Tritium
