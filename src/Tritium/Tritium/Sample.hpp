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

#ifndef TRITIUM_SAMPLE_HPP
#define TRITIUM_SAMPLE_HPP

#include <Tritium/globals.hpp>
#include <QtCore/QString>

class QUrl;

namespace Tritium
{

/**
 * Standard interface for accessing an audio sample.
 *
 */
class Sample
{
public:
    virtual ~Sample() {}

    /**
     * Number of channels for the audio data. (e.g. 2 for Stereo)
     */
    virtual int channel_count() = 0;

    /**
     * Returns the length (number of frames) of the sample.
     */
    virtual unsigned size() = 0;

    /**
     * Returns a pointer to the start of the audio data for given channel.
     */
    virtual float* data(int chan) = 0;

    /**
     * Returns the sample rate for the audio data.
     */
    virtual float sample_rate() = 0;

    /**
     * Returns the name of the file that sourced the data
     */
    virtual const QUrl& source_url() = 0;

    /**
     * Returns a file name for the source data.
     */
    TRITIUM_DEPRECATED
    virtual QString get_filename() = 0;

}; // class Sample

} // namespace Tritium

#endif // TRITIUM_SAMPLE_HPP
