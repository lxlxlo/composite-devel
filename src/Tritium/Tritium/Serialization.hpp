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
    class EngineInterface;

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

	    /**
	     * Loads the data pointed to by filename.
	     *
	     * The data is automatically detected based on the
	     * filename (e.g. *.h2song).  At this time, the function
	     * only supports loading drumkit.xml, .h2song, and
	     * .h2pattern files.
	     *
	     * This function returns immediately.  After the data has
	     * been loaded, report_to() will be called.
	     *
	     * \param filename relative or absolute path to data file.
	     * For drumkits, this needs to be the path to the
	     * drumkit.xml file (including 'drumkit.xml').
	     *
	     * \param report_to a function abject that carries the
	     * payload of the load operation.  The callback
	     * ObjectBundle::operator()() will be called after the data
	     * has been loaded so that it can be utilized.
	     *
	     * \param engine a pointer to a valid EngineInterface instance.
	     */
	    virtual void load_file(const QString& filename,
				   ObjectBundle& report_to,
				   EngineInterface *engine) = 0;

	    /**
	     * Saves a song/sequence to a file.
	     *
	     * The data in song is saved to the file pointed to by
	     * filename.
	     *
	     * \param filename the path and name to the file to be
	     * saved.  Filename should end in '.h2song'.  It will not
	     * be added.
	     *
	     * \param song pointer to the song object that will be saved.
	     *
	     * \param report_to a function object that reports when
	     * and whether the song was saved.  After saving (or not)
	     * then the callback SaveReport::operator()() will be
	     * called.
	     *
	     * \param engine a pointer to a valid EngineInterface instance.
	     *
	     * \param overwrite if true, will overwrite filename if it
	     * already exists.  If false, the save will fail if
	     * filename already exists.
	     */
	    virtual void save_song(const QString& filename,
				   T<Song>::shared_ptr song,
				   SaveReport& report_to,
				   EngineInterface *engine,
				   bool overwrite = false) = 0;

	    /**
	     * Saves a drumkit to a folder.
	     *
	     * The current drumkit data is saved to the folder dirname.
	     *
	     * \param dirname the path and name to the folder to be
	     * saved.  It should \em not contain 'drumkit.xml'.  (Note
	     * that this is different from load_file().)  It \em
	     * should be the exact folder where the kit should be
	     * saved.  This typically means that the last part of the
	     * path is the name of the drumkit.  If the folder does
	     * not already exist, it will be created.
	     *
	     * \param drumkit pointer to the drumkit object object
	     * that will be saved.
	     *
	     * \param report_to a function object that reports when
	     * and whether the drumkit was saved.  After saving (or
	     * not) then the callback SaveReport::operator()() will be
	     * called.
	     *
	     * \param engine a pointer to a valid EngineInterface instance.
	     *
	     * \param overwrite if true, will overwrite the contents
	     * of dirname if it already exists.  If false, the save
	     * will fail if dirname already exists.
	     */
	    virtual void save_drumkit(const QString& dirname,
				      T<Drumkit>::shared_ptr drumkit,
				      SaveReport& report_to,
				      EngineInterface *engine,
				      bool overwrite = false) = 0;

	    /**
	     * Saves a pattern/sequence to a file.
	     *
	     * The data in a pattern is saved to the file pointed to
	     * by filename.
	     *
	     * \param filename the path and name to the file to be
	     * saved.  Filename should end in '.h2pattern'.  It will
	     * not be added.
	     *
	     * \param pattern pointer to the song object that will be
	     * saved.
	     *
	     * \param drumkit_name the name of the drumkit that this
	     * pattern is associated with.
	     *
	     * \param report_to a function object that reports when
	     * and whether the song was saved.  After saving (or not)
	     * then the callback SaveReport::operator()() will be
	     * called.
	     *
	     * \param engine a pointer to a valid EngineInterface instance.
	     *
	     * \param overwrite if true, will overwrite filename if it
	     * already exists.  If false, the save will fail if
	     * filename already exists.
	     */
	    virtual void save_pattern(const QString& filename,
				      T<Pattern>::shared_ptr pattern,
				      const QString& drumkit_name,
				      SaveReport& report_to,
				      EngineInterface *engine,
				      bool overwrite = false) = 0;

	    /**
	     * \brief Creates a stand-alone serailizer object.
	     *
	     * Not real-time safe.
	     *
	     * Typically, you will want to use the serializer that
	     * comes from Tritium::Engine::get_serializer() so that
	     * the worker thread can be shared.  Sometimes you may
	     * wish to use the serializer without the EngineInterface, and this
	     * function returns a serializer object that can be used
	     * for this purpose.
	     */
	    static Serializer* create_standalone(EngineInterface *engine);
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
