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

#ifndef TRITIUM_PATTERN_HPP
#define TRITIUM_PATTERN_HPP

#include <Tritium/globals.hpp>
#include <Tritium/memory.hpp>
#include <vector>

namespace Tritium
{

class Note;
class Instrument;
class Engine;

///
/// The Pattern is a Note container.
///
class Pattern
{
public:
	typedef std::multimap <int, Note*> note_map_t;
	note_map_t note_map;

	Pattern( const QString& name, const QString& category, unsigned length = MAX_NOTES );
	~Pattern();

	/**
	  Delete notes that pertain to instrument I.
	  The function is thread safe (it locks the audio data while deleting notes)
	*/
	void purge_instrument( T<Instrument>::shared_ptr I, Engine* engine );

	/**
	  Check if there are any notes pertaining to I
	*/
	bool references_instrument( T<Instrument>::shared_ptr I );

	static T<Pattern>::shared_ptr get_empty_pattern();
	T<Pattern>::shared_ptr copy();

	void debug_dump();

	unsigned get_length() {
		return __length;
	}
	void set_length( unsigned length ) {
		__length = length;
	}

	void set_name( const QString& name ) {
		__name = name;
	}
	const QString& get_name() const {
		return __name;
	}

	void set_category( const QString& category ) {
		__category = category;
	}
	const QString& get_category() const {
		return __category;
	}

private:
	unsigned __length;
	QString __name;
	QString __category;
};


/// Pattern List
class PatternList
{
public:
	PatternList();
	~PatternList();

	void add( T<Pattern>::shared_ptr new_pattern );
	T<Pattern>::shared_ptr get( int pos );
	unsigned int get_size();
	void clear();

	void replace( T<Pattern>::shared_ptr new_pattern, unsigned pos );
	int index_of( T<Pattern>::shared_ptr pattern );

	/// Remove a pattern from the list (every instance in the list), the pattern is not deleted!!!
	/// Returns NULL if the pattern is not in the list
	T<Pattern>::shared_ptr del( T<Pattern>::shared_ptr pattern );

	/// Remove one pattern from the list, the pattern is not deleted!!!
	void del( unsigned index );

private:
	std::vector< T<Pattern>::shared_ptr > list;
};

};

#endif // TRITIUM_PATTERN_HPP
