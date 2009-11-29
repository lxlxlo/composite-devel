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

#ifndef TRITIUM_OBJECT_HPP
#define TRITIUM_OBJECT_HPP

#ifdef check
#undef check
#endif


#include <QtCore>
#include <map>
#include <vector>
#include <sstream>
#include <cassert>
#include <QMutex>

namespace Tritium
{

class LoggerThread;

/**
 * Class for writing logs to the console
 */
class Logger
{
public:
    /* The log level setting is internally a bit-masked integer.
     * These are the bits.  It is valid for the log level to *not*
     * be one of these explicitly... however, you won't get proper
     * syntax highlighting.  E.g. ~0 will log, but won't get any
     * syntax highlighting.  Same with Error|Warning.
     *
     * This also allows for (future) debugging profiles.  For
     * example, if you only want to turn on log messages in a
     * specific section of code, you might do Logger::log( 0x80,
     * ... ), and set the logging level to 0x80 or 0x81 (to
     * include Error logs) with your debugger.
     */
    typedef enum _log_level {
	None = 0,
	Error = 1,
	Warning = 2,
	Info = 4,
	Debug = 8,
	AELockTracing = 0x10
    } log_level_t;
    typedef std::list<QString> queue_t;

    static void create_instance();
    static Logger* get_instance() { assert(__instance); return __instance; }

    /** Destructor */
    ~Logger();

    static void set_logging_level( const char* level ); // May be None, Error, Warning, Info, or Debug
    static void set_log_level(unsigned lev) { __log_level = lev; }
    static unsigned get_log_level() { return __log_level; }

    void log( unsigned lev,
	      const char* funcname,
	      const QString& msg );

    friend class LoggerThread;

private:
    static Logger *__instance;

    /* __msg_queue needs to be a list type (e.g. std::list<>)
     * because of the following properties:
     *
     * - Constant time insertion/removal of elements
     * - Changing the list does not invalidate its iterators.
     *
     * However, the __mutex class member is here for safe access
     * to __msg_queue.  It should only be locked when you are
     * adding or removing elements to the END of the list.  This
     * works because:
     *
     * - Only one thread is referencing and removing elements
     *   from the beginning (the Logger thread).
     *
     * - While many threads are adding elements, they are only
     *   adding elements to the END of the list.
     *
     */
    QMutex __mutex;  // Lock for adding or removing elements only
    queue_t __msg_queue;
    static unsigned __log_level; // A bitmask of log_level_t
    bool __use_file;

    /** Constructor */
    Logger();
};


} // namespace Tritium

// LOG MACROS

/* __LOG_WRAPPER enables us to filter out log messages _BEFORE_ the
 * function call.  This is good, because it avoids QString
 * constructors for temporaries.
 */
#define __LOG_WRAPPER(lev, funct, msg) {				\
	if( Tritium::Logger::get_log_level() & (lev) ){			\
	    Tritium::Logger::get_instance()->log(			\
		(lev),							\
		(funct),						\
		(msg)							\
		);							\
	}								\
    }

#define INFOLOG(x) __LOG_WRAPPER( Tritium::Logger::Info, __PRETTY_FUNCTION__, (x) );
#define WARNINGLOG(x) __LOG_WRAPPER( Tritium::Logger::Warning, __PRETTY_FUNCTION__, (x) );
#define ERRORLOG(x) __LOG_WRAPPER( Tritium::Logger::Error, __PRETTY_FUNCTION__, (x) );

#endif // TRITIUM_OBJECT_HPP
