/*
 * Copyright(c) 2010 by Gabriel M. Beddingfield <gabriel@teuton.org>
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
 * t_TritiumXml.cpp
 *
 */

#include "../src/TritiumXml.cpp"
#include <Tritium/ObjectBundle.hpp>
#include <QString>

// CHANGE THIS TO MATCH YOUR FILE:
#define THIS_NAMESPACE t_TritiumXml
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
    Serialization::TritiumXml reader;
    CK(reader.empty());

}

TEST_CASE( 020_file_typ_1 )
{
    const QString file_typ(
	"<?xml version='1.0' encoding='UTF-8'?>\n"
	"<T:presets xmlns:T='http://gabe.is-a-geek.org/tritium/xml/1/'>\n"
	"  <T:bank coarse='0' fine='0'>\n"
	"    <T:program>\n"
	"      <T:midi_number>0</T:midi_number>\n"
	"      <T:resource>tritium:drumkits/GMkit</T:resource>\n"
	"    </T:program>\n"
	"    <T:program>\n"
	"      <T:midi_number>1</T:midi_number>\n"
	"      <T:resource>tritium:drumkits/TR808EmulationKit</T:resource>\n"
	"    </T:program>\n"
	"  </T:bank>\n"
	"</T:presets>\n"
	);

    Serialization::TritiumXml reader;
    reader.readContent(file_typ);

    if( reader.error() ) {
	BOOST_ERROR(reader.error_message().toLocal8Bit().data());
    }

    BOOST_REQUIRE( ! reader.error() );
    CK( ! reader.empty() );
    CK( reader.peek_type() == ObjectItem::Presets_t );

    T<Presets>::shared_ptr presets = reader.pop<Presets>();
    CK( reader.empty() );

    CK( presets->program(0, 0, 0) == "tritium:drumkits/GMkit" );
    CK( presets->program(0x0100, 1) != "tritium:drumkits/TR808EmulationKit" );
    CK( presets->program(0, 1) == "tritium:drumkits/TR808EmulationKit" );
    CK( presets->program(9, 1, 5) == "" );
}

TEST_CASE( 030_file_typ_2 )
{
    const QString file_typ(
	"<?xml version='1.0' encoding='UTF-8'?>\n"
	"<presets xmlns='http://gabe.is-a-geek.org/tritium/xml/1/'>\n"
	"  <bank>\n"
	"    <program>\n"
	"      <midi_number>0</midi_number>\n"
	"      <resource>tritium:drumkits/GMkit_new</resource>\n"
	"    </program>\n"
	"    <program>\n"
	"      <midi_number>1</midi_number>\n"
	"      <resource>tritium:drumkits/TR808EmulationKit_new</resource>\n"
	"    </program>\n"
	"  </bank>\n"
	"  <bank coarse='2'>\n"
	"    <program>\n"
	"      <midi_number>99</midi_number>\n"
	"      <resource>file://foo/bar/bat</resource>\n"
	"    </program>\n"
	"    <program>\n"
	"      <midi_number>66</midi_number>\n"
	"      <resource>/home/joe/dk/somekit/drumkit.xml</resource>\n"
	"    </program>\n"
	"  </bank>\n"
	"  <bank coarse='1'/>\n"
	"</presets>\n"
	);

    Serialization::TritiumXml reader;
    reader.readContent(file_typ);

    if( reader.error() ) {
	BOOST_ERROR(reader.error_message().toLocal8Bit().data());
    }

    BOOST_REQUIRE( ! reader.error() );
    CK( ! reader.empty() );
    CK( reader.peek_type() == ObjectItem::Presets_t );

    T<Presets>::shared_ptr presets = reader.pop<Presets>();
    CK( reader.empty() );

    CK( presets->program(0, 0, 0) == "tritium:drumkits/GMkit_new" );
    CK( presets->program(0, 1) == "tritium:drumkits/TR808EmulationKit_new" );
    /* (0x2 << 7) == 0x100 */
    CK( presets->program(0x0100, 99) == "file://foo/bar/bat" );
    CK( presets->program(2, 0, 66) == "/home/joe/dk/somekit/drumkit.xml" );
    CK( presets->program(1, 0, 25) == "" );
    CK( presets->program(9, 1, 5) == "" );
}

