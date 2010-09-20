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

#ifndef TRITIUM_SIMPLESAMPLE_HPP
#define TRITIUM_SIMPLESAMPLE_HPP

#include <Tritium/globals.hpp>
#include <Tritium/memory.hpp>
#include <Tritium/Sample.hpp>
#include <vector>
#include <QtCore/QString>
#include <QtCore/QUrl>

namespace Tritium
{

class SimpleSample : public Sample
{
public:
    SimpleSample(
	int channels,
	unsigned frames,
	float sample_rate,
	const QUrl& url
	);
    virtual ~SimpleSample();

    // Tritium::Sample interface:
    virtual int channel_count() const {
	return _chan_ptrs.size();
    }
    virtual unsigned size() const {
	return _size;
    }
    virtual float* data(int chan) {
	assert(chan < channel_count());
	return _chan_ptrs[chan];
    }
    virtual const float* data(int chan) const {
	assert(chan < channel_count());
	return _chan_ptrs[chan];
    }
    virtual float sample_rate() const {
	return _sample_rate;
    }
    virtual const QUrl& source_url() const {
	return _url;
    }

    TRITIUM_DEPRECATED
    virtual QString get_filename() const {
	return QString();
    }

    static T<SimpleSample>::shared_ptr empty_sample(int chans);

private:
    unsigned _size; // length of sample in frames
    float _sample_rate;
    std::vector<float> _memory;
    std::vector<float*> _chan_ptrs;
    QUrl _url;

}; // class SimpleSample

} // namespace Tritium

#endif // TRITIUM_SIMPLESAMPLE_HPP
