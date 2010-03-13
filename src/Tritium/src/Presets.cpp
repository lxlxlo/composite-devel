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
#include <cassert>

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
	QString tmp;

	dirs << QDir(DataPath::get_data_path()).absolutePath() + "/";
	if(prefs) {
	    dirs << QDir(prefs->getDataDirectory()).absolutePath() + "/";
	}

	QStringList drumkits;
	QStringList::Iterator it, sd;
	for( it=dirs.begin() ; it!=dirs.end() ; ++it ) {
	    QDir dir(*it);
	    dir.cd("drumkits");
	    if( ! dir.exists() ) continue;
	    QStringList subdirs;
	    subdirs << dir.entryList(QDir::AllDirs, QDir::Name);
	    for( sd=subdirs.begin() ; sd!=subdirs.end() ; ++sd ) {
		QDir sub = dir;
		sub.cd(*sd);
		if(sub.exists("drumkit.xml")) {
		    drumkits << sub.absolutePath();
		}
	    }
	}

	/* The following code depends on Qt always returning '/'
	 * for path separators.... which is indeed what they do.
	 * Therefore this check should always pass.  But we leave
	 * it here because it fails then our code will break.
	 *
	 * Docs for QDir::rootPath() say that it should return
	 * "/" on Unix and "C:/" on Windows.
	 *
	 * The reason why this is important is that we are
	 * constructing URI's with the path names.  URI's must have
	 * '/' as the separator.
	 */
	assert( QDir::rootPath().endsWith("/") );

	// Convert paths to tritium: URL's
	QStringList::Iterator dk;
	for( dk=drumkits.begin() ; dk != drumkits.end() ; ++dk ) {
	    for( it=dirs.begin() ; it!=dirs.end() ; ++it ) {
		QString& data = (*it);
		if(dk->startsWith(data)) {
		    (*dk) = (*dk).replace(data, "tritium:");
		    assert( (*dk).startsWith("tritium:drumkits/") );
		}
	    }
	    if( ! dk->startsWith("tritium:") ) {
		(*dk) = "file:///" + (*dk); // E.g. file:///C:/foo/bar
		(*dk).replace("file:////", "file:///");
	    }
	}

	// Make GMkit patch 0,0,0
	const QString GMkit = "tritium:drumkits/GMkit";
	assert( drumkits.contains(GMkit) );
	drumkits.removeAll(GMkit);
	drumkits.push_front(GMkit);

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
