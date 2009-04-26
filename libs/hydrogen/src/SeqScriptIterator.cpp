
#include <hydrogen/SeqScriptIterator.h>
#include <hydrogen/SeqEvent.h>
#include <hydrogen/SeqScript.h>

#include <cassert>

#warning "SeqScriptIterator.cpp needs to be IMPLEMENTED."

namespace H2Core
{

    // E stands for SeqEvent
    // S stands for SeqScript
    //
    // These abbreviations and the _Self macro are an attempt to make the code
    // READABLE.  :-)

#define _Self _SeqScriptIterator<E,S>

    template <typename E, typename S>
    _SeqScriptIterator<E,S>::_SeqScriptIterator(S& s) : q(s)
    {
        assert(false);
    }

    template <typename E, typename S>
    _SeqScriptIterator<E,S>::~_SeqScriptIterator()
    {
        assert(false);
    }

    template <typename E, typename S>
    typename _Self::reference _SeqScriptIterator<E,S>::operator*() const
    {
        assert(false);
        return q.at(0);
    }

    template <typename E, typename S>
    typename _Self::pointer _SeqScriptIterator<E,S>::operator->() const
    {
        assert(false);
        return &(q.at(0));
    }

    template <typename E, typename S>
    _Self& _SeqScriptIterator<E,S>::operator++()
    {
        assert(false);
        return *this;
    }

    template <typename E, typename S>
    _Self _SeqScriptIterator<E,S>::operator++(int)
    {
        assert(false);
        return *this;
    }

    template <typename E, typename S>
    _Self& _SeqScriptIterator<E,S>::operator+=(difference_type n)
    {
        assert(false);
        return *this;
    }

    template <typename E, typename S>
    _Self _SeqScriptIterator<E,S>::operator+(difference_type n) const
    {
        assert(false);
        return *this;
    }

    template <typename E, typename S>
    _Self& _SeqScriptIterator<E,S>::operator-=(difference_type n)
    {
        assert(false);
        return *this;
    }

    template <typename E, typename S>
    _Self _SeqScriptIterator<E,S>::operator-(difference_type n) const
    {
        assert(false);
        return *this;
    }

    template <typename E, typename S>
    typename _Self::reference _SeqScriptIterator<E,S>::operator[](difference_type n) const
    {
        assert(false);
        return q.at(0);
    }

    template class _SeqScriptIterator<SeqEvent, SeqScript>;
    template class _SeqScriptIterator<const SeqEvent, const SeqScript>;


} // namespace H2Core
