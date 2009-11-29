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
unsigned Logger::__log_level = 0;

void Logger::set_logging_level(const char* level)
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

namespace Tritium
{

    class LoggerThread : public QThread
    {
	bool m_done;
	Logger *pLogger;
    public:
	LoggerThread(Logger* d) :
	    m_done(false),
	    pLogger(d)
	    {}
	void shutdown() { m_done = true; }
	void run();
    };

    LoggerThread *loggerThread;

} // namespace Tritium

void LoggerThread::run()
{

#ifdef WIN32
	::AllocConsole();
//	::SetConsoleTitle( "Hydrogen debug log" );
	freopen( "CONOUT$", "wt", stdout );
#endif

	FILE *pLogFile = NULL;
	if ( pLogger->__use_file ) {
#ifdef Q_OS_MACX
		QString sLogFilename = QDir::homePath().append( "/Library/Composite/composite.log" );
#else
		QString sLogFilename = QDir::homePath().append( "/.composite/composite.log" );
#endif

		pLogFile = fopen( sLogFilename.toLocal8Bit(), "w" );
		if ( pLogFile == NULL ) {
			std::cerr << "Error: can't open log file for writing..." << std::endl;
		} else {
			fprintf( pLogFile, "Start logger" );
		}
	}

	QMutexLocker logger_mutex(&pLogger->__mutex);
	logger_mutex.unlock();
	while ( ! m_done ) {
#ifdef WIN32
		Sleep( 1000 );
#else
		usleep( 999999 );
#endif

		Logger::queue_t& queue = pLogger->__msg_queue;
		Logger::queue_t::iterator it, last;
		QString tmpString;
		for( it = last = queue.begin() ; it != queue.end() ; ++it ) {
			last = it;
			printf( it->toLocal8Bit() );
			if( pLogFile ) {
				fprintf( pLogFile, it->toLocal8Bit() );
				fflush( pLogFile );
			}
		}
		// See Logger.hpp for documentation on __mutex and when
		// it should be locked.
		queue.erase( queue.begin(), last );
		logger_mutex.relock();
		if( ! queue.empty() ) queue.pop_front();
		logger_mutex.unlock();
	}

	if ( pLogFile ) {
		fprintf( pLogFile, "Stop logger" );
		fclose( pLogFile );
	}
#ifdef WIN32
	::FreeConsole();
#endif


#ifdef WIN32
	Sleep( 1000 );
#else
	usleep( 100000 );
#endif

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
		: __use_file( false )
{
	__instance = this;
	loggerThread = new LoggerThread(this);
	loggerThread->start();
}

/**
 * Destructor
 */
Logger::~Logger()
{
	loggerThread->shutdown();
	loggerThread->wait();
	delete loggerThread;
	__instance = 0;
}

void Logger::log( unsigned level,
		  const char* funcname,
		  const QString& msg )
{
	if( level == None ) return;

	const char* prefix[] = { "", "(E) ", "(W) ", "(I) ", "(D) " };
#ifdef WIN32
	const char* color[] = { "", "", "", "", "" };
#else
	const char* color[] = { "", "\033[31m", "\033[36m", "\033[32m", "" };
#endif // WIN32

	int i;
	switch(level) {
	case None:
		assert(false);
		i = 0;
		break;
	case Error:
		i = 1;
		break;
	case Warning:
		i = 2;
		break;
	case Info:
		i = 3;
		break;
	case Debug:
		i = 4;
		break;
	default:
		i = 0;
		break;
	}

	QString tmp = QString("%1%2%3\t%4 \033[0m\n")
		.arg(color[i])
		.arg(prefix[i])
		.arg(funcname)
		.arg(msg);

	QMutexLocker mx(&__mutex);
	__msg_queue.push_back( tmp );
}