TEST_CASE( 040_file_inv_namespace )
{
    const QString file_typ(
	"<?xml version='1.0' encoding='UTF-8'?>\n"
	"<T:presets xmlns:T='http://null.org/'>\n"
	"  <T:bank coarse='0' fine='0'>\n"
	"    <T:program>\n"
	"      <T:midi_number>0</T:midi_number>\n"
	"      <T:resource>tritium:drumkits/GMkit</T:resource>\n"
	"    </T:program>\n"
	"    <T:program>\n"
	"      <T:midi_number>1</T:midi_number>\n"
	"      <T:resource>tritium:drumkits/TR808EmulationKit</T:resource>\n"
	"    </T:program>\n"
	"  </T:bank>\n"
	"</T:presets>\n"
	);

    Serialization::TritiumXml reader;
    reader.readContent(file_typ);

    CK( reader.error() );
    CK( reader.empty() );
}

TEST_CASE( 050_file_inv_bank_numbers )
{
    const QString file_typ_coarse(
	"<?xml version='1.0' encoding='UTF-8'?>\n"
	"<T:presets xmlns:T='http://gabe.is-a-geek.org/tritium/xml/1/'>\n"
	"  <T:bank coarse='256' fine='0'>\n"
	"    <T:program>\n"
	"      <T:midi_number>0</T:midi_number>\n"
	"      <T:resource>tritium:drumkits/GMkit</T:resource>\n"
	"    </T:program>\n"
	"  </T:bank>\n"
	"</T:presets>\n"
	);

    const QString file_typ_fine(
	"<?xml version='1.0' encoding='UTF-8'?>\n"
	"<T:presets xmlns:T='http://gabe.is-a-geek.org/tritium/xml/1/'>\n"
	"  <T:bank coarse='0' fine='-1'>\n"
	"    <T:program>\n"
	"      <T:midi_number>0</T:midi_number>\n"
	"      <T:resource>tritium:drumkits/GMkit</T:resource>\n"
	"    </T:program>\n"
	"  </T:bank>\n"
	"</T:presets>\n"
	);

    Serialization::TritiumXml reader;
    reader.readContent(file_typ_coarse);

    CK( reader.error() );
    CK( reader.empty() );

    reader.readContent(file_typ_fine);

    CK( reader.error() );
    CK( reader.empty() );
}

TEST_CASE( 050_file_inv_program )
{
    const QString file_typ_num(
	"<?xml version='1.0' encoding='UTF-8'?>\n"
	"<T:presets xmlns:T='http://gabe.is-a-geek.org/tritium/xml/1/'>\n"
	"  <T:bank coarse='0' fine='0'>\n"
	"    <T:program>\n"
	"      <T:midi_number>128</T:midi_number>\n"
	"      <T:resource>tritium:drumkits/GMkit</T:resource>\n"
	"    </T:program>\n"
	"  </T:bank>\n"
	"</T:presets>\n"
	);

    const QString file_typ_missing(
	"<?xml version='1.0' encoding='UTF-8'?>\n"
	"<T:presets xmlns:T='http://gabe.is-a-geek.org/tritium/xml/1/'>\n"
	"  <T:bank coarse='0' fine='0'>\n"
	"    <T:program>\n"
	"      <T:midi_number>0</T:midi_number>\n"
	"      <!-- resource missing -->\n"
	"    </T:program>\n"
	"  </T:bank>\n"
	"</T:presets>\n"
	);

    Serialization::TritiumXml reader;
    reader.readContent(file_typ_num);

    CK( reader.error() );
    CK( reader.empty() );

    reader.readContent(file_typ_missing);

    CK( reader.error() );
    CK( reader.empty() );
}

TEST_END()
