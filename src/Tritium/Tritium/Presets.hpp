/*
 * Copyright(c) 2010 by Gabriel M. Beddingfield <gabriel@teuton.org>
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
#ifndef TRITIUM_PRESETS_HPP
#define TRITIUM_PRESETS_HPP

#include <stdint.h>
#include <map>
#include <QString>
#include <Tritium/memory.hpp>

namespace Tritium
{
    class Preferences;

    /**
     * A bank of presets (programs)
     *
     * You will typically /not/ access this class directly unless you
     * are querying using the iterators.  You will usually access the
     * presets through Tritium::Presets object.
     */
    class Bank
    {
    public:
	typedef std::map<uint8_t, QString> map_t;
	typedef map_t::value_type pair_t;
	typedef map_t::value_type value_type;
	typedef map_t::iterator iterator;
	typedef map_t::const_iterator const_iterator;

	Bank() {}
	~Bank() {}

	/**
	 * Retrieve the preset URI for a channel.
	 */
	const QString& program(uint8_t num) const {
	    map_t::const_iterator it;
	    it = _programs.find(num);
	    if(it != _programs.end()) {
		return it->second;
	    }
	    return _none;
	}

	/**
	 * Assigns a URI to a channel.
	 *
	 * To remove a program, use uri = ""
	 */
	void program(uint8_t num, const QString& uri) {
	    if(num < 128) {
		if(uri != "") {
		    _programs[num] = uri;
		} else {
		    iterator it = _programs.find(num);
		    if( it != _programs.end() ) {
			_programs.erase(it);
		    }
		}
	    }
	}

	// ITERATOR ACCESS
	iterator begin() { return _programs.begin(); }
	const_iterator begin() const { return _programs.begin(); }
	iterator end() { return _programs.end(); }
	const_iterator end() const { return _programs.end(); }

    private:
	map_t _programs;
	QString _none;
    }; // class Bank

    /**
     * Class that manages user presets.
     *
     * In simple terms, this class manages a bunch of Tritium::Bank
     * objects.  However, you usually do not need to access the Bank
     * objects directly.  The only reason to do so is if you're
     * querying the presets with the iterator access.
     */
    class Presets
    {
    public:
	struct bank_address_t {
	    uint8_t coarse;
	    uint8_t fine;

	    int operator<(const bank_address_t& right) const {
		if(coarse == right.coarse) {
		    return fine < right.fine;
		}
		return coarse < right.coarse;
	    }
	};

	typedef std::map<bank_address_t, Bank> map_t;
	typedef map_t::value_type pair_t;
	typedef map_t::value_type value_type;
	typedef map_t::iterator iterator;
	typedef map_t::const_iterator const_iterator;

	Presets() {}
	~Presets() {}

    protected:
	inline static bool valid(uint8_t coarse, uint8_t fine, uint8_t prog) {
	    return (coarse < 128) && (fine < 128) && (prog < 128);
	}

    public:
	/**
	 * Returns the URI stored at bank (coarse, fine) and program number.
	 *
	 * If none is stored, an empty string will be returned.
	 */
	const QString& program(uint8_t coarse, uint8_t fine, uint8_t prog) const {
	    if( valid(coarse, fine, prog) ) {
		bank_address_t bank = {coarse, fine};
		map_t::const_iterator it;
		it = _banks.find(bank);
		if(it != _banks.end()) {
		    return it->second.program(prog);
		}
	    }
	    return _none;
	}

	/**
	 * Overloaded method for convenience.
	 */
	const QString& program(uint16_t bank, uint8_t prog) const {
	    if( bank == (bank & 0x3FFF) ) {
		uint8_t fine = bank & 0x7F;
		uint8_t coarse = bank >> 7; // allowing invalid
		return program(coarse, fine, prog);
	    }
	    return _none;
	}

	/**
	 * Assign a URI to a specific bank (coarse, fine) and program number.
	 *
	 * To delete the preset, simply set uri = "".
	 */
	void set_program(uint8_t coarse, uint8_t fine, uint8_t prog, QString uri) {
	    /* Note that when URI == "", the Bank itself is /not/ being
	     * deleted.  No real reason... just doesn't seem necc.
	     */
	    if( valid(coarse, fine, prog) ) {
		bank_address_t bank = {coarse, fine};
		if(_banks.find(bank) == _banks.end()) {
		    _banks[bank] = Bank();
		}
		_banks[bank].program(prog, uri);
	    }
	}

	/**
	 * Overloaded method for convenience.
	 */
	void set_program(uint16_t bank, uint8_t prog, QString uri) {
	    if( bank == (bank & 0x3FFF) ) {
		uint8_t fine = bank & 0x7F;
		uint8_t coarse = bank >> 7; // allowing invalid
		set_program(coarse, fine, prog, uri);
	    }
	}

	void clear() {
	    _banks.clear();
	}

	void generate_default_presets(T<Preferences>::shared_ptr prefs);

	// ITERATOR ACCESS
	iterator begin() { return _banks.begin(); }
	const_iterator begin() const { return _banks.begin(); }
	iterator end() { return _banks.end(); }
	const_iterator end() const { return _banks.end(); }

    private:
	map_t _banks;
	QString _none;
    }; //class Presets

} // namespace Tritium

#endif // TRITIUM_PRESETS_HPP
