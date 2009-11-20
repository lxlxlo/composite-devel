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
#ifndef H2CORE_SEQSCRIPTITERATOR_H
#define H2CORE_SEQSCRIPTITERATOR_H

#include "SeqEvent.h"

namespace Tritium
{
    class SeqEventWrapIterator;

    template <typename _Event>
    class _SeqScriptIterator
    {
    public:
        typedef _Event value_type;
        typedef _Event* pointer;
        typedef _Event& reference;
        typedef _SeqScriptIterator _Self;
        typedef SeqEventWrapIterator _Internal;

	_SeqScriptIterator();
        _SeqScriptIterator(_Internal s);
	_SeqScriptIterator(const _Self& o);
        virtual ~_SeqScriptIterator();

        reference operator*() const;
        pointer operator->() const;
        _Self& operator++();  // prefix
        _Self operator++(int);  // postfix

	bool operator!=(const _Self& o) const;
	bool operator==(const _Self& o) const;
	_Self& operator=(const _Self& o);

    private:
        _Internal* d;
    };

    typedef _SeqScriptIterator<SeqEvent> SeqScriptIterator;
    typedef _SeqScriptIterator<const SeqEvent> SeqScriptConstIterator;

}

#endif // H2CORE_SEQSCRIPTITERATOR_H
