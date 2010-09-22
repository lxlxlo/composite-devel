/*
 * Copyright(c) 2009,2010 by Gabriel M. Beddingfield <gabriel@teuton.org>
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
 * t_SampleBank.cpp
 *
  */

#include <Tritium/SampleBank.hpp>
#include <deque>
#include <algorithm>

// CHANGE THIS TO MATCH YOUR FILE:
#define THIS_NAMESPACE t_TestTemplate
#include "test_macros.hpp"
#include "test_config.hpp"

namespace Tritium
{
    class Sample // A Mock object
    {
    public:
	unsigned val;
    };

}

using namespace Tritium;

namespace THIS_NAMESPACE
{

    struct Fixture
    {
	// SETUP AND TEARDOWN OBJECTS FOR YOUR TESTS.
	SampleBank a;

	Fixture() {}
	~Fixture() {
	}
    };

} // namespace THIS_NAMESPACE

TEST_BEGIN( Fixture );

TEST_CASE( 010_defaults )
{
    CK( a.size() == 0 );
    CK( a.begin() == a.end() );
}

TEST_CASE( 020_push_pop )
{
    std::deque< SampleBank::key_t > keys;
    SampleBank::key_t k;
    unsigned j, N = 64;

    CK( a.size() == 0 );
    for( j=0 ; j<N ; ++j ) {
	k = a.push( T<Sample>::shared_ptr( new Sample ) );
	keys.push_back(k);
	CK( a.size() == j+1 );
	CK( keys.size() == j+1 );
    }
    CK( a.size() == N );
    CK( keys.size() == N );

    a.clear();
    CK( a.size() == 0 );
    keys.clear();

    for( j=0 ; j<N ; ++j ) {
	k = a.push( T<Sample>::shared_ptr( new Sample ) );
	keys.push_back(k);
	CK( a.size() == j+1 );
	CK( keys.size() == j+1 );
    }
    CK( a.size() == N );
    CK( keys.size() == N );

    std::deque< SampleBank::key_t >::iterator ki;
    --N;
    for( ki = keys.begin() ; ki != keys.end() ; ++ki, --N ) {
	a.pop( *ki );
	CK( a.size() == N );
    }
}

TEST_CASE( 020_value_integrity )
{
    unsigned vals[] = { 0x7a7b42c3, 0x0ef40b83, 0x1d8e9844, 0x42cf363a,
			0x68c5a0aa, 0x4bb233c6, 0x0ae29622, 0x044ef6f6,
			0x41ab4d45, 0x6c08187f, 0x67a9f64d, 0x65a7b89b,
			0x0c104bb0, 0x23f35304, 0x3ed9fc94, 0x3665d570,
			0xb92cbe2d, 0xf7da0f7e, 0x40c7ced4, 0x2f455865,
			0x1ea63252, 0x46bff81c, 0x646e40ca, 0xd3995c53,
			0x0f72bdbb, 0xe4703cdf, 0xc4ed168e, 0xb1584216,
			0xff03505c, 0x48cbd37d, 0xae7eabfe, 0xbea37bc8,
			0x0 };

    // Create a bunch of samples
    typedef std::deque< T<Sample>::shared_ptr > sample_array_t;
    sample_array_t sample_array;

    unsigned *vi;
    for( vi = vals ; *vi ; ++vi ) {
	T<Sample>::shared_ptr tmp( new Sample );
	tmp->val = *vi;
	sample_array.push_back(tmp);
    }
    CK( sample_array.size() == ((sizeof(vals)-1)/sizeof(unsigned)) );

    // Add samples to the bank and track the keys
    typedef std::deque< SampleBank::key_t > key_array_t;
    key_array_t key_array;

    sample_array_t::iterator sit;
    for( sit = sample_array.begin() ; sit != sample_array.end() ; ++sit ) {
	SampleBank::key_t key;
	key = a.push( *sit );
	CK( key != 0 );
	CK( std::find(key_array.begin(), key_array.end(), key) == key_array.end() );
	key_array.push_back( key );
	CK( (*sit)->val == a.get(key)->val );
    }

    CK( key_array.size() == sample_array.size() );
    CK( a.size() == sample_array.size() );

    for( sit = sample_array.begin() ; sit != sample_array.end() ; ++sit ) {
	CK( sit->use_count() == 2 );
    }

    // Check values back and forth
    key_array_t::iterator kit;
    for( sit = sample_array.begin(), kit = key_array.begin() ;
	 sit != sample_array.end() ;
	 ++sit, ++kit ) {
	CK( kit != key_array.end() );

	CK( (*kit) == a.find_key(*sit) );
	CK( (*sit) == a.get(*kit) );
    }

    a.clear();
    CK( a.size() == 0 );
    for( sit = sample_array.begin() ; sit != sample_array.end() ; ++sit ) {
	CK( sit->use_count() == 1 );
    }
   
}

TEST_END()
