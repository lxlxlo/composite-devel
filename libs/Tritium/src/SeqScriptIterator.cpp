
#include <Tritium/SeqScriptIterator.h>
#include <Tritium/SeqEvent.h>
#include "SeqScriptPrivate.h"

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
