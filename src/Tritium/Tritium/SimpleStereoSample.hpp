/*
 * Copyright(c) 2002-2008 by Alex >Comix< Cominu [comix@users.sourceforge.net]
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

#ifndef TRITIUM_SIMPLESTEREOSAMPLE_HPP
#define TRITIUM_SIMPLESTEREOSAMPLE_HPP

#include <Tritium/globals.hpp>
#include <Tritium/memory.hpp>
#include <Tritium/Sample.hpp>
#include <QtCore/QString>
#include <QtCore/QUrl>

namespace Tritium
{

class SimpleStereoSample : public Sample
{
public:
    SimpleStereoSample(
	unsigned frames,
	const QString& filename,
	unsigned sample_rate,
	float* data_L = NULL,
	float* data_R = NULL
	);

    virtual ~SimpleStereoSample();

    // Tritium::Sample interface:
    virtual int channel_count() const { return 2; }
    virtual unsigned size() const { return __n_frames; }

    virtual float* data(int chan) {
	if(chan == 0) {
	    return __data_l;
	} else if (chan == 1) {
	    return __data_r;
	}
	return 0;
    }
    virtual const float* data(int chan) const {
	return data(chan);
    }

    virtual float sample_rate() const { return float(__sample_rate); }
    virtual const QUrl& source_url() const;

    TRITIUM_DEPRECATED
    virtual QString get_filename() const {
	return __filename;
    }


    // Old Hydrogen::Sample interface

    TRITIUM_DEPRECATED
    float* get_data_l() {
	return __data_l;
    }

    TRITIUM_DEPRECATED
    float* get_data_r() {
	return __data_r;
    }

    TRITIUM_DEPRECATED
    unsigned get_sample_rate() {
	return unsigned( sample_rate() + 0.5f );
    }

    /// Returns the bytes number ( 2 channels )
    TRITIUM_DEPRECATED
    unsigned get_size() {
	return size() * sizeof( float ) * 2;
    }

    TRITIUM_DEPRECATED
    unsigned get_n_frames() {
	return size();
    }

    /// Loads a sample from disk
    static T<SimpleStereoSample>::shared_ptr load( const QString& filename );

private:
    float *__data_l;	///< Left channel data
    float *__data_r;	///< Right channel data

    unsigned __sample_rate;		///< samplerate for this sample
    QUrl __url;
    QString __filename;		///< filename associated with this sample
    unsigned __n_frames;		///< Total number of frames in this sample.

    //static int __total_used_bytes;

    /// loads a wave file
    static T<SimpleStereoSample>::shared_ptr load_wave( const QString& filename );

    /// loads a FLAC file
    static T<SimpleStereoSample>::shared_ptr load_flac( const QString& filename );
}; // class SimpleStereoSample

} // namespace Tritium

#endif // TRITIUM_SIMPLESTEREOSAMPLE_HPP
