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
 * t_AudioPort.cpp
 *
 * Tests AudioPort and AudioPortImpl
 */

#include <Tritium/AudioPort.hpp>
#include "../src/AudioPortImpl.hpp"
#include <Tritium/globals.hpp>
#include <deque>
#include <cmath>

// CHANGE THIS TO MATCH YOUR FILE:
#define THIS_NAMESPACE t_AudioPort
#include "test_macros.hpp"
#include "test_config.hpp"

namespace Tritium {}  // please compile! :-)

using namespace Tritium;

namespace THIS_NAMESPACE
{
    bool feq(float a, float b) {
	const float eps = 1.0e-45;
	return fabs(a-b) < eps;
    }

    struct Fixture
    {
	// port[0] == default buffer size
	// port[1] == 128 frame buffer size
	T<AudioPort>::shared_ptr mono[2];
	T<AudioPort>::shared_ptr stereo[2];

	Fixture() {
	    T<AudioPortImpl>::shared_ptr tmp;
	    tmp.reset( new AudioPortImpl() );
	    mono[0] = boost::dynamic_pointer_cast<AudioPort, AudioPortImpl>(tmp);
	    tmp.reset( new AudioPortImpl(AudioPort::MONO, 128) );
	    mono[1] = boost::dynamic_pointer_cast<AudioPort, AudioPortImpl>(tmp);
	    tmp.reset( new AudioPortImpl(AudioPort::STEREO) );
	    stereo[0] = boost::dynamic_pointer_cast<AudioPort, AudioPortImpl>(tmp);
	    tmp.reset( new AudioPortImpl(AudioPort::STEREO, 128) );
	    stereo[1] = boost::dynamic_pointer_cast<AudioPort, AudioPortImpl>(tmp);
	}
	~Fixture() {}
    };

} // namespace THIS_NAMESPACE

TEST_BEGIN( Fixture );

TEST_CASE( 010_defaults )
{
    CK( mono[0]->get_buffer() != 0 );
    CK( mono[0]->get_buffer(1) == 0 );
    CK( mono[0]->size() == MAX_BUFFER_SIZE );
    CK( mono[0]->type() == AudioPort::MONO );
    CK( mono[0]->zero_flag() == true );

    CK( mono[1]->get_buffer() != 0 );
    CK( mono[1]->get_buffer(1) == 0 );
    CK( mono[1]->size() == 128 );
    CK( mono[1]->type() == AudioPort::MONO );
    CK( mono[1]->zero_flag() == true );

    CK( stereo[0]->get_buffer() != 0 );
    CK( stereo[0]->get_buffer(1) != 0 );
    CK( stereo[0]->size() == MAX_BUFFER_SIZE );
    CK( stereo[0]->type() == AudioPort::STEREO );
    CK( stereo[0]->zero_flag() == true );

    CK( stereo[1]->get_buffer() != 0 );
    CK( stereo[1]->get_buffer(1) != 0 );
    CK( stereo[1]->size() == 128 );
    CK( stereo[1]->type() == AudioPort::STEREO );
    CK( stereo[1]->zero_flag() == true );
}

TEST_CASE( 020_write_read )
{
    typedef T<AudioPort>::shared_ptr port_ref_t;
    typedef std::deque< port_ref_t > list_t;
    list_t all;

    all.push_back( mono[0] );
    all.push_back( mono[1] );
    all.push_back( stereo[0] );
    all.push_back( stereo[1] );

    // Write the data.
    list_t::iterator it;
    float dt = 3.1415;
    float val = 0.0f;

    for(it=all.begin() ; it!=all.end() ; ++it) {
	AudioPort& t = *((*it).get());
	float* left = t.get_buffer();
	float* right = t.get_buffer(1);
	uint32_t k;
	for(k=0; k<t.size() ; ++k) {
	    left[k] = val;
	    val += dt;
	    if(right) {
		right[k] = (1.0f/left[k]);
	    }
	}
    }

    // Read the data
    val = 0.0f;
    for(it=all.begin() ; it!=all.end() ; ++it) {
	AudioPort& t = *((*it).get());
	float* left = t.get_buffer();
	float* right = t.get_buffer(1);
	uint32_t k;
	for(k=0; k<t.size() ; ++k) {
	    CK( feq(left[k], val) );
	    val += dt;
	    if(right) {
		CK( feq(right[k], (1.0f/left[k])) );
	    }
	}
    }
}

