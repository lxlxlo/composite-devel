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

// CHANGE THIS TO MATCH YOUR FILE:
#define THIS_NAMESPACE t_Sample
#include "test_macros.hpp"
#include "test_config.hpp"
#include <Tritium/Sample.hpp>

using namespace Tritium;

namespace THIS_NAMESPACE
{
    const char sine_wav_file[] =
	TEST_DATA_DIR "/samples/sine_480.46875_hz.wav";
    const char triangle_wav_file[] = 
	TEST_DATA_DIR "/samples/triangle_480.46875_hz.wav";
    const char sine_flac_file[] = 
	TEST_DATA_DIR "/samples/sine_480.46875_hz.flac";
    const char triangle_flac_file[] = 
	TEST_DATA_DIR "/samples/triangle_480.46875_hz.flac";

    // These are true for all 4 samples.
    const double signal_frequency = 480.46875;
    const unsigned long sample_rate = 96000;
    const double sample_length = .2560; //seconds
    const long sample_count = 24576; // samples per channel
    const double max_error = 1.0e-10; // -100 dB

    struct Fixture
    {
	// SETUP AND TEARDOWN OBJECTS FOR YOUR TESTS.
	Sample *sine_wav, *sine_flac;
	Sample *tri_wav, *tri_flac;

	Fixture() : sine_wav(0), 
		    sine_flac(0), 
		    tri_wav(0), 
		    tri_flac(0) {
	    sine_wav = Sample::load(sine_wav_file);
	    sine_flac = Sample::load(sine_flac_file);
	    tri_wav = Sample::load(triangle_wav_file);
	    tri_flac = Sample::load(triangle_flac_file);
	}
	~Fixture() {
	    delete sine_wav;
	    delete sine_flac;
	    delete tri_wav;
	    delete tri_flac;
	}
    };

} // namespace THIS_NAMESPACE

TEST_BEGIN( Fixture );

TEST_CASE( 010_defaults )
{
    Sample* samples[] = {
	sine_wav,
	sine_flac,
	tri_wav,
	tri_flac,
	0 };
    Sample **iter = samples;

    while(*iter) {
	Sample *that = *iter;

	CK(that);
	CK(that->get_data_l());
	CK(that->get_data_r());
	CK(that->get_sample_rate() == sample_rate);
	CK(that->get_size() == 2 * sample_count * sizeof(float));
	CK(that->get_n_frames() == sample_count);

	++iter;
    }

    CK(sine_wav->get_filename() == sine_wav_file);
    CK(sine_flac->get_filename() == sine_flac_file);
    CK(tri_wav->get_filename() == triangle_wav_file);
    CK(tri_flac->get_filename() == triangle_flac_file);
}

TEST_CASE( 020_something )
{
    CK( true );
}

TEST_END()
