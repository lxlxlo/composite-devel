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
#ifndef TRITIUM_LOGGERPRIVATE_HPP
#define TRITIUM_LOGGERPRIVATE_HPP

#include "WorkerThread.hpp"
#include <Tritium/Logger.hpp>
#include <list>
#include <QString>

namespace Tritium
{
    class LoggerPrivate : public WorkerThreadClient
    {
    public:
	typedef std::list<QString> queue_t;

	LoggerPrivate(Logger* parent, bool use_file = false);
	virtual ~LoggerPrivate();

	virtual bool events_waiting();
	virtual int process();
	virtual void shutdown();

	// Implementation of Logger
	void set_logging_level(const char* level);
	void set_log_level(unsigned lev) { m_log_level = lev; }
	unsigned get_log_level() { return m_log_level; }
	void log( unsigned level,
		  const char* funcname,
		  const char* file,
		  unsigned line,
		  const QString& msg );

    private:
	/* m_msg_queue needs to be a list type (e.g. std::list<>)
	 * because of the following properties:
	 *
	 * - Constant time insertion/removal of elements
	 * - Changing the list does not invalidate its iterators.
	 *
	 * However, the m_mutex class member is here for safe access
	 * to m_msg_queue.  It should only be locked when you are
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
	QMutex m_mutex;       ///< Lock for adding or removing elements only
	queue_t m_msg_queue;
	unsigned m_log_level; ///< A bitmask of log_level_t
	bool m_use_file;

	bool m_kill;
	Logger *m_logger;
	FILE *m_logfile;
    };

} // namespace Tritium

#endif // TRITIUM_LOGGERPRIVATE_HPP
