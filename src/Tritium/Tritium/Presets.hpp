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

namespace Tritium
{
    /**
     * A bank of presets (programs)
     */
    class Bank
    {
	typedef std::map<uint8_t, QString> map_t;
    public:
	Bank() {}
	~Bank() {}

	const QString& program(uint8_t num) const {
	    map_t::const_iterator it;
	    it = _programs.find(num);
	    if(it != _programs.end()) {
		return it->second;
	    }
	    return _none;
	}

	void program(uint8_t num, const QString& uri) {
	    if(num < 128) {
		_programs[num] = uri;
	    }
	}

    private:
	map_t _programs;
	QString _none;
    }; // class Bank

    /**
     * Class that manages a set of preset banks.
     */
    class Presets
    {
	typedef std::map<uint8_t, Bank> map_t;

	inline static uint16_t _8_16(uint8_t coarse, uint8_t fine) {
	    uint16_t rv = ((coarse & 0x7F) << 7) | (fine & 0x7F);
	    return rv;
	}

    public:
	Presets() {}
	~Presets() {}

	const QString& program(uint8_t coarse, uint8_t fine, uint8_t prog) const {
	    uint16_t bank = _8_16(coarse, fine);
	    return program(bank, prog);
	}

	const QString& program(uint16_t bank, uint8_t prog) const {
	    map_t::const_iterator it;
	    it = _banks.find(bank);
	    if(it != _banks.end()) {
		return it->second.program(prog);
	    }
	    return _none;
	}

	void program(uint8_t coarse, uint8_t fine, uint8_t prog, QString uri) {
	    uint16_t bank = _8_16(coarse, fine);
	    program(bank, prog, uri);
	}

	void program(uint16_t bank, uint8_t prog, QString uri) {
	    if(_banks.find(bank) == _banks.end()) {
		_banks[bank] = Bank();
	    }
	    _banks[bank].program(prog, uri);
	}

    private:
	map_t _banks;
	QString _none;
    }; //class Presets

} // namespace Tritium

#endif // TRITIUM_PRESETS_HPP
