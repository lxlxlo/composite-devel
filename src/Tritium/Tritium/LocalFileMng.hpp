/*
 * Copyright(c) 2002-2008 by Alex >Comix< Cominu [comix@users.sourceforge.net]
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

#ifndef TRITIUM_LOCALFILEMNG_HPP
#define TRITIUM_LOCALFILEMNG_HPP

#include <iostream>
#include <fstream>
#include <vector>

#include <QDomDocument>
#include <Tritium/memory.hpp>
#include <Tritium/ObjectBundle.hpp>

namespace Tritium
{

    class Note;
    class Instrument;
    class InstrumentList;
    class Sequence;
    class Pattern;
    class Song;
    class Drumkit;
    class EngineInterface;

    /**
     *
     */
    class LocalFileMng
    {
    public:
        LocalFileMng(EngineInterface* parent);
        ~LocalFileMng();

        /* Methods for determining where resource locations
         * are (e.g. the "data" directory.)
         */
        std::vector<QString> getDrumkitsFromDirectory( QString );
        std::vector<QString> getUserDrumkitList();
        std::vector<QString> getSystemDrumkitList();
        std::vector<QString> getPatternDirList();
        std::vector<QString> getSongList();
        std::vector<QString> getPatternsForDrumkit( const QString&  );
        std::vector<QString> getAllPatternName();
        QString getDrumkitDirectory( const QString& drumkitName );
        QString getDrumkitNameForPattern( const QString& patternDir );
        QString getCategoryFromPatternName( const QString& patternPathName );
        QString getPatternNameFromPatternDir( const QString& patternDirName);


        /* Methods for extracting metadata regarding the patterns
         * loaded in the patterns directory.
         */
        int getPatternList( const QString& );
        int mergeAllPatternList( std::vector<QString> );
        std::vector<QString> getallPatternList(){
            return m_allPatternList;
        }
        std::vector<QString> getAllCategoriesFromPattern();

        /* Methods for loading/saving files.
         *
         * Note that loadSong() and saveSong() are currently
         * handled by Tritium::Song.
         */
        T<Drumkit>::shared_ptr loadDrumkit( const QString& directory );
        int saveDrumkit( T<Drumkit>::shared_ptr pDrumkit );

        T<Pattern>::shared_ptr loadPattern( const QString& filename );
        int savePattern( T<Song>::shared_ptr song,
                         int selectedpattern,
                         const QString& patternname,
                         const QString& realpatternname,
                         int mode);

        int savePlayList( const std::string& patternname );
        int loadPlayList( const std::string& patternname);

        /* Static helper functions for reading/writing bits
         * of an XML file.
         */
        static void writeXmlString( QDomNode parent,
                                    const QString& name,
                                    const QString& text );
        static void writeXmlBool( QDomNode parent,
                                  const QString& name,
                                  bool value );

        static QString readXmlString( QDomNode node,
                                      const QString& nodeName,
                                      const QString& defaultValue,
                                      bool bCanBeEmpty = false,
                                      bool bShouldExists = true,
                                      bool tinyXmlCompatMode = false);
        static float readXmlFloat( QDomNode node,
                                   const QString& nodeName,
                                   float defaultValue,
                                   bool bCanBeEmpty = false,
                                   bool bShouldExists = true,
                                   bool tinyXmlCompatMode = false);
        static int readXmlInt( QDomNode node,
                               const QString& nodeName,
                               int defaultValue,
                               bool bCanBeEmpty = false,
                               bool bShouldExists = true,
                               bool tinyXmlCompatMode = false);
        static bool readXmlBool( QDomNode node,
                                 const QString& nodeName,
                                 bool defaultValue,
                                 bool bShouldExists = true,
                                 bool tinyXmlCompatMode = false);
        static void convertFromTinyXMLString( QByteArray* str );
        static bool checkTinyXMLCompatMode( const QString& filename );
        static QDomDocument openXmlDocument( const QString& filename );

    private:
        EngineInterface* m_engine;
        void fileCopy( const QString& sOrigFilename,
                       const QString& sDestFilename );
        std::vector<QString> m_allPatternList;
    };

} // namespace Tritium

#endif // TRITIUM_LOCALFILEMNG_HPP
