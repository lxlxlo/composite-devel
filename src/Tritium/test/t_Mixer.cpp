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

#include <Tritium/MixerImpl.hpp>
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
	T<MixerImpl>::auto_ptr m;

	Fixture() {
	    m.reset( new MixerImpl() );
	}
	~Fixture() {}
    };

} // namespace THIS_NAMESPACE

TEST_BEGIN( Fixture );

TEST_CASE( 010_defaults )
{
    BOOST_REQUIRE( m.get() );
    CK( m->count() == 0 );
    m->pre_process(4096);

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

    float* buf;
    size_t k, N=1024;

    m->pre_process(N);

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

TEST_CASE( 030_channel_properties )
{
    T<AudioPort>::shared_ptr mono, stereo;

    mono = m->allocate_port("mono", AudioPort::OUTPUT, AudioPort::MONO);
    stereo = m->allocate_port("stereo", AudioPort::OUTPUT, AudioPort::STEREO);

    T<Mixer::Channel>::shared_ptr c_mono = m->channel(0);
    T<Mixer::Channel>::shared_ptr c_stereo = m->channel(1);

    c_mono->gain( 2.0f );
    c_mono->pan( .75f );

    c_stereo->gain( 0.25f );
    c_stereo->pan_L( 1.0f ); // Reverse L/R
    c_stereo->pan_R( 0.0f );

    // These writers are the same as for 020_simple_mix

    float* buf;
    size_t k, N=1024;

    m->pre_process(N);

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

    // End of 020_simple_mix writers

    m->mix_send_return(4096);
    float left[1024], right[1024];
    float Lv, Rv, Lvm, Lvs, Rvm, Rvs;
    Lvm = .1f * 2.0f * .25f / .75f; // Left mono
    Rvm = .1f * 2.0f; // Right mono
    Lvs = 0.3f * 0.25f; // Left stereo
    Rvs = 0.2f * 0.25f; // Right stereo
    Lv = Lvm + Lvs;
    Rv = Rvm + Rvs;
    m->mix_down(N, left, right);

    for(k=0 ; k<N ; ++k) {
	CK( left[k] == Lv );
	CK( right[k] == Rv );
    }

    m->release_port(mono);
    m->release_port(stereo);

    CK(c_mono.use_count() == 1);
    CK(c_stereo.use_count() == 1);
    c_mono.reset();
    c_stereo.reset();
    CK(mono.use_count() == 1);
    CK(stereo.use_count() == 1);
}

TEST_END()
