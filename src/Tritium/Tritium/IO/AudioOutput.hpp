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
#ifndef TRITIUM_AUDIOOUTPUT_HPP
#define TRITIUM_AUDIOOUTPUT_HPP

namespace Tritium
{

///
/// Base abstract class for audio output classes.
///
class AudioOutput
{
public:
	AudioOutput() : __track_out_enabled( false ) { }

	virtual ~AudioOutput() { }

	virtual int init( unsigned nBufferSize ) = 0;
	virtual int connect() = 0;
	virtual void disconnect() = 0;
	virtual unsigned getBufferSize() = 0;
	virtual unsigned getSampleRate() = 0;
	virtual float* getOut_L() = 0;
	virtual float* getOut_R() = 0;

	bool has_track_outs() {
		return __track_out_enabled;
	}

protected:
	bool __track_out_enabled;	///< True if is capable of per-track audio output

};

} // namespace Tritium

#endif // TRITIUM_AUDIOOUTPUT_HPP

