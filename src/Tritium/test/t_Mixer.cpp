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

/**
 * t_Mixer.cpp
 *
 */

#include <Tritium/Mixer.hpp>
#include <Tritium/AudioPort.hpp>
#include <cstring>
#include <QString>

// CHANGE THIS TO MATCH YOUR FILE:
#define THIS_NAMESPACE t_Mixer
#include "test_macros.hpp"
#include "test_config.hpp"

using namespace Tritium;

namespace THIS_NAMESPACE
{

    struct Fixture
    {
	// SETUP AND TEARDOWN OBJECTS FOR YOUR TESTS.
	T<Mixer>::auto_ptr m;

	Fixture() {
	    m.reset( new Mixer() );
	}
	~Fixture() {}
    };

} // namespace THIS_NAMESPACE

TEST_BEGIN( Fixture );

TEST_CASE( 010_defaults )
{
    BOOST_REQUIRE( m.get() );
    CK( m->count() == 0 );
    m->pre_process();

    float left[4096], right[4096];
    memset(left, ~0, 4096 * sizeof(float));
    memset(right, ~0, 4096 * sizeof(float));
    for(size_t k=0 ; k<4096 ; ++k) {
	BOOST_REQUIRE(left[k] != 0.0f);
	BOOST_REQUIRE(right[k] != 0.0f);
    }

    m->mix_send_return(4096);
    m->mix_down(4096, left, right);

    for(size_t k=0 ; k<4096 ; ++k) {
	CK(left[k] == 0.0f);
	CK(right[k] == 0.0f);
    }
}

TEST_CASE( 020_simple_mix )
{
    T<AudioPort>::shared_ptr mono, stereo;

    mono = m->allocate_port("mono", AudioPort::OUTPUT, AudioPort::MONO);
    stereo = m->allocate_port("stereo", AudioPort::OUTPUT, AudioPort::STEREO);

    m->pre_process();

    float* buf;
    size_t k, N=1024;
    buf = mono->get_buffer();
    BOOST_REQUIRE( N <= mono->size() );
    for(k=0 ; k<N ; ++k) {
	buf[k] = 0.1f;
    }

    buf = stereo->get_buffer(0);
    BOOST_REQUIRE( N <= stereo->size() );
    for(k=0 ; k<N ; ++k) {
	buf[k] = 0.2f;
    }

    buf = stereo->get_buffer(1);
    for(k=0 ; k<N ; ++k) {
	buf[k] = 0.3f;
    }

    m->mix_send_return(4096);
    float left[1024], right[1024];
    float Lv, Rv;
    Lv = .1f + .2f;
    Rv = .1f + .3f;
    m->mix_down(N, left, right);

    for(k=0 ; k<N ; ++k) {
	CK( left[k] == Lv );
	CK( right[k] == Rv );
    }

    m->release_port(mono);
    m->release_port(stereo);

    CK(mono.use_count() == 1);
    CK(stereo.use_count() == 1);
}

TEST_END()
