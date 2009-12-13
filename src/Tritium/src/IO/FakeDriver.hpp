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

#ifndef TRITIUM_FAKEDRIVER_HPP
#define TRITIUM_FAKEDRIVER_HPP

#include <Tritium/IO/AudioOutput.hpp>
#include <inttypes.h>

namespace Tritium
{

class Engine;
typedef int  ( *audioProcessCallback )( uint32_t, void * );

/**
 * Fake audio driver. Used only for profiling.
 */
class FakeDriver : public AudioOutput
{
public:
	FakeDriver( Engine* parent, audioProcessCallback processCallback, void* arg );
	~FakeDriver();

	int init( unsigned nBufferSize );
	int connect();
	void disconnect();
	unsigned getBufferSize() {
		return m_nBufferSize;
	}
	unsigned getSampleRate();

	float* getOut_L();
	float* getOut_R();

private:
	audioProcessCallback m_processCallback;
	void* m_processCallback_arg;
	unsigned m_nBufferSize;
	float* m_pOut_L;
	float* m_pOut_R;

};


} // namespace Tritium

#endif // TRITIUM_FAKEDRIVER_HPP
