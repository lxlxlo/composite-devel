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
 * t_Presets.cpp
 *
 * This is just the template for a test.  If the template is not
 * executable, it will not be kept up-to-date.
 */

// YOUR INCLUDES HERE
#include <Tritium/Presets.hpp>
#include <cstdlib>
#include <string>

// CHANGE THIS TO MATCH YOUR FILE:
#define THIS_NAMESPACE t_Presets
#include "test_macros.hpp"
#include "test_config.hpp"

using namespace Tritium;

namespace THIS_NAMESPACE
{

    struct Fixture
    {
	// SETUP AND TEARDOWN OBJECTS FOR YOUR TESTS.

	Fixture() {}
	~Fixture() {}
    };

} // namespace THIS_NAMESPACE

TEST_BEGIN( Fixture );

TEST_CASE( 010_defaults )
{
    Presets pre;
    uint8_t coarse, fine, pc;
    QString mt; // eMpTy

    CK( pre.begin() == pre.end() );
    for(int i=0 ; i<128 ; ++i) {
	coarse = rand();
	fine = rand();
	pc = rand();
	CK( mt == pre.program(coarse, fine, pc) );
    }
}

TEST_CASE( 020_basic )
{
    struct table_t {
	uint8_t coarse;
	uint8_t fine;
	uint8_t pc;
	QString uri;
    };
    table_t table[] = {
	{0, 0, 0, "/zero/zero/zero"},
	{0, 0, 8, "/zero/zero/eight"},
	{1, 2, 3, "/one/two/three"},
	{9, 8, 7, "/nine/eight/seven"},
	{128, 0, 0, "non"} /* this one is invalid */
    };
    const size_t SIZE = 5;
    Presets presets;

    // Assignment:
    table_t* it;
    for( it = &table[0] ; it < &table[SIZE] ; ++ it ) {
	presets.set_program(it->coarse,
			    it->fine,
			    it->pc,
			    it->uri);
    }

    for( it = &table[0] ; it < &table[SIZE-1] ; ++ it ) {
	CK( it->uri == presets.program(it->coarse, it->fine, it->pc) );
    }
    CK( "" == presets.program(it->coarse, it->fine, it->pc) );

    // Just in case we programmed the check wrong....
    CK( "/nine/eight/seven" == presets.program(9, 8, 7) );

    // Manually clear out and brute-force check.
    for( it = &table[0] ; it < &table[SIZE] ; ++ it ) {
	presets.set_program(it->coarse,
			    it->fine,
			    it->pc,
			    QString());
    }
    uint32_t ctr;
    uint8_t coarse, fine, pc;
    for( ctr=0 ; ctr < 0xFFFF ; ++ctr ) {
	pc = ctr & 0x7F;
	fine = (ctr >> 7) & 0x7F;
	coarse = (ctr >> 14) & 0x7F;
	CK( "" == presets.program(coarse, fine, pc) );
    }
}

TEST_CASE( 020_iterators )
{
    struct table_t {
	uint8_t coarse;
	uint8_t fine;
	uint8_t pc;
	QString uri;
    };
    table_t table[] = {
	{0, 0, 0, "/zero/zero/zero"},
	{0, 0, 8, "/zero/zero/eight"},
	{1, 2, 3, "/one/two/three"},
	{9, 8, 7, "/nine/eight/seven"},
	{128, 0, 0, "non"} /* this one is invalid */
    };
    const size_t SIZE = 5;
    Presets presets;

    // Assignment:
    table_t* it;
    for( it = &table[0] ; it < &table[SIZE] ; ++ it ) {
	presets.set_program(it->coarse,
			    it->fine,
			    it->pc,
			    it->uri);
    }

    Presets::const_iterator p;
    Bank::const_iterator b;
    uint8_t coarse, fine, pc;
    uint32_t add;
    for( p=presets.begin() ; p!=presets.end() ; ++p ) {
	coarse = p->first.coarse;
	fine = p->first.fine;
	for( b=p->second.begin() ; b!=p->second.end() ; ++b ) {
	    pc = b->first;
	    const QString& uri = b->second;
	    // Aligned 8-bit instead of 7...
	    add = (coarse << 16) | (fine << 8) | pc;
	    switch(add) {
	    case 0x000000:
		CK( uri == "/zero/zero/zero" );
		break;
	    case 0x000008:
		CK( uri == "/zero/zero/eight" );
		break;
	    case 0x010203:
		CK( uri == "/one/two/three" );
		break;
	    case 0x090807:
		CK( uri == "/nine/eight/seven" );
		break;
	    default:
		BOOST_ERROR( QString("Unknown URI returned for (%1, %2, %3): %4")
			     .arg(coarse)
			     .arg(fine)
			     .arg(pc)
			     .arg(uri)
			     .toLocal8Bit()
			     .data() );
	    }
	}
    }

    presets.clear();
    CK( presets.begin() == presets.end() );
}

TEST_END()
