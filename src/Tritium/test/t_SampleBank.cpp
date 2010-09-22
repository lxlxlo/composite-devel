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

TEST_END()
