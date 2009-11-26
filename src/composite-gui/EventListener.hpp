/*
 * Copyright(c) 2002-2008 by Alex >Comix< Cominu [comix@users.sourceforge.net]
 *
 * This file is part of Composite
 *
 * Composite is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Composite is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY, without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */
#ifndef COMPOSITE_EVENTLISTENER_HPP
#define COMPOSITE_EVENTLISTENER_HPP

#include <Tritium/globals.hpp>
#include <Tritium/TransportPosition.hpp>

class EventListener
{
	public:
		virtual void stateChangedEvent(int nState) { UNUSED( nState ); }
		virtual void patternChangedEvent() {}
		virtual void patternModifiedEvent() {}
		virtual void selectedPatternChangedEvent() {}
		virtual void selectedInstrumentChangedEvent() {}
		virtual void midiActivityEvent() {}
		virtual void noteOnEvent( int nInstrument ) { UNUSED( nInstrument ); }
		virtual void XRunEvent() {}
		virtual void errorEvent( int nErrorCode ) { UNUSED( nErrorCode ); }
		virtual void metronomeEvent( int nValue ) { UNUSED( nValue ); }
		virtual void progressEvent( int nValue ) { UNUSED( nValue ); }
		virtual void transportEvent( Tritium::TransportPosition::State state ) { UNUSED( state ); }
		virtual void jackTimeMasterEvent( int nValue ) { UNUSED( nValue ); }

		virtual ~EventListener() {}
};


#endif // COMPOSITE_EVENTLISTENER_HPP
