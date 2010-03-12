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

#include <Tritium/Presets.hpp>
#include <Tritium/Preferences.hpp>
#include <Tritium/DataPath.hpp>
#include <QStringList>
#include <QString>

namespace Tritium
{
    /**
     * Creates presets mapping by scanning user's drumkits.
     *
     */
    void Presets::generate_default_presets(T<Preferences>::shared_ptr prefs)
    {
	// Discovery
	QStringList dirs;

	dirs << DataPath::get_data_path();
	if(prefs) {
	    dirs << prefs->getDataDirectory();
	}

	QStringList drumkits;
	QStringList::Iterator it, sd;
	for( it=dirs.begin() ; it!=dirs.end() ; ++it ) {
	    QDir dir(*it);
	    dir.cd("drumkits");
	    if( ! dir.exists() ) continue;
	    QStringList subdirs;
	    subdirs << dir.entryList(QDir::AllDirs);
	    for( sd=subdirs.begin() ; sd!=subdirs.end() ; ++sd ) {
		QDir sub = dir;
		sub.cd(*sd);
		if(sub.exists("drumkit.xml")) {
		    drumkits << sub.absolutePath();
		}
	    }
	}

	// Convert paths to tritium: URL's
	QStringList::Iterator dk;
	for( it=dirs.begin() ; it!=dirs.end() ; ++it ) {
	    QString data = (*it);
	    if( ! data.endsWith("/") ) {
		data += "/";
	    }
	    for( dk=drumkits.begin() ; dk!=drumkits.end() ; ++dk ) {
		if(dk->startsWith(data)) {
		    (*dk) = (*dk).replace(data, "tritium:");
		}
	    }
	}

	// Assignment
	clear();
	uint32_t ctr;
	for( ctr=0, dk=drumkits.begin() ; dk!=drumkits.end() ; ++ctr, ++dk ) {
	    if( (ctr & 0x1FFFFF) != ctr ) break;
	    uint8_t pc = ctr & 0x7F;
	    uint8_t coarse = (ctr >> 7) & 0x7F;
	    uint8_t fine = (ctr >> 14) & 0x7F;
	    set_program(coarse, fine, pc, *dk);
	}
    }

} // namespace Tritium
