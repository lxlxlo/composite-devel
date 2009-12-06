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
#ifndef TRITIUM_WORKERTHREAD_HPP
#define TRITIUM_WORKERTHREAD_HPP

#include <QThread>
#include <QMutex>
#include <set>
#include <memory>

namespace Tritium
{
    /**
     * \brief abstract class to contain a WorkerThread module.
     */
    class WorkerThreadClient
    {
    public:
	virtual ~WorkerThreadClient() {}

	/**
	 * \brief Signal that there are events waiting to be processed.
	 *
	 * process() will only be called if this returns true.
	 */
	virtual bool events_waiting() = 0;

	/**
	 * \brief Process any active events.
	 */
	virtual int process() = 0;

	/**
	 * \brief End processing ASAP, thread is shutting down.
	 *
	 * This function should return _immediately_.
	 */
	virtual void shutdown() = 0;
    };

    class WorkerThread : public QThread
    {
    public:
	typedef WorkerThreadClient* pointer_t;
	typedef std::set<pointer_t> client_list_t;

	WorkerThread();
	virtual ~WorkerThread();

	void add_client(WorkerThreadClient* module);
	void shutdown();
	void run();

    private:
	QMutex m_mutex;
	client_list_t m_clients;
	bool m_kill;
    };

} // namespace Tritium

#endif // TRITIUM_WORKERTHREAD_HPP
