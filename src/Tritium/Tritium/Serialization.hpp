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
#ifndef TRITIUM_SERIALIZATION_HPP
#define TRITIUM_SERIALIZATION_HPP

#include <Tritium/memory.hpp>
#include <list>
#include <QString>

namespace Tritium
{
    class Song;
    class Instrument;
    class Drumkit;
    class Pattern;
    class ObjectBundle;
    class Engine;

    namespace Serialization
    {
	class SaveReport;

        /**
	 * \brief Class that handles loading and saving objects.
	 *
	 * In order to load and save songs, patterns, drumkits, etc.
	 * in real-time, they are handled asynchronously by Tritium.
	 * In order to load/save files, there must also be an instance
	 * of a worker thread.
	 *
	 */
	class Serializer
	{
	public:
	    virtual ~Serializer() {}

	    virtual void load_file(const QString& filename,
				   ObjectBundle& report_to,
				   Engine *engine) = 0;

	    virtual void save_song(const QString& filename,
				   T<Song>::shared_ptr song,
				   SaveReport& report_to,
				   Engine *engine,
				   bool overwrite = false) = 0;
	    virtual void save_drumkit(const QString& filename,
				      T<Drumkit>::shared_ptr song,
				      SaveReport& report_to,
				      Engine *engine,
				      bool overwrite = false) = 0;
	    virtual void save_pattern(const QString& filename,
				      T<Pattern>::shared_ptr pattern,
				      QString drumkit_name,
				      SaveReport& report_to,
				      Engine *engine,
				      bool overwrite = false) = 0;

	    /**
	     * \brief Creates a stand-alone serailizer object.
	     *
	     * Not real-time safe.
	     *
	     * Typically, you will want to use the serializer that
	     * comes from Tritium::Engine::get_serializer() so that
	     * the worker thread can be shared.  Sometimes you may
	     * wish to use the serializer without the Engine, and this
	     * function returns a serializer object that can be used
	     * for this purpose.
	     */
	    static Serializer* create_standalone(Engine *engine);
	};

	/**
	 * \brief Function Object (functor) with status of save operation
	 *
	 * Derive from this class and overload operator().  When the
	 * save is complete, the results will be sent by calling
	 * operator().  For example:
	 *
	 * \code
	 * class MyReport : public Tritium::Serialization::SaveReport
	 * {
	 * public:
	 *     operator()() {
	 *         switch(status) {
	 *         case SaveFailed:
	 *             cerr << "ERROR: Save operation failed for "
	 *                  << filename.toStdString() << endl;
	 *             cerr << message.toStdString() << endl;
	 *             // Take corrective actions.
	 *             break;
	 *         case SaveSuccess:
	 *             // No actions necc.
	 *             break;
	 *         default:
	 *             // Some future status we can't handle.
         *         }
	 *     }
	 * \endcode
	 */
	class SaveReport
	{
	public:
	    typedef enum {
		SaveFailed = 0,
		SaveSuccess,
		_Reserved = 0xFF
	    } status_t;

	    virtual ~SaveReport() {}
	    virtual void operator()() = 0;
	    QString filename;
	    QString message;
	    status_t status;
	};

    } // namespace Serialization

} // namespace Tritium

#endif // TRITIUM_SERIALIZATION_HPP
