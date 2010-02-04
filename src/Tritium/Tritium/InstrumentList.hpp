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

#ifndef TRITIUM_INSTRUMENTLIST_HPP
#define TRITIUM_INSTRUMENTLIST_HPP

#include <Tritium/globals.hpp>
#include <deque>
#include <map>
#include <utility> // std::pair
#include <Tritium/memory.hpp>

namespace Tritium
{
    class Instrument;

    /**

       \brief Instrument List

    */
    class InstrumentList
    {
    public:
	typedef T<Instrument>::shared_ptr instrument_t;
	typedef std::deque<instrument_t> sequence_t;
	typedef std::map<instrument_t, unsigned> map_t;

	InstrumentList();
	~InstrumentList();

	void add( T<Instrument>::shared_ptr pInstrument );
	T<Instrument>::shared_ptr get( unsigned int pos );
	int get_pos( T<Instrument>::shared_ptr inst );
	unsigned get_size();

	void del( int pos );
	void clear();

	void replace( T<Instrument>::shared_ptr pNewInstr, unsigned nPos );

    private:
	sequence_t m_list;
	map_t m_posmap;
    };

} // namespace Tritium

#endif // TRITIUM_INSTRUMENTLIST_HPP
