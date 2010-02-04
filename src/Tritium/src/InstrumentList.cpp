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

#include <Tritium/InstrumentList.hpp>
#include <Tritium/Instrument.hpp>
#include <Tritium/Logger.hpp>
#include <QString>

#include <deque>
#include <map>
#include <cassert>

using namespace Tritium;

InstrumentList::InstrumentList()
{
}



InstrumentList::~InstrumentList()
{
}



void InstrumentList::add( T<Instrument>::shared_ptr newInstrument )
{
    m_list.push_back( newInstrument );
    m_posmap[newInstrument] = m_list.size() - 1;
}



T<Instrument>::shared_ptr InstrumentList::get( unsigned int pos )
{
    if ( pos >= m_list.size() ) {
	ERRORLOG( QString( "pos > list.size(). pos = %1" ).arg( pos ) );
	return T<Instrument>::shared_ptr();
    }
    return m_list[pos];
}



/// Returns index of instrument in list, if instrument not found, returns -1
int InstrumentList::get_pos( T<Instrument>::shared_ptr pInstr )
{
    if ( m_posmap.find( pInstr ) == m_posmap.end() )
	return -1;
    return m_posmap[ pInstr ];
}

unsigned int InstrumentList::get_size()
{
    return m_list.size();
}


void InstrumentList::replace( T<Instrument>::shared_ptr pNewInstr, unsigned nPos )
{
    if ( nPos >= m_list.size() ) {
	ERRORLOG( QString( "Instrument index out of bounds in InstrumentList::replace. pos >= list.size() - %1 > %2" ).arg( nPos ).arg( m_list.size() ) );
	return;
    }
    m_list.insert( m_list.begin() + nPos, pNewInstr );	// insert the new Instrument
    // remove the old Instrument
    m_list.erase( m_list.begin() + nPos + 1 );
}


void InstrumentList::del( int pos )
{
    assert( pos < ( int )m_list.size() );
    assert( pos >= 0 );
    m_list.erase( m_list.begin() + pos );
}

/**
 * Removes all instruments from the list.
 */
void InstrumentList::clear()
{
    unsigned size = get_size();
    while( size-- > 0 ) {
	del(size);
    }
}
