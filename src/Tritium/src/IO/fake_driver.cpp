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

#include <Tritium/hydrogen.h>
#include <Tritium/Transport.h>
#include "FakeDriver.h"

namespace Tritium
{

FakeDriver::FakeDriver( audioProcessCallback processCallback )
		: AudioOutput( "FakeDriver" )
		, m_processCallback( processCallback )
		, m_pOut_L( NULL )
		, m_pOut_R( NULL )
{
	INFOLOG( "INIT" );
}


FakeDriver::~FakeDriver()
{
	INFOLOG( "DESTROY" );
}


int FakeDriver::init( unsigned nBufferSize )
{
	INFOLOG( QString( "Init, %1 samples" ).arg( nBufferSize ) );

	m_nBufferSize = nBufferSize;
	m_pOut_L = new float[nBufferSize];
	m_pOut_R = new float[nBufferSize];

	return 0;
}


int FakeDriver::connect()
{
	INFOLOG( "connect" );

        Hydrogen::get_instance()->get_transport()->locate(0);
        Hydrogen::get_instance()->get_transport()->start();

	return 0;
}


void FakeDriver::disconnect()
{
	INFOLOG( "disconnect" );

	delete[] m_pOut_L;
	m_pOut_L = NULL;

	delete[] m_pOut_R;
	m_pOut_R = NULL;
}


unsigned FakeDriver::getSampleRate()
{
	return 44100;
}

float* FakeDriver::getOut_L()
{
	return m_pOut_L;
}


float* FakeDriver::getOut_R()
{
	return m_pOut_R;
}


};
