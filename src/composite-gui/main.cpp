/*
 * Copyright(c) 2010 by Gabriel M. Beddingfield <gabriel@teuton.org>
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

#include <QtGui/QApplication>
#include <Composite/Main/MainWidget.hpp>
#include <Composite/Looks/Colors.hpp>

#include <Tritium/Engine.hpp>
#include <Tritium/Logger.hpp>
#include <Tritium/Preferences.hpp>

// Temporary header:
#include <Tritium/LocalFileMng.hpp>

#include <cassert>

int main(int argc, char* argv[])
{
    QApplication qapp(argc, argv);
    Composite::Main::MainWidget mainwin(argc, argv);

    QPalette pal = Composite::Looks::create_default_palette();
    qapp.setPalette(pal);

    // Set up audio engine
    Tritium::Logger::create_instance();
    Tritium::Logger::get_instance()->set_logging_level( "Debug" );
    Tritium::T<Tritium::Preferences>::shared_ptr prefs( new Tritium::Preferences() );
    Tritium::Engine engine(prefs);

    /////////////////////////////////////////////
    // Temporary code to get GMkit loaded
    /////////////////////////////////////////////
    {
	Tritium::LocalFileMng loc( &engine );
	QString gmkit;
	std::vector<QString>::iterator it;
	std::vector<QString> list;
	list = loc.getSystemDrumkitList();
	for( it = list.begin() ; it < list.end() ; ++it ) {
	    if( (*it).endsWith("GMkit") ) {
		gmkit = *it;
	    }
	    break;
	}
	assert( ! gmkit.isNull() );
	Tritium::T<Tritium::Drumkit>::shared_ptr dk = loc.loadDrumkit( gmkit );
	assert( dk );
	engine.loadDrumkit( dk );
    }

    /////////////////////////////////////////////
    // End of temporary code
    /////////////////////////////////////////////

    mainwin.set_audio_engine( &engine );

    mainwin.show();

    return qapp.exec();
}
