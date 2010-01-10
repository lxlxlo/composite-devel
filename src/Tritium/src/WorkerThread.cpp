/*
 * Copyright(c) 2009 by Gabriel M. Beddingfield <gabriel@teuton.org>
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

#include "WorkerThread.hpp"
#include <QMutexLocker>
#include <iostream>
#include <typeinfo>

#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif


using namespace Tritium;

inline void wait_a_moment()
{
#ifdef WIN32
    ::Sleep(100); // milliseconds
#else
    usleep(100000); // microseconds
#endif

}

WorkerThread::WorkerThread() :
    m_kill(0)
{
}

WorkerThread::~WorkerThread()
{
    shutdown();

    QMutexLocker lock(&m_mutex);
    m_clients.clear();
}

/**
 * \brief Add a module to the worker thread.
 *
 */
void WorkerThread::add_client(WorkerThread::pointer_t module)
{
    pointer_t tmp(module);
    m_clients.insert(tmp);
}

/**
 * \brief Prep this thread and all clients to shut down.
 */
void WorkerThread::shutdown()
{
    client_list_t::iterator k;
    for(k=m_clients.begin() ; k!=m_clients.end() ; ++k) {
	(*k)->shutdown();
    }
    m_kill = true;
}

/**
 * \brief The main loop of the application.
 */
void WorkerThread::run()
{
    bool did_work, avail;
    int rv;
    client_list_t::iterator client;
    QMutexLocker lock(&m_mutex);
    lock.unlock();

    while( ! m_kill ) {
	lock.relock();
	if( ! m_kill ) {
	    did_work = false;
	    for(client=m_clients.begin() ; client!=m_clients.end() ; ++client ) {
		avail = (*client)->events_waiting();
		if(avail) {
		    // TODO: What do we do if a client returns non-zero?
		    rv = (*client)->process();
		    if(rv) {
			std::cerr << "ERROR: " << typeid(*client).name()
				  << " returned " << rv << std::endl;
		    }
		    did_work = true;
		}
	    }

	    if( ! did_work ) {
		wait_a_moment();
	    }
	}
	lock.unlock();
    } // for(;;)
}
