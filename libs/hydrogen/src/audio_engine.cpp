/*
 * Hydrogen
 * Copyright(c) 2002-2008 by Alex >Comix< Cominu [comix@users.sourceforge.net]
 *
 * http://www.hydrogen-music.org
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY, without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <hydrogen/audio_engine.h>

#include <hydrogen/fx/Effects.h>
#include <hydrogen/sampler/Sampler.h>

#include <hydrogen/hydrogen.h>	// TODO: remove this line as soon as possible
#include <cassert>

namespace H2Core
{


AudioEngine* AudioEngine::__instance = NULL;


void AudioEngine::create_instance()
{
	if( __instance == 0 ) {
		__instance = new AudioEngine;
	}
}

AudioEngine::AudioEngine()
		: Object( "AudioEngine" )
		, __sampler( NULL )
		, __synth( NULL )
{
	INFOLOG( "INIT" );

	pthread_mutex_init( &__engine_mutex, NULL );

	__sampler = new Sampler;
	__synth = new Synth;

#ifdef LADSPA_SUPPORT
	Effects::get_instance();
#endif

//	Sequencer::get_instance();
}



AudioEngine::~AudioEngine()
{
	INFOLOG( "DESTROY" );
#ifdef LADSPA_SUPPORT
	delete Effects::get_instance();
#endif

//	delete Sequencer::get_instance();
	delete __sampler;
	delete __synth;
}



Sampler* AudioEngine::get_sampler()
{
	assert(__sampler);
	return __sampler;
}




Synth* AudioEngine::get_synth()
{
	assert(__synth);
	return __synth;
}

void AudioEngine::lock( const char* file, unsigned int line, const char* function )
{
	pthread_mutex_lock( &__engine_mutex );
	__LOG_WRAPPER( Logger::AELockTracing,
		       __PRETTY_FUNCTION__,
		       get_class_name(),
		       QString( "locked at %1 (%2:%3)" ).arg(function).arg(file).arg(line)
		);  // Lock obtained.
}



bool AudioEngine::try_lock( const char* file, unsigned int line, const char* function )
{
	int res = pthread_mutex_trylock( &__engine_mutex );
	if ( res != 0 ) {
		__LOG_WRAPPER( Logger::AELockTracing,
			       __PRETTY_FUNCTION__,
			       get_class_name(),
			       QString( "trylock failed for %1 (%2:%3)" ).arg(function).arg(file).arg(line)
			);  // Lock not obtained.
		return false;
	}

	__LOG_WRAPPER( Logger::AELockTracing,
		       __PRETTY_FUNCTION__,
		       get_class_name(),
		       QString( "locked at %1 (%2:%3)" ).arg(function).arg(file).arg(line)
		);  // Lock obtained.

	return true;
}



void AudioEngine::unlock()
{
	const QString msg("Unlocked");

	__LOG_WRAPPER( Logger::AELockTracing,
		       __PRETTY_FUNCTION__,
		       get_class_name(),
		       msg
		);  // Lock released
	pthread_mutex_unlock( &__engine_mutex );
}


}; // namespace H2Core
