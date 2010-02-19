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

    float left[1024], right[1024];
    float Lv, Rv;
    Lv = .1f + .2f;
    Rv = .1f + .3f;
    m->mix_send_return(N);
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

    float left[1024], right[1024];
    float Lv, Rv, Lvm, Lvs, Rvm, Rvs;
    Lvm = .1f * 2.0f * .25f / .75f; // Left mono
    Rvm = .1f * 2.0f; // Right mono
    Lvs = 0.3f * 0.25f; // Left stereo
    Rvs = 0.2f * 0.25f; // Right stereo
    Lv = Lvm + Lvs;
    Rv = Rvm + Rvs;
    m->mix_send_return(N);
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

TEST_CASE( 040_channel_methods )
{
    Mixer::Channel c_a;
    T<AudioPort>::shared_ptr p_a = m->allocate_port("tmp-0");

    c_a.port() = p_a;
    c_a.gain(0.73914f);
    c_a.pan(0.2519f);
    c_a.pan_R(0.99134f);
    BOOST_REQUIRE(c_a.send_count() >= 4);
    c_a.send_gain(0, 0.0f);
    c_a.send_gain(1, 0.1f);
    c_a.send_gain(2, 0.2f);
    c_a.send_gain(3, 0.3f);

    CK(c_a.port() == p_a);
    CK(c_a.gain() == 0.73914f);
    CK(c_a.pan() == 0.2519f);
    CK(c_a.pan_L() == 0.2519f);
    CK(c_a.pan_R() == 0.99134f);
    CK(c_a.send_gain(0) == 0.0f);
    CK(c_a.send_gain(1) == 0.1f);
    CK(c_a.send_gain(2) == 0.2f);
    CK(c_a.send_gain(3) == 0.3f);

    Mixer::Channel c_b(c_a);
    CK(c_b.port() == p_a);
    CK(c_b.gain() == 0.73914f);
    CK(c_b.pan() == 0.2519f);
    CK(c_b.pan_L() == 0.2519f);
    CK(c_b.pan_R() == 0.99134f);
    CK(c_b.send_gain(0) == 0.0f);
    CK(c_b.send_gain(1) == 0.1f);
    CK(c_b.send_gain(2) == 0.2f);
    CK(c_b.send_gain(3) == 0.3f);

    Mixer::Channel c_c;
    T<AudioPort>::shared_ptr p_c = m->allocate_port("tmp-1");
    CK( p_c != p_a );
    c_c.port() = p_c;
    CK( c_c.port() != p_a );
    CK( c_c.port() == p_c );
    c_c.gain(1.125f);
    CK( c_c.gain() == 1.125f );
    c_c.match_props(c_a);
    CK(c_c.port() != p_a);
    CK(c_c.port() == p_c);
    CK(c_c.gain() == 0.73914f);
    CK(c_c.pan() == 0.2519f);
    CK(c_c.pan_L() == 0.2519f);
    CK(c_c.pan_R() == 0.99134f);
    CK(c_c.send_gain(0) == 0.0f);
    CK(c_c.send_gain(1) == 0.1f);
    CK(c_c.send_gain(2) == 0.2f);
    CK(c_c.send_gain(3) == 0.3f);

}

TEST_END()
