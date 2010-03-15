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

#include "LoggerPrivate.hpp"
#include "WorkerThread.hpp"
#include <Tritium/Logger.hpp>
#include <Tritium/util.hpp>

#include <QDir>
#include <QThread>
#include <QMutex>
#include <QMutexLocker>
#include <iostream>
#include <cassert>
#include <strings.h>

#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

using namespace std;
using namespace Tritium;

Logger* Logger::__instance = 0;
static WorkerThread *worker_thread = 0;

/*********************************************************************
 * LoggerPrivate implementation
 *********************************************************************
 */

LoggerPrivate::LoggerPrivate(Logger* parent, bool use_file) :
    m_log_level(Logger::Error | Logger::Warning | Logger::Info),
    m_use_file(use_file),
    m_kill(false),
    m_logger(parent),
    m_logfile(0)
{
#ifdef WIN32
    ::AllocConsole();
    freopen( "CONOUT$", "wt", stdout );
#endif
    FILE *m_logfile = NULL;
    if( m_use_file ) {
	QString sLogFilename;
#ifdef Q_OS_MACX
	sLogFilename = QDir::homePath().append("/Library/Composite/composite.log");
#else
	sLogFilename = QDir::homePath().append("/.composite/composite.log");
#endif
	m_logfile = fopen( sLogFilename.toLocal8Bit(), "w" );
	if( m_logfile == 0 ) {
	    std::cerr << "Error: can't open log file for writing..." << endl;
	} else {
	    fprintf( m_logfile, "Start logger" );
	}
    }
}

LoggerPrivate::~LoggerPrivate()
{
    if(m_logfile) {
	fprintf( m_logfile, "Stop logger" );
	fclose( m_logfile );
    }
#ifdef WIN32
    ::FreeConsole();
#endif
}

bool LoggerPrivate::events_waiting()
{
    if(m_logger) {
	return !(m_msg_queue.empty());
    }
    return false;
}

void LoggerPrivate::shutdown()
{
    m_kill = true;
}

int LoggerPrivate::process()
{
    if( m_kill ) return 0;

    LoggerPrivate::queue_t& queue = m_msg_queue;
    LoggerPrivate::queue_t::iterator it, last;
    QString tmpString;
    for( it = last = queue.begin() ; (it != queue.end()) && (!m_kill) ; ++it ) {
	last = it;
	printf( it->toLocal8Bit() );
	if( m_logfile ) {
	    fprintf( m_logfile, it->toLocal8Bit() );
	}
    }
    if(m_kill)
	return 0;

    if( m_logfile ) fflush(m_logfile);
    queue.erase( queue.begin(), last );
    QMutexLocker lock(&m_mutex);
    if( ! queue.empty() ) queue.pop_front();
    return 0;
}

void LoggerPrivate::set_logging_level(const char* level)
{
	const char none[] = "None";
	const char error[] = "Error";
	const char warning[] = "Warning";
	const char info[] = "Info";
	const char debug[] = "Debug";
	bool use;
	unsigned log_level;

	// insert hex-detecting code here.  :-)

	if( 0 == strncasecmp( level, none, sizeof(none) ) ) {
		log_level = 0;
		use = false;
	} else if ( 0 == strncasecmp( level, error, sizeof(error) ) ) {
		log_level = Logger::Error;
		use = true;
	} else if ( 0 == strncasecmp( level, warning, sizeof(warning) ) ) {
		log_level = Logger::Error | Logger::Warning;
		use = true;
	} else if ( 0 == strncasecmp( level, info, sizeof(info) ) ) {
		log_level = Logger::Error | Logger::Warning | Logger::Info;
		use = true;
	} else if ( 0 == strncasecmp( level, debug, sizeof(debug) ) ) {
		log_level = Logger::Error | Logger::Warning | Logger::Info | Logger::Debug;
		use = true;
	} else {
		int val = Tritium::hextoi(level, -1);
		if( val == 0 ) {
			// Probably means hex was invalid.  Use -VNone instead.
			log_level = Logger::Error;
		} else {
			log_level = val;
			if( log_level & ~0x1 ) {
				use = true;
			} else {
				use = false;
			}
		}
	}

	Logger::set_log_level( log_level );
	// __use_log = use;
	// Logger::get_instance()->__use_file = use;
}

void LoggerPrivate::log( unsigned level,
			 const char* funcname,
			 const QString& msg )
{
    if( level == Logger::None ) return;

    const char* prefix[] = { "", "(E) ", "(W) ", "(I) ", "(D) " };
#ifdef WIN32
    const char* color[] = { "", "", "", "", "" };
#else
    const char* color[] = { "", "\033[31m", "\033[36m", "\033[32m", "" };
#endif // WIN32

    int i;
    switch(level) {
    case Logger::None:
	assert(false);
	i = 0;
	break;
    case Logger::Error:
	i = 1;
	break;
    case Logger::Warning:
	i = 2;
	break;
    case Logger::Info:
	i = 3;
	break;
    case Logger::Debug:
	i = 4;
	break;
    default:
	i = 0;
	break;
    }

    QString tmp;
    if(level != Logger::Info) {
	tmp = QString("%1%2%3\t%4 \033[0m\n")
	    .arg(color[i])
	    .arg(prefix[i])
	    .arg(funcname)
	    .arg(msg);
    } else {
	// The INFOLOG should be a very simple, user
	// feedback message.  It should /not/ have
	// function names or line numbers.
	tmp = msg;
    }

    QMutexLocker mx(&m_mutex);
    m_msg_queue.push_back( tmp );
}

/*********************************************************************
 * Logger implementation
 *********************************************************************
 */

void Logger::set_logging_level(const char* level)
{
    get_instance()->d->set_logging_level(level);
}

void Logger::create_instance()
{
	if ( __instance == 0 ) {
		__instance = new Logger;
	}
}

/**
 * Constructor
 */
Logger::Logger()
{
    __instance = this;
    T<LoggerPrivate>::shared_ptr lp( new LoggerPrivate(this, false) );
    d = lp.get();
    worker_thread = new WorkerThread();
    worker_thread->add_client(lp);
    worker_thread->start();
}

/**
 * Destructor
 */
Logger::~Logger()
{
    __instance = 0;
    worker_thread->shutdown();
    worker_thread->wait();
    delete worker_thread;
}

void Logger::log( unsigned level,
		  const char* funcname,
		  const QString& msg )
{
    get_instance()->d->log(level, funcname, msg);
}

void Logger::set_log_level(unsigned lev)
{
    get_instance()->d->set_log_level(lev);
}

unsigned Logger::get_log_level()
{
    return get_instance()->d->get_log_level();
}
