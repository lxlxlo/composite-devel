/*
 * Copyright(c) 2002-2008 Jonathan Dempsey, Alessandro Cominu
 *
 * This file is part of Tritium
 *
 * Tritium is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Tritium is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */
/*
 * DataPath.h
 * Header to define the path to the data files for Composite in such a
 * way that self-contained Mac OS X application bundles can be built.
 * Copyright (c) 2005 Jonathan Dempsey
 *
 */
#include <Tritium/DataPath.hpp>

#include <QFile>

//#ifdef Q_OS_MACX
//#  include <Carbon.h>
//#endif

#include <iostream>
#include <cstdlib>
using namespace std;

#include "config.h"

namespace Tritium
{

    QString DataPath::__data_path;

    /**
     * \brief Find the directory where things like drum kits are stored.
     *
     * This function only evaluates once per session.  After that, the
     * value is cached.
     *
     * The function searches for the directory in this order:
     *
     *   1. Looks for "COMPOSITE_DATA_PATH" environment variable.
     *   2. Uses the compiled-in value (usually $PREFIX/share/composite/data/
     *
     * \return Path to the data directory.
     */
    QString DataPath::get_data_path()
    {
	if( ! __data_path.isEmpty() ) return __data_path;

	QString tmp;
	QFileInfo fi;

	char* env = getenv("COMPOSITE_DATA_PATH");
	if(env) {
	    fi.setFile( QString(env) );
	    if( fi.exists() ) {
		__data_path = fi.absoluteFilePath();
	    }
	    return __data_path;
	}

	__data_path = DATA_PATH;
	return __data_path;
    }

} // namespace Tritium
