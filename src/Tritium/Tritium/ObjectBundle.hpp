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
#ifndef TRITIUM_OBJECTBUNDLE_HPP
#define TRITIUM_OBJECTBUNDLE_HPP

#include <Tritium/memory.hpp>
#include <Tritium/Mixer.hpp>
#include <QString>
#include <list>

namespace Tritium
{
    class Instrument;
    class Pattern;
    class Song;
    class Drumkit;
    class LadspaFX;

    /**
     * \brief Container for various classes.
     */
    struct ObjectItem
    {
	typedef enum {
	    Song_t = 0,
	    Pattern_t,
	    Instrument_t,
	    LadspaFX_t,
	    Drumkit_t,
	    Channel_t,
	    _Reserved = 0xFF
	} object_t;

	object_t type;
	T<void>::shared_ptr ref;
    };

    /**
     * \brief Delivers the results of loading a file.
     *
     * Loading operations typically result in one or more new
     * classes being allocated, created, and initialized.  This
     * class is used to deliver these to the part of the program
     * that requested the load operation.
     *
     * Ownership of the objects is transferred to the class
     * derived from LoadBundle.  Wherever these objects are
     * delivered to, they are responsible for deleting them.
     *
     * Example:
     * \code
     * class SongLoaded : public Tritium::Serialization::LoadBundle
     * {
     *     virtual void operator()() {
     *         list_t::iterator k;
     *         Song *pSong; Pattern *pPattern; Instrument *pInstrument;
     *         for(k=objects.begin() ; k!=objects.end() ; ++k) {
     *             switch(k->type) {
     *             case Song_t:
     *                 pSong = static_cast<Song*>(k->ref);
     *                 // handle new song obj.
     *                 break;
     *             case Pattern_t:
     *                 pPattern = static_cast<Pattern*>(k->ref);
     *                 // handle new pattern obj.
     *                 break;
     *             case Instrument_t:
     *                 pInstrument = static_cast<Instrument*>(k->ref);
     *                 // handle new instrument obj.
     *                 break;
     *             default:
     *                 assert(false); // This is a logic error.
     *                 delete k->ref;
     *             };
     *         }
     *     }
     * };
     * \endcode
     *
     * Note the assert() for the default case.  If a load
     * operation results in a type that you were not expecting,
     * this is an error.
     */
    class ObjectBundle
    {
    public:
	typedef std::list<ObjectItem> list_t;

	ObjectBundle() : error(false) {}
	virtual ~ObjectBundle() {}
	// A hook to a callback.
	virtual void operator()() {}

	list_t objects;
	bool error;
	QString error_message;

	/* Helper methods (creation)
	 */
	void push(T<Song>::shared_ptr obj) {
	    ObjectItem tmp = {ObjectItem::Song_t, T<void>::shared_ptr(obj)};
	    objects.push_back(tmp);
	}
	void push(T<Pattern>::shared_ptr obj) {
	    ObjectItem tmp = {ObjectItem::Pattern_t, T<void>::shared_ptr(obj)};
	    objects.push_back(tmp);
	}
	void push(T<Instrument>::shared_ptr obj) {
	    ObjectItem tmp = {ObjectItem::Instrument_t, T<void>::shared_ptr(obj)};
	    objects.push_back(tmp);
	}
	void push(T<LadspaFX>::shared_ptr obj) {
	    ObjectItem tmp = {ObjectItem::LadspaFX_t, T<void>::shared_ptr(obj)};
	    objects.push_back(tmp);
	}
	void push(T<Drumkit>::shared_ptr obj) {
	    ObjectItem tmp = {ObjectItem::Drumkit_t, T<void>::shared_ptr(obj)};
	    objects.push_back(tmp);
	}
	void push(T<Mixer::Channel>::shared_ptr obj) {
	    ObjectItem tmp = {ObjectItem::Channel_t, T<void>::shared_ptr(obj)};
	    objects.push_back(tmp);
	}
	/* Helper methods (access)
	 */
	bool empty() {
	    return objects.empty();
	}
	ObjectItem::object_t peek_type() {
	    return objects.front().type;
	}

	template <typename X>
	typename T<X>::shared_ptr pop() {
	    typedef typename T<X>::shared_ptr XSP;
	    XSP rv;
	    rv = boost::static_pointer_cast< X, void >(objects.front().ref);
	    objects.pop_front();
	    return rv;
	}

	/** Deletes the object on the top of the stack.
	 */
	void pop() {
	    objects.pop_front();
	}

    };

} // namespace Tritium

#endif // TRITIUM_OBJECTBUNDLE_HPP
