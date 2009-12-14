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
#include <cmath>

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
    /* I measured -66.2343 dB max noise between
     * sin() and sinf() with GNU libc 2.7.
     *
     * However, the signals were saved as
     * 16-bit floating point PCM, data... so
     * the roundoff error is more like -40 dB
     * for [-1.0, 1.0].
     */
    const double max_error = 1.0e-4; // -40 dB

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

	/* This works like a trig function.
	 */
	double triangle(double _angle) {
	    const double half_pi = M_PI/2.0;
	    const double one_and_half_pi = M_PI * 1.5;
	    const double two_pi = 2.0 * M_PI;

	    double rv = 0.0;
	    double angle = fmodf(_angle, two_pi);
	    if(angle <= half_pi) {
		angle += half_pi;
	    } else {
		angle -= one_and_half_pi;
	    }

	    rv = fabs(angle / half_pi) - 1.0;

	    return rv;
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
	CK(that->get_n_frames() == ((unsigned)sample_count));

	++iter;
    }

    CK(sine_wav->get_filename() == sine_wav_file);
    CK(sine_flac->get_filename() == sine_flac_file);
    CK(tri_wav->get_filename() == triangle_wav_file);
    CK(tri_flac->get_filename() == triangle_flac_file);
}

TEST_CASE( 020_no_clipping )
{
    Sample* samples[] = {
	sine_wav,
	sine_flac,
	tri_wav,
	tri_flac,
	0 };
    Sample **iter = samples;
    float *left, *right;
    unsigned long k;

    while(*iter) {
	Sample *that = *iter;

	CK(that);
	left = that->get_data_l();
	right = that->get_data_r();
	for(k=0 ; k<that->get_n_frames() ; ++k) {
	    CK( fabs(left[k]) <= 1.0f );
	    CK( fabs(right[k]) <= 1.0f );
	}

	++iter;
    }

}

TEST_CASE( 020_sine_waves )
{
    /*****************************************
     * The sine waves were generated with the
     * C Standard Library sinf( (2*PI*f/rate) * offset ).
     * This just checks that we get it back.
     *****************************************
     */
    unsigned long k;
    double scalar = 2 * M_PI * signal_frequency / ((double) sample_rate);
    float tmp;
    float *left, *right;
    double e, e_max=0.0;

    // WAV file
    left = sine_wav->get_data_l();
    right = sine_wav->get_data_r();
    for(k=0 ; k<sine_wav->get_n_frames() ; ++k) {
	tmp = sin(scalar * double(k));
	e = fabs(left[k] - tmp);
	CK( e < max_error );
	if( e > e_max ) e_max = e;
	e = fabs(right[k] - tmp);
	CK( e < max_error );
	if( e > e_max ) e_max = e;
    }

    // FLAC file (was actually created from the WAV file)
    left = sine_flac->get_data_l();
    right = sine_flac->get_data_r();
    for(k=0 ; k<sine_flac->get_n_frames() ; ++k) {
	tmp = sin(scalar * double(k));
	e = fabs(left[k] - tmp);
	CK( e < max_error );
	if( e > e_max ) e_max = e;
	e = fabs(right[k] - tmp);
	CK( e < max_error );
	if( e > e_max ) e_max = e;
    }

    // WAV and FLAC files should be identical.
    for( k=0 ; k<sine_wav->get_n_frames() ; ++k ) {
	CK( fabs( sine_wav->get_data_l()[k] - sine_flac->get_data_l()[k] )
	    == 0.0 );
	CK( fabs( sine_wav->get_data_r()[k] - sine_flac->get_data_r()[k] )
	    == 0.0 );
    }

    BOOST_MESSAGE("Max sine wave error (dB):");
    BOOST_MESSAGE(10.0*log10(e_max));
}

TEST_CASE( 030_triangle_waves )
{
    /*****************************************
     * The triangle waves were generated with
     * a trig-like function triangle( (2*PI*f/rate) * offset ).
     * the triangle() function was:
     *
     * const double PI = M_PI;
     * float triangle(float _angle)
     * {
     *     const double half_pi = PI/2.0;
     *     const double one_and_half_pi = PI * 1.5;
     *     const double two_pi = 2.0 * PI;
     *     double rv = 0.0;
     *     double angle = fmodf(_angle, two_pi);
     *
     *     if(angle <= half_pi) {
     * 	       rv = angle/half_pi;
     *     } else if (angle <= one_and_half_pi) {
     * 	       angle -= PI;
     * 	       rv = - angle/half_pi;
     *     } else {
     * 	       angle -= two_pi;
     * 	       rv = angle/half_pi;
     *     }
     *     assert( fabs(rv) <= 1.0 );
     *     return rv;
     * }
     *
     * We will check that we get it back with
     * a slightly different algorithm.
     *****************************************
     */
    unsigned long k;
    double scalar = 2 * M_PI * signal_frequency / ((double) sample_rate);
    float tmp;
    float *left, *right;
    double e, e_max = 0.0;

    // WAV file
    left = tri_wav->get_data_l();
    right = tri_wav->get_data_r();
    for(k=0 ; k<tri_wav->get_n_frames() ; ++k) {
	tmp = triangle(scalar * double(k));
	e = fabs(left[k] - tmp);
	CK( e < max_error );
	if( e > e_max ) e_max = e;
	e = fabs(right[k] - tmp);
	CK( e < max_error );
	if( e > e_max ) e_max = e;
    }

    // FLAC file (was actually created from the WAV file)
    left = tri_flac->get_data_l();
    right = tri_flac->get_data_r();
    for(k=0 ; k<tri_flac->get_n_frames() ; ++k) {
	tmp = triangle(scalar * double(k));
	e = fabs(left[k] - tmp);
	CK( e < max_error );
	if( e > e_max ) e_max = e;
	e = fabs(right[k] - tmp);
	CK( e < max_error );
	if( e > e_max ) e_max = e;
    }

    // WAV and FLAC files should be identical.
    for( k=0 ; k<tri_wav->get_n_frames() ; ++k ) {
	CK( fabs( tri_wav->get_data_l()[k] - tri_flac->get_data_l()[k] )
	    == 0.0 );
	CK( fabs( tri_wav->get_data_r()[k] - tri_flac->get_data_r()[k] )
	    == 0.0 );
    }

    BOOST_MESSAGE("Max triangle wave error (dB):");
    BOOST_MESSAGE(10.0*log10(e_max));
}

TEST_END()
