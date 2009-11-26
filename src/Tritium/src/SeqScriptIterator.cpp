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

#include <Tritium/SeqScriptIterator.hpp>
#include <Tritium/SeqEvent.hpp>
#include "SeqScriptPrivate.hpp"

#include <cassert>



namespace Tritium
{

    // E stands for SeqEvent
    // S stands for SeqScript
    //
    // These abbreviations and the _Self macro are an attempt to make the code
    // READABLE.  :-)

#define _Self _SeqScriptIterator<E>

    template <typename E>
    _SeqScriptIterator<E>::_SeqScriptIterator() : d(0)
    {
	d = new _Internal;
    }

    template <typename E>
    _SeqScriptIterator<E>::_SeqScriptIterator(_Internal s) : d(0)
    {
	d = new _Internal(s);
    }

    template <typename E>
    _SeqScriptIterator<E>::_SeqScriptIterator(const _SeqScriptIterator<E>& o) : d(0)
    {
	d = new _Internal(*o.d);
    }

    template <typename E>
    _SeqScriptIterator<E>::~_SeqScriptIterator()
    {
	delete d;
	d = 0;
    }

    template <typename E>
    typename _Self::reference _SeqScriptIterator<E>::operator*() const
    {
	return static_cast<reference>((*d)->ev);
    }

    template <typename E>
    typename _Self::pointer _SeqScriptIterator<E>::operator->() const
    {
	return static_cast<pointer>(&((*d)->ev));
    }

    template <typename E>
    _Self& _SeqScriptIterator<E>::operator++()
    {
	++(*d);
	return (*this);
    }

    template <typename E>
    _Self _SeqScriptIterator<E>::operator++(int)
    {
	_Self tmp(*this);
	++(*d);
        return tmp;
    }

    template <typename E>
    bool _SeqScriptIterator<E>::operator!=(const _Self& o) const
    {
	return ((*d) != (*o.d));
    }

    template <typename E>
    bool _SeqScriptIterator<E>::operator==(const _Self& o) const
    {
	return ((*d) == (*o.d));
    }

    template <typename E>
    _Self& _SeqScriptIterator<E>::operator=(const _SeqScriptIterator<E>& o)
    {
	(*d) = (*o.d);
    }

    template class _SeqScriptIterator<SeqEvent>;
    template class _SeqScriptIterator<const SeqEvent>;

} // namespace Tritium