TEST_CASE( 030_write_then_zero )
{
    typedef T<AudioPort>::shared_ptr port_ref_t;
    typedef std::deque< port_ref_t > list_t;
    list_t all;

    all.push_back( mono[0] );
    all.push_back( mono[1] );
    all.push_back( stereo[0] );
    all.push_back( stereo[1] );

    // Write the data.
    list_t::iterator it;
    float dt = 3.1415;
    float val = 0.0f;

    for(it=all.begin() ; it!=all.end() ; ++it) {
	AudioPort& t = *((*it).get());
	float* left = t.get_buffer();
	float* right = t.get_buffer(1);
	uint32_t k;
	for(k=0; k<t.size() ; ++k) {
	    left[k] = val;
	    val += dt;
	    if(right) {
		right[k] = (1.0f/left[k]);
	    }
	}
    }


    for(it=all.begin() ; it!=all.end() ; ++it) {
	AudioPort& t = *((*it).get());
	t.write_zeros();
    }


    // Read the data
    val = 0.0f;
    for(it=all.begin() ; it!=all.end() ; ++it) {
	AudioPort& t = *((*it).get());
	float* left = t.get_buffer();
	float* right = t.get_buffer(1);
	uint32_t k;
	for(k=0; k<t.size() ; ++k) {
	    CK( left[k] == 0.0f );
	    val += dt;
	    if(right) {
		CK( right[k] == 0.0f );
	    }
	}
    }
}

TEST_CASE( 040_write_then_zero_64 )
{
    typedef T<AudioPort>::shared_ptr port_ref_t;
    typedef std::deque< port_ref_t > list_t;
    list_t all;

    all.push_back( mono[0] );
    all.push_back( mono[1] );
    all.push_back( stereo[0] );
    all.push_back( stereo[1] );

    // Write the data.
    list_t::iterator it;
    float dt = 3.1415;
    float val = 0.0f;

    for(it=all.begin() ; it!=all.end() ; ++it) {
	AudioPort& t = *((*it).get());
	float* left = t.get_buffer();
	float* right = t.get_buffer(1);
	uint32_t k;
	for(k=0; k<t.size() ; ++k) {
	    left[k] = val;
	    val += dt;
	    if(right) {
		right[k] = (1.0f/left[k]);
	    }
	}
    }


    for(it=all.begin() ; it!=all.end() ; ++it) {
	AudioPort& t = *((*it).get());
	t.write_zeros(64);
    }


    // Read the data
    val = 0.0f;
    for(it=all.begin() ; it!=all.end() ; ++it) {
	AudioPort& t = *((*it).get());
	float* left = t.get_buffer();
	float* right = t.get_buffer(1);
	uint32_t k;
	for(k=0; k<64 ; ++k) {
	    CK( left[k] == 0.0f );
	    val += dt;
	    if(right) {
		CK( right[k] == 0.0f );
	    }
	}
	for(k=64; k<t.size(); ++k) {
	    CK( feq(left[k], val) );
	    val += dt;
	    if(right) {
		CK( feq(right[k], (1.0f/left[k])) );
	    }
	}
    }
}

TEST_CASE( 050_write_then_zero_size )
{
    typedef T<AudioPort>::shared_ptr port_ref_t;
    typedef std::deque< port_ref_t > list_t;
    list_t all;

    all.push_back( mono[0] );
    all.push_back( mono[1] );
    all.push_back( stereo[0] );
    all.push_back( stereo[1] );

    // Write the data.
    list_t::iterator it;
    float dt = 3.1415;
    float val = 0.0f;

    for(it=all.begin() ; it!=all.end() ; ++it) {
	AudioPort& t = *((*it).get());
	float* left = t.get_buffer();
	float* right = t.get_buffer(1);
	uint32_t k;
	for(k=0; k<t.size() ; ++k) {
	    left[k] = val;
	    val += dt;
	    if(right) {
		right[k] = (1.0f/left[k]);
	    }
	}
    }


    for(it=all.begin() ; it!=all.end() ; ++it) {
	AudioPort& t = *((*it).get());
	t.write_zeros( t.size() );
    }


    // Read the data
    val = 0.0f;
    for(it=all.begin() ; it!=all.end() ; ++it) {
	AudioPort& t = *((*it).get());
	float* left = t.get_buffer();
	float* right = t.get_buffer(1);
	uint32_t k;
	for(k=0; k<t.size() ; ++k) {
	    CK( left[k] == 0.0f );
	    val += dt;
	    if(right) {
		CK( right[k] == 0.0f );
	    }
	}
    }
}

TEST_END()
