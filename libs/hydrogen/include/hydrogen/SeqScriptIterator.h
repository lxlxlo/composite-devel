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

namespace H2Core
{
    class SeqScript;

    template <typename _Event, typename _Script>
    class _SeqScriptIterator
    {
    public:
        typedef _Event value_type;
        typedef long difference_type;
        typedef _Event* pointer;
        typedef _Event& reference;
        typedef _SeqScriptIterator _Self;
        typedef _Script _Parent;

        _SeqScriptIterator(_Script& s);
        virtual ~_SeqScriptIterator();

        reference operator*() const;
        pointer operator->() const;
        _Self& operator++();  // prefix
        _Self operator++(int);  // postfix
        _Self& operator+=(difference_type n);
        _Self operator+(difference_type n) const;
        _Self& operator-=(difference_type n);
        _Self operator-(difference_type n) const;
        reference operator[](difference_type n) const;
    private:
        _Parent& q;
    };

    typedef _SeqScriptIterator<SeqEvent,
                                     SeqScript> SeqScriptIterator;
    typedef _SeqScriptIterator<const SeqEvent,
                                     const SeqScript> SeqScriptConstIterator;

}

#endif // H2CORE_SEQSCRIPTITERATOR_H
