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
 * t_TestTemplate.cpp
 *
 * This is just the template for a test.  If the template is not
 * executable, it will not be kept up-to-date.
 */

// YOUR INCLUDES HERE
#include <Tritium/ObjectBundle.hpp>

// CHANGE THIS TO MATCH YOUR FILE:
#define THIS_NAMESPACE t_ObjectBundle
#include "test_macros.hpp"
#include "test_config.hpp"

namespace Tritium
{
    // These are MOCK OBJECTS
    class Instrument
    {
    public:
	int i;
    };

    class Pattern
    {
    public:
	float p;
    };

    class Song
    {
    public:
	double s;
    };

    class Drumkit
    {
    public:
	long d;
    };

    class LadspaFX
    {
    public:
	int fx;
    };
}

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
    ObjectBundle bdl;

    // Set up.
    {
	T<Song>::shared_ptr s(new Song);
	s->s = 5.5;
	bdl.push(s);
	T<Instrument>::shared_ptr i1(new Instrument);
	i1->i = -1;
	bdl.push(i1);
	T<Instrument>::shared_ptr i2(new Instrument);
	i2->i = -2;
	bdl.push(i2);
    }

    // Check
    {
	CK(bdl.peek_type() == ObjectItem::Song_t);
	T<Song>::shared_ptr s;
	s = bdl.pop<Song>();
	CK(s->s == 5.5);
	CK(!bdl.empty());
	CK(bdl.peek_type() == ObjectItem::Instrument_t);
	T<Instrument>::shared_ptr i;
	i = bdl.pop<Instrument>();
	CK(i->i == -1);
	CK(bdl.peek_type() == ObjectItem::Instrument_t);
	i = bdl.pop<Instrument>();
	CK(i->i == -2);
	CK(bdl.empty());
    }
}

TEST_CASE( 020_something )
{
    CK( true );
}

TEST_END()
