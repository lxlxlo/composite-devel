/*
 * Hydrogen
 * Copyright(c) 2002-2008 by Alex >Comix< Cominu [comix@users.sourceforge.net]
 *
 * http://www.hydrogen-music.org
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY, without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#ifndef H2_OBJECT_H
#define H2_OBJECT_H

#ifdef check
#undef check
#endif


#include <QtGui>
#include <map>
#include <vector>
#include <sstream>
#include <pthread.h>
#include <cassert>

class Object;

/**
 * Class for writing logs to the console
 */
class Logger
{
public:
	typedef enum _log_level {
		None = 0,
		Error = 1,
		Warning = 2,
		Info = 4,
		Debug = 8
	} log_level_t;
	typedef std::list<QString> queue_t;

	bool __use_file;
	bool __running;

	static void create_instance();
	static Logger* get_instance() { assert(__instance); return __instance; }

	/** Destructor */
	~Logger();

	void log( log_level_t lev, const char* funcname, const QString& class_name, const QString& msg );

	friend void* loggerThread_func(void* param);  // object.cpp

private:
	static Logger *__instance;
	pthread_mutex_t __mutex;  // Lock for adding or removing elements only
	queue_t __msg_queue;

	/** Constructor */
	Logger();

};


/**
 * Base class.
 */
class Object
{
public:
	static bool __use_log;

	/** Constructor */
	Object( const QString& className );
	Object( const Object& obj );

	/** Destructor */
	virtual ~Object();

	const QString& get_class_name() const {
		return __class_name;
	}

	static int get_objects_number();
	static void print_object_map();
	static void set_logging_level( const char* level ); // May be None, Error, Warning, Info, or Debug
	static bool is_using_verbose_log();

private:
	static unsigned __objects;
	static std::map<QString, int> __object_map;
	Logger *__logger;
	QString __class_name;
};

// LOG MACROS

namespace H2Core {
    extern unsigned logLevel; // see object.cpp for instance
}

/* __LOG_WRAPPER enables us to filter out log messages _BEFORE_ the
 * function call.  This is good, because it avoids QString
 * constructors for temporaries.
 */
#define __LOG_WRAPPER(lev, funct, class_n, msg) {			\
		if( H2Core::logLevel & (lev) ){				\
			Logger::get_instance()->log(			\
				(lev),					\
				(funct),				\
				(class_n),				\
				(msg)					\
				);					\
		}							\
	}

#define _INFOLOG(x) __LOG_WRAPPER( Logger::Info, __PRETTY_FUNCTION__, "", (x) );
#define _WARNINGLOG(x) __LOG_WRAPPER( Logger::Warning, __PRETTY_FUNCTION__, "", (x) );
#define _ERRORLOG(x) __LOG_WRAPPER( Logger::Error, __PRETTY_FUNCTION__, "", (x) );

#define INFOLOG(x) __LOG_WRAPPER( Logger::Info, __FUNCTION__, get_class_name(), (x) );
#define WARNINGLOG(x) __LOG_WRAPPER( Logger::Warning, __FUNCTION__, get_class_name(), (x) );
#define ERRORLOG(x) __LOG_WRAPPER( Logger::Error, __FUNCTION__, get_class_name(), (x) );




#endif // H2_OBJECT_H
