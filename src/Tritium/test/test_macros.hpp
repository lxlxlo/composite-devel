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
#ifndef TRITIUMTEST_TEST_MACROS_HPP
#define TRITIUMTEST_TEST_MACROS_HPP

/*
 * test_macros.h
 *
 * These macros are primarily intended to make writing tests more
 * uniform and straightforward.  Using them is optional, but highly
 * encouraged.
 *
 * A nice side-effect of using these macros is that it abstracts out
 * the testing framework that is being used.  However, that is not the
 * intention.  For example, it's perfectly fine to use the
 * BOOST_MESSAGE() macro, or anything else the BOOST framework
 * provides.
 *
 * To use it, make a copy of t_Foo.cpp.template and season to taste.
 */

#include <boost/test/unit_test.hpp>

#ifndef THIS_NAMESPACE
#error "You must define THIS_NAMESPACE before including this file."
#endif

#if defined(TEST_BEGIN) || defined(TEST_END) || defined(TEST_CASE)
#error "TEST_SUITE and/or TEST_CASE are already defined... but should not be."
#endif

/**
 * TEST_BEGIN declares a series of tests to be used.  It must be
 * terminated with TEST_END().
 */
#define TEST_BEGIN(fixture) BOOST_FIXTURE_TEST_SUITE( THIS_NAMESPACE, fixture )
#define TEST_END() BOOST_AUTO_TEST_SUITE_END()

/**
 * TEST_CASE is a macro for declaring a function that will run a
 * series of tests.  The fixture will be set up before this test case
 * is called, and members of the fixture may be referred to without
 * qualifiers.
 */
#define TEST_CASE(x) BOOST_AUTO_TEST_CASE( _##x )

/**
 * CK() is a macro for "check".  It's an alias for BOOST_CHECK()
 */
#define CK(x) BOOST_CHECK(x)


#endif // TRITIUMTEST_TEST_MACROS_HPP
