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

#ifndef TRITIUM_DISKWRITERDRIVER_HPP
#define TRITIUM_DISKWRITERDRIVER_HPP

#include <sndfile.h>

#include <inttypes.h>

#include <Tritium/IO/AudioOutput.hpp>
#include <QString>

namespace Tritium
{

typedef int  ( *audioProcessCallback )( uint32_t, void * );

///
/// Driver for export audio to disk
///
class DiskWriterDriver : public AudioOutput
{
public:
	unsigned m_nSampleRate;
	QString m_sFilename;
	unsigned m_nBufferSize;
	audioProcessCallback m_processCallback;
	void* m_processCallback_arg;
	float* m_pOut_L;
	float* m_pOut_R;

	DiskWriterDriver(
		audioProcessCallback processCallback,
		void* arg,
		unsigned nSamplerate,
		const QString& sFilename );
	~DiskWriterDriver();

	int init( unsigned nBufferSize );

	int connect();
	void disconnect();

	void write( float* buffer_L, float* buffer_R, unsigned int bufferSize );

	unsigned getBufferSize() {
		return m_nBufferSize;
	}

	unsigned getSampleRate();
	float* getOut_L() {
		return m_pOut_L;
	}
	float* getOut_R() {
		return m_pOut_R;
	}

private:

};

} // namespace Tritium

#endif // TRITIUM_DISKWRITERDRIVER_HPP
