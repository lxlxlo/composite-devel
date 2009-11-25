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
#ifndef H2CORE_SEQSCRIPTPRIVATE_H
#define H2CORE_SEQSCRIPTPRIVATE_H

#include <Tritium/SeqEvent.h>
#include <vector>

namespace Tritium
{

    struct SeqEventWrap
    {
	typedef std::vector<SeqEventWrap>::iterator internal_iterator;

	SeqEvent ev;
	internal_iterator next;
	internal_iterator me;
	bool used;

	SeqEventWrap() : ev(), next(0), used(false) {}
    }; // struct SeqEventWrap

    class SeqEventWrapIterator  // a bidirectional, Input iterator
    {
    public:
	typedef SeqEventWrapIterator            _Self;
	typedef SeqEventWrap::internal_iterator internal_iterator;
	typedef SeqEventWrap                    value_type;
	typedef SeqEventWrap*                   pointer;
	typedef SeqEventWrap&                   reference;

	SeqEventWrapIterator() : m_pos(0) {}
	//SeqEventWrapIterator(pointer p) : m_pos(p) {}
	SeqEventWrapIterator(const SeqEventWrapIterator& o) : m_pos(o.m_pos) {}
	SeqEventWrapIterator(internal_iterator i) : m_pos(i) {}

	void reset(internal_iterator p) {
	    m_pos = p;
	}

	reference operator*() {
	    return *m_pos;
	}

	pointer operator->() {
	    return &(*m_pos);
	}

	_Self& operator++() {
	    m_pos = m_pos->next;
	    return *this;
	}

	_Self operator++(int) {
	    _Self tmp = *this;
	    m_pos = m_pos->next;
	    return tmp;
	}

	bool operator==(const _Self& o) const {
	    return m_pos == o.m_pos;
	}

	bool operator!=(const _Self& o) const {
	    return m_pos != o.m_pos;
	}

    private:
	internal_iterator m_pos;
    };  // class SeqEventWrapIterator

    class SeqScriptPrivate
    {
    public:
	typedef std::vector<SeqEventWrap>              internal_sequence_type;
	typedef internal_sequence_type::iterator       internal_iterator;
	typedef internal_sequence_type::const_iterator const_internal_iterator;
	typedef SeqEventWrapIterator                   iterator;
	typedef SeqEvent::frame_type                   frame_type;

    private:
	internal_sequence_type m_vec;     // Data storage
	iterator m_head, m_tail;          // Head and tail of linked list
	internal_iterator m_next_free;    // Pointer to next free location
	size_t m_list_size;               // Number of elements in list
	size_t m_free;                    // Amount of free storage avail.
	QMutex m_mutex;

    public:
	SeqScriptPrivate(size_t reserved = 1024);
	~SeqScriptPrivate();

	// Info about events stored.
	size_t size();
	bool empty();
	size_t max_size();

	// Memory reservations.  Not real-time safe,
	// and it clears out all current data.
	void reserve(size_t events);

	// Insertion and deletion
	void insert(const SeqEvent& event);
	void remove(const SeqEvent& event);
	void remove(iterator pos);
	void clear();
	
	// Iterator access;
	iterator begin();
	iterator end();

	// Frame adjustment
	void consumed(frame_type before_frame);

    private:
	// These two presume that mutex is locked.
	internal_iterator alloc();  // Allocates and returns next pos. to be used.
	void insert(internal_iterator pos); // Inserts an allocated location into the list.
    };

}

#endif // H2CORE_SEQSCRIPTPRIVATE_H
