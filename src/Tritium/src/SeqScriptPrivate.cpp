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

#include <Tritium/SeqEvent.hpp>

#include <vector>
#include <cassert>

#include "SeqScriptPrivate.hpp"

using namespace Tritium;

/**************************************************************
 * SeqScriptPrivate: The SeqScript implementation
 * ----------------------------------------------
 *
 * The idea for SeqScript is that it is a sorted container (sorted by
 * frame) that contains note-like events.  The events should be stored
 * in pre-allocated memory (like a circular buffer) and also be
 * otherwise real-time safe.  One big feature of the SeqScript is the
 * consumed() method that takes the first chunk of frames out of
 * circulation (i.e. they've all been processed).
 *
 * Implementing this will use a pre-allocated vector with an embedded
 * linked-list.  The vector is the pre-allocated block of memory.  We
 * keep track of the next free memory location for that.  The values
 * inside the vector will have a pointer to the "next" element in the
 * linked list.  Thus, we only need to keep track of the head and the
 * tail of the linked list.
 *
 **************************************************************
 */

SeqScriptPrivate::SeqScriptPrivate(size_t reserved) :
    m_vec(reserved),
    m_head(),
    m_tail(),
    m_list_size(0),
    m_free(reserved)
{
    QMutexLocker mx(&m_mutex);

    internal_iterator cur;
    for( cur = m_vec.begin() ; cur != m_vec.end() ; ++cur ) {
	cur->me = cur;
    }
    m_next_free = m_vec.begin();
    m_head.reset( alloc() );
    m_tail = m_head;
}

SeqScriptPrivate::~SeqScriptPrivate()
{
}

size_t SeqScriptPrivate::size()
{
    return m_list_size;
}

bool SeqScriptPrivate::empty()
{
    return m_list_size == 0;
}

size_t SeqScriptPrivate::max_size()
{
    return m_vec.size();
}

void SeqScriptPrivate::reserve(size_t events)
{
    QMutexLocker mx(&m_mutex);
    m_vec.clear();
    m_vec.reserve(events);
    m_vec.insert( m_vec.end(), events, SeqEventWrap() );
    m_list_size = 0;
    m_free = m_vec.size();

    internal_iterator cur;
    for( cur = m_vec.begin() ; cur != m_vec.end() ; ++cur ) {
	cur->me = cur;
    }
    m_next_free = m_vec.begin();
    m_head.reset(alloc());
    m_tail = m_head;
}

void SeqScriptPrivate::insert(const SeqEvent& event)
{
    QMutexLocker mx(&m_mutex);
    internal_iterator next = alloc();
    next->ev = event;
    insert(next);    
}

void SeqScriptPrivate::remove(const SeqEvent& event)
{
    iterator cur;
    for( cur=begin() ; cur!=end() ; ++cur ) {
	if( event == cur->ev ) {
	    remove(cur);
	}
    }
}

void SeqScriptPrivate::remove(iterator pos)
{
    QMutexLocker mx(&m_mutex);
    internal_iterator cur;

    if( (pos->me) == (m_head->me) ) {
	m_head = m_head->next;
	pos->used = false;
	++m_free;
	--m_list_size;
	return;
    }
    for(cur = m_vec.begin() ; cur != m_vec.end() ; ++cur ) {
	if( cur->next == pos->me ) {
	    break;
	}
    }
    if( cur == m_vec.end() ) {
	return;
    }
    if( cur->next == pos->me ) {
	cur->next = pos->next;
	pos->used = false;
	++m_free;
	--m_list_size;
    }
}

void SeqScriptPrivate::clear()
{
    QMutexLocker mx(&m_mutex);
    iterator cur;
    for( cur = m_head ; cur != m_tail ; ++cur ) {
	cur->used = false;
	++m_free;
	--m_list_size;
    }
    assert( m_list_size == 0 );
    m_next_free = m_vec.begin();
    m_head.reset( alloc() );
    m_tail = m_head;
}

SeqScriptPrivate::iterator SeqScriptPrivate::begin()
{
    return m_head;
}

SeqScriptPrivate::iterator SeqScriptPrivate::end()
{
    return m_tail;
}

void SeqScriptPrivate::consumed(SeqScriptPrivate::frame_type before_frame)
{
    QMutexLocker mx(&m_mutex);

    iterator cur = m_head;
    while( ((cur->me) != (m_tail->me)) && (cur->ev.frame < before_frame) ) {
	cur->used = false;
	++m_free;
	--m_list_size;
	++cur;
    }
    m_head = cur->me;

    for(cur = m_head ; (cur->me) != (m_tail->me) ; ++cur ) {
	cur->ev.frame -= before_frame;
    }
}

SeqScriptPrivate::internal_iterator SeqScriptPrivate::alloc()
{
    internal_iterator rv;
    if( m_free ) {
	m_next_free->used = true;
	rv = m_next_free;
	--m_free;
	while( m_free && m_next_free->used == true ) {
	    ++m_next_free;
	    if(m_next_free == m_vec.end()) m_next_free = m_vec.begin();
	}
    } else {
	assert(false);
    }    
    return rv;
}

void SeqScriptPrivate::insert(SeqScriptPrivate::internal_iterator pos)
{
    iterator cur;
    if( m_list_size == 0 ) {
	// Insert into empty list
	pos->next = m_tail->me;
	m_head = pos;
	++m_list_size;
    } else if(  pos->ev < m_head->ev ) {
	// Insert at front
	pos->next = m_head->me;
	m_head = pos;
	++m_list_size;
    } else {
	// Search for insertion point
	for( cur = m_head ; cur->me != m_tail->me ; ++cur ) {
	    if( (cur->next == m_tail->me) // Insert at end.
		|| (pos->ev < cur->next->ev) ) { 
		pos->next = cur->next;
		cur->next = pos->me;
		++m_list_size;
		break;
	    }
	}
    }
}
