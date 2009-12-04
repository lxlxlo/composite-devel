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

#include <list>
#include <QString>

namespace Tritium
{
    class Song;
    class Instrument;
    class Drumkit;
    class Pattern;

    /**
     * \brief Several functions for loading resources.
     *
     * These functions behave asynchronously, loading objects and
     * allocating memory in a dedicated thread.  It also abstracts
     * access to resources... looking toward a future system where
     * resources may be files or database keys.  This makes it
     * possible to call these functions from a realtime thread, and
     * provide a notification means to deliver the goods.
     */
    namespace Serialization
    {
	class LoadBundle;
	class SaveReport;
	void load_file(const QString& filename,
		       LoadBundle& report_to);

	bool save_song(const QString& filename,
		       Song& song,
		       SaveReport& report_to,
		       bool overwrite = false);
	bool save_drumkit(const QString& filename,
			  Drumkit& song,
			  SaveReport& report_to,
			  bool overwrite = false);
	bool save_pattern(const QString& filename,
			  Pattern& pattern,
			  SaveReport& report_to,
			  bool overwrite = false);

	/**
	 * \brief Container for various classes.
	 */
	struct LoadBundleObject
	{
	    typedef enum {
		Song_t = 0,
		Pattern_t,
		Instrument_t,
		_Reserved = 0xFF
	    } object_t;

	    object_t type;
	    void* ref;
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
	class LoadBundle
	{
	public:
	    typedef std::list<LoadBundleObject> list_t;

	    virtual ~LoadBundle() {}
	    virtual void operator()() = 0;

	    list_t objects;
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
	    const QString filename;
	    const QString message;
	    status_t status;
	};

    } // namespace Serialization

} // namespace Tritium

#endif // TRITIUM_SERIALIZATION_HPP
