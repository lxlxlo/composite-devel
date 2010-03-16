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

#include "config.h"
#include "version.h"


#include <Tritium/Logger.hpp>
#include <Tritium/ADSR.hpp>
#include <Tritium/DataPath.hpp>
#include <Tritium/EngineInterface.hpp>
#include <Tritium/Engine.hpp>
#include <Tritium/H2Exception.hpp>
#include <Tritium/Instrument.hpp>
#include <Tritium/InstrumentLayer.hpp>
#include <Tritium/InstrumentList.hpp>
#include <Tritium/LocalFileMng.hpp>
#include <Tritium/Note.hpp>
#include <Tritium/Pattern.hpp>
#include <Tritium/Preferences.hpp>
#include <Tritium/Song.hpp>
#include <Tritium/SoundLibrary.hpp>
#include <Tritium/Sample.hpp>
#include <Tritium/Sampler.hpp>
#include <Tritium/ObjectBundle.hpp>
#include <Tritium/Serialization.hpp>
#include <Tritium/fx/Effects.hpp>
#include <Tritium/memory.hpp>

#include <cstdlib>
#include <cassert>
#include <sys/stat.h>
#include <ctype.h>

#include <QDir>
#include <QApplication>
#include <QVector>
#include <QDomDocument>
#include <QLocale>


#include <algorithm>
#include <memory>
//#include <cstdio>
//#include <vector>

using namespace Tritium::Serialization;

namespace Tritium
{
    // class for synchronously loading/saving files
    // using the asynchronous Serializer.
    class SyncBundle : public ObjectBundle
    {
    public:
        bool done;

        SyncBundle() : done(false) {}
        void operator()() { done = true; }
    };

    class SyncSaveReport : public SaveReport
    {
    public:
	bool done;

	SyncSaveReport() : done(false) {}
	void operator()() { done = true; }
    };

    LocalFileMng::LocalFileMng(EngineInterface* parent) :
        m_engine(parent)
    {
        assert(parent);
    }



    LocalFileMng::~LocalFileMng()
    {
    }

    QString LocalFileMng::getDrumkitNameForPattern( const QString& patternDir )
    {
        QDomDocument doc = LocalFileMng::openXmlDocument( patternDir );

        QDomNode rootNode = doc.firstChildElement( "drumkit_pattern" ); // root element
        if (  rootNode.isNull() ) {
            ERRORLOG( "Error reading Pattern: Pattern_drumkit_infonode not found " + patternDir);
            return NULL;
        }

        return LocalFileMng::readXmlString( rootNode,"pattern_for_drumkit", "" );
    }


    QString LocalFileMng::getCategoryFromPatternName( const QString& patternPathName )
    {
        QDomDocument doc = LocalFileMng::openXmlDocument( patternPathName );


        QDomNode rootNode = doc.firstChildElement( "drumkit_pattern" ); // root element
        if ( rootNode.isNull() ) {
            ERRORLOG( "Error reading Pattern: Pattern_drumkit_info node not found ");
            return NULL;
        }

        QDomNode patternNode = rootNode.firstChildElement( "pattern" );

        return LocalFileMng::readXmlString( patternNode,"category", "" );

    }

    QString LocalFileMng::getPatternNameFromPatternDir( const QString& patternDirName)
    {
        QDomDocument doc = LocalFileMng::openXmlDocument( patternDirName );


        QDomNode rootNode =doc.firstChildElement( "drumkit_pattern" );  // root element
        if ( rootNode.isNull() ) {
            ERRORLOG( "Error reading Pattern: Pattern_drumkit_info node not found ");
            return NULL;
        }

        QDomNode patternNode = rootNode.firstChildElement( "pattern" );

        return LocalFileMng::readXmlString( patternNode,"pattern_name", "" );

    }


    T<Pattern>::shared_ptr LocalFileMng::loadPattern( const QString& filename )
    {
	T<Serializer>::auto_ptr serializer;
	SyncBundle bdl;

	serializer.reset( Serializer::create_standalone(m_engine) );
	serializer->load_uri(filename, bdl, m_engine);

	while( ! bdl.done ) {
	    sleep(1);
	}

	T<Pattern>::shared_ptr rv;

	if( bdl.error ) {
	    ERRORLOG(bdl.error_message);
	    return rv;
	}

	while( ! bdl.empty() ) {
	    switch(bdl.peek_type()) {
	    case ObjectItem::Pattern_t:
		if( ! rv ) {
		    rv = bdl.pop<Pattern>();
		} else {
		    ERRORLOG("Loading pattern returned more than one.");
		    bdl.pop();
		}
		break;
	    default:
		ERRORLOG("Loading pattern also loaded an unexpected type.");
		bdl.pop();
	    }
	}

	return rv;
    }


    /**
     * This function is now a front-end for Serializer::save_pattern().
     *
     * mode: 1 == save, 2 == save as, 3 == save, but don't verify
     *
     * Returns: 0 if all OK, non-zero on error.
     */
    int LocalFileMng::savePattern( T<Song>::shared_ptr song , int selectedpattern , const QString& patternname, const QString& realpatternname, int mode)
    {
	T<Serializer>::auto_ptr serializer;
	SyncSaveReport save_report;

	serializer.reset( Serializer::create_standalone(m_engine) );

        T<Pattern>::shared_ptr pat = song->get_pattern_list()->get( selectedpattern );
        T<Instrument>::shared_ptr instr = m_engine->get_sampler()->get_instrument_list()->get( 0 );
        assert( instr );
	QString drumkit_name = instr->get_drumkit_name();
	QString data_directory = m_engine->get_preferences()->getDataDirectory();

	QString sPatternDir = data_directory + "patterns/" + drumkit_name;
        DEBUGLOG( "[savePattern]" + sPatternDir );

        // check if the directory exists
        QDir dir( sPatternDir );
        if ( !dir.exists() ) {
            dir.mkdir( sPatternDir );// create the drumkit directory
        }

        QString sPatternXmlFilename;
        // create the drumkit.xml file
        switch ( mode ){
        case 1: //save
            sPatternXmlFilename = sPatternDir + "/" + QString( patternname + QString( ".h2pattern" ));
            break;
        case 2: //save as
            sPatternXmlFilename = patternname;
            break;
        case 3: //"save" but overwrite a existing pattern. mode 3 disable the last file exist check
            sPatternXmlFilename = sPatternDir + "/" + QString( patternname + QString( ".h2pattern" ));
            break;
        default:
            DEBUGLOG( "Pattern Save unknown status");
	    sPatternXmlFilename = patternname;
            break;
        }

        QFile testfile( sPatternXmlFilename );
        if ( testfile.exists() && mode == 1)
            return 1;

	bool overwrite = true;
	if( mode == 2 || mode > 3 ) {
	    overwrite = false;
	}
	serializer->save_pattern(sPatternXmlFilename,
				 pat,
				 drumkit_name,
				 save_report,
				 m_engine,
				 overwrite);

	while( ! save_report.done ) {
	    sleep(1);
	}

	int rv;
	switch( save_report.status ) {
	case SaveReport::SaveSuccess:
	    rv = 0;
	    break;
	case SaveReport::SaveFailed:
	default:
	    rv = 1;
	    ERRORLOG( QString("Error saving file %1: %2")
		      .arg(save_report.filename)
		      .arg(save_report.message) );
	}

	return rv;
    }




    void LocalFileMng::fileCopy( const QString& sOrigFilename, const QString& sDestFilename )
    {
        // TODO: use QT copy functions

        DEBUGLOG( sOrigFilename + " --> " + sDestFilename );

        if ( sOrigFilename == sDestFilename ) {
            return;
        }

        FILE *inputFile = fopen( sOrigFilename.toLocal8Bit(), "rb" );
        if ( inputFile == NULL ) {
            ERRORLOG( "Error opening " + sOrigFilename );
            return;
        }

        FILE *outputFile = fopen( sDestFilename.toLocal8Bit(), "wb" );
        if ( outputFile == NULL ) {
            ERRORLOG( "Error opening " + sDestFilename );
            return;
        }

        const int bufferSize = 512;
        char buffer[ bufferSize ];
        while ( feof( inputFile ) == 0 ) {
            size_t read = fread( buffer, sizeof( char ), bufferSize, inputFile );
            fwrite( buffer, sizeof( char ), read, outputFile );
        }

        fclose( inputFile );
        fclose( outputFile );
    }



    std::vector<QString> LocalFileMng::getSongList()
    {
        std::vector<QString> list;
        QString sDirectory = m_engine->get_preferences()->getDataDirectory();

        if( ! sDirectory.endsWith("/") ) {
            sDirectory += "/songs/";
        } else {
            sDirectory += "songs/";
        }

        QDir dir( sDirectory );

        if ( !dir.exists() ) {
            ERRORLOG( QString( "[getSongList] Directory %1 not found" ).arg( sDirectory ) );
        } else {
            dir.setFilter( QDir::Files );
            QFileInfoList fileList = dir.entryInfoList();

            for ( int i = 0; i < fileList.size(); ++i ) {
                QString sFile = fileList.at( i ).fileName();

                if ( ( sFile == "." ) || ( sFile == ".." ) || ( sFile == "CVS" )  || ( sFile == ".svn" ) ) {
                    continue;
                }

                list.push_back( sFile.left( sFile.indexOf( "." ) ) );
            }
        }

        return list;
    }

    int LocalFileMng::getPatternList( const QString&  sPatternDir)
    {
        std::vector<QString> list;
        QDir dir( sPatternDir );

        if ( !dir.exists() ) {
            ERRORLOG( QString( "[getPatternList] Directory %1 not found" ).arg( sPatternDir ) );
        } else {
            dir.setFilter( QDir::Files );
            QFileInfoList fileList = dir.entryInfoList();

            for ( int i = 0; i < fileList.size(); ++i ) {
                QString sFile = sPatternDir + "/" + fileList.at( i ).fileName();

                if( sFile.endsWith(".h2pattern") ){
                    list.push_back( sFile/*.left( sFile.indexOf( "." ) )*/ );
                }
            }
        }
        mergeAllPatternList( list );
        return 0;
    }


    std::vector<QString> LocalFileMng::getAllPatternName()
    {
        std::vector<QString> alllist;

        for (uint i = 0; i < m_allPatternList.size(); ++i) {
            QString patternInfoFile =  m_allPatternList[i];

            QDomDocument doc  = LocalFileMng::openXmlDocument( patternInfoFile );

            QDomNode rootNode =  doc.firstChildElement( "drumkit_pattern" );    // root element
            if ( rootNode.isNull() ) {
                ERRORLOG( "Error reading Pattern: Pattern_drumkit_info node not found ");
            }else{
                QDomNode patternNode = rootNode.firstChildElement( "pattern" );

                QString sPatternName( LocalFileMng::readXmlString( patternNode,"pattern_name", "" ) );
                alllist.push_back(sPatternName);
            }

        }
        return alllist;
    }



    std::vector<QString> LocalFileMng::getAllCategoriesFromPattern()
    {
        T<Preferences>::shared_ptr pPref = m_engine->get_preferences();
        std::list<QString>::const_iterator cur_testpatternCategories;

        std::vector<QString> categorylist;
        for (uint i = 0; i < m_allPatternList.size(); ++i) {
            QString patternInfoFile =  m_allPatternList[i];

            QDomDocument doc  = LocalFileMng::openXmlDocument( patternInfoFile );


            QDomNode rootNode = doc.firstChildElement( "drumkit_pattern" );     // root element
            if ( rootNode.isNull() ) {
                ERRORLOG( "Error reading Pattern: Pattern_drumkit_info node not found ");
            }else{
                QDomNode patternNode = rootNode.firstChildElement( "pattern" );
                QString sCategoryName( LocalFileMng::readXmlString( patternNode,"category", "" ) );


                if ( !sCategoryName.isEmpty() ){
                    bool test = true;
                    for (uint i = 0; i < categorylist.size(); ++i){
                        if ( sCategoryName == categorylist[i] ){
                            test = false;
                        }
                    }
                    if (test == true){
                        categorylist.push_back(sCategoryName);

                        //this merge new categories to user categories list
                        bool test2 = true;
                        for( cur_testpatternCategories = pPref->m_patternCategories.begin(); cur_testpatternCategories != pPref->m_patternCategories.end(); ++cur_testpatternCategories ){
                            if ( sCategoryName == *cur_testpatternCategories ){
                                test2 = false;
                            }
                        }

                        if (test2 == true ) {
                            pPref->m_patternCategories.push_back( sCategoryName );
                        }
                    }
                }
            }
        }

        std::sort(categorylist.begin(), categorylist.end());
        return categorylist;
    }



    std::vector<QString> LocalFileMng::getPatternsForDrumkit( const QString& sDrumkit )
    {
        std::vector<QString> list;

        QDir dir( m_engine->get_preferences()->getDataDirectory() + "/patterns/" + sDrumkit );

        if ( !dir.exists() ) {
            DEBUGLOG( QString( "No patterns for drumkit '%1'." ).arg( sDrumkit ) );
        } else {
            dir.setFilter( QDir::Dirs );
            QFileInfoList fileList = dir.entryInfoList();

            for ( int i = 0; i < fileList.size(); ++i ) {
                QString sFile = fileList.at( i ).fileName();

                if ( ( sFile == "." ) || ( sFile == ".." ) || ( sFile == "CVS" )  || ( sFile == ".svn" ) ) {
                    continue;
                }

                list.push_back( sFile.left( sFile.indexOf( "." ) ) );
            }
        }

        return list;
    }



    std::vector<QString> LocalFileMng::getDrumkitsFromDirectory( QString sDirectory )
    {
        /*
          returns a list of all drumkits in the given directory
        */

        std::vector<QString> list;

        QDir dir( sDirectory );
        if ( !dir.exists() ) {
            ERRORLOG( QString( "[getDrumkitList] Directory %1 not found" ).arg( sDirectory ) );
        } else {
            dir.setFilter( QDir::Dirs );
            QFileInfoList fileList = dir.entryInfoList();

            for ( int i = 0; i < fileList.size(); ++i ) {
                QString sFile = fileList.at( i ).fileName();
                if ( ( sFile == "." ) || ( sFile == ".." ) || ( sFile == "CVS" )
		     || ( sFile == ".svn" ) || (sFile =="songs" ) || ( sFile == "patterns" )
		     || (sFile == "drumkits") || (sFile == "playlists" ) || (sFile == "scripts" )
		     || (sFile == "presets") ) {
                    continue;
                }
                if(! sDirectory.endsWith("/")) sDirectory = sDirectory + "/";
                list.push_back( sDirectory + sFile );
            }
        }

        return list;
    }



    std::vector<QString> mergeQStringVectors( std::vector<QString> firstVector , std::vector<QString> secondVector )
    {
        /*
          merges two vectors ( containing drumkits). Elements of the first vector have priority
        */

        if( firstVector.size() == 0 ) return secondVector;
        if( secondVector.size() == 0 ) return firstVector;

        std::vector<QString> newVector;

        newVector = firstVector;
        newVector.resize(firstVector.size()+ secondVector.size());


        for ( int i = 0; i < (int)secondVector.size(); ++i )
        {
            QString toFind = secondVector[i];

            for ( int ii = 0; ii < (int)firstVector.size(); ++ii )
            {
                if( toFind == firstVector[ii])
                {
                    //the String already exists in firstVector, don't copy it to the resulting vector
                    break;
                }
            }
            newVector[firstVector.size() + i] = toFind;
        }

        return newVector;
    }


    std::vector<QString> LocalFileMng::getPatternDirList()
    {
        return getDrumkitsFromDirectory( m_engine->get_preferences()->getDataDirectory() + "patterns" );
    }


    int  LocalFileMng::mergeAllPatternList( std::vector<QString> current )
    {
        m_allPatternList = mergeQStringVectors (m_allPatternList, current );
        return 0;
    }



    std::vector<QString> LocalFileMng::getUserDrumkitList()
    {
        std::vector<QString> oldLocation = getDrumkitsFromDirectory( m_engine->get_preferences()->getDataDirectory() );
        std::vector<QString> newLocation = getDrumkitsFromDirectory( m_engine->get_preferences()->getDataDirectory() + "drumkits" );
        return mergeQStringVectors( newLocation ,  oldLocation );
    }

    std::vector<QString> LocalFileMng::getSystemDrumkitList()
    {
        return getDrumkitsFromDirectory( DataPath::get_data_path() + "/drumkits" );
    }


    QString LocalFileMng::getDrumkitDirectory( const QString& drumkitName )
    {
        // search in system drumkit
        std::vector<QString> systemDrumkits = Drumkit::getSystemDrumkitList(m_engine);
        for ( unsigned i = 0; i < systemDrumkits.size(); i++ ) {
            if ( systemDrumkits[ i ].endsWith(drumkitName) ) {
                QString path = QString( DataPath::get_data_path() ) + "/drumkits/";
                return path;
            }
        }

        // search in user drumkit
        std::vector<QString> userDrumkits = Drumkit::getUserDrumkitList(m_engine);
        for ( unsigned i = 0; i < userDrumkits.size(); i++ ) {
            if ( userDrumkits[ i ].endsWith(drumkitName) ) {
                QString path = m_engine->get_preferences()->getDataDirectory();
                return userDrumkits[ i ].remove(userDrumkits[ i ].length() - drumkitName.length(),drumkitName.length());
            }
        }

        ERRORLOG( "drumkit \"" + drumkitName + "\" not found" );
        return "";      // FIXME
    }

    T<Drumkit>::shared_ptr LocalFileMng::loadDrumkit( const QString& directory )
    {

        QString drumkitInfoFile = directory + "/drumkit.xml";

	T<Serializer>::auto_ptr serializer;
	SyncBundle bdl;

	serializer.reset( Serializer::create_standalone(m_engine) );
	serializer->load_uri(drumkitInfoFile, bdl, m_engine);

	while( ! bdl.done ) {
	    sleep(1);
	}

	T<Drumkit>::shared_ptr rv;

	if( bdl.error ) {
	    ERRORLOG(bdl.error_message);
	    return rv;
	}

	T<InstrumentList>::shared_ptr inst_list(new InstrumentList);

	while( ! bdl.empty() ) {
	    switch(bdl.peek_type()) {
	    case ObjectItem::Drumkit_t:
		if( ! rv ) {
		    rv = bdl.pop<Drumkit>();
		} else {
		    ERRORLOG("Loading drumkit returned more than one.");
		    bdl.pop();
		}
	    case ObjectItem::Instrument_t:
		inst_list->add( bdl.pop<Instrument>() );
		break;
	    case ObjectItem::Channel_t:
		assert(rv);
		rv->channels().push_back( bdl.pop<Mixer::Channel>() );
		break;
	    default:
		ERRORLOG("Loading pattern also loaded an unexpected type.");
		bdl.pop();
	    }
	}

	if( ! rv ) {
	    ERRORLOG("A drumkit object was not returned.");
	} else {
	    rv->setInstrumentList(inst_list);
	}

	return rv;
    }



    int LocalFileMng::saveDrumkit( T<Drumkit>::shared_ptr drumkit )
    {
        DEBUGLOG( "[saveDrumkit]" );

	T<Serializer>::auto_ptr serializer;
	SyncSaveReport save_report;

	serializer.reset( Serializer::create_standalone(m_engine) );

        QString sDrumkitDir = m_engine->get_preferences()->getDataDirectory() + "drumkits/" + drumkit->getName();

	serializer->save_drumkit(
	    sDrumkitDir,
	    drumkit,
	    save_report,
	    m_engine,
	    true
	    );

	while( ! save_report.done ) {
	    sleep(1);
	}

	int rv;
	if( save_report.status == SaveReport::SaveSuccess ) {
	    rv = 0;
	} else {
	    rv = -1;
	}

	return rv;
    }

    int LocalFileMng::savePlayList( const std::string& patternname)
    {

        std::string name = patternname.c_str();

        std::string realname = name.substr(name.rfind("/")+1);

        QDomDocument doc;
        QDomProcessingInstruction header = doc.createProcessingInstruction( "xml", "version=\"1.0\" encoding=\"UTF-8\"");
        doc.appendChild( header );

        QDomNode rootNode = doc.createElement( "playlist" );

        //LIB_ID just in work to get better usability
        writeXmlString( rootNode, "Name", QString (realname.c_str()) );
        writeXmlString( rootNode, "LIB_ID", "in_work" );

        QDomNode playlistNode = doc.createElement( "Songs" );
	Engine *eng;
	eng = dynamic_cast<Engine*>(m_engine);
	if(eng) {
	    for ( uint i = 0; i < eng->get_internal_playlist().size(); ++i ){
		QDomNode nextNode = doc.createElement( "next" );

		LocalFileMng::writeXmlString ( nextNode, "song", eng->get_internal_playlist()[i].m_hFile );

		LocalFileMng::writeXmlString ( nextNode, "script", eng->get_internal_playlist()[i].m_hScript );

		LocalFileMng::writeXmlString ( nextNode, "enabled", eng->get_internal_playlist()[i].m_hScriptEnabled );

		playlistNode.appendChild( nextNode );
	    }
	}

        rootNode.appendChild( playlistNode );
        doc.appendChild( rootNode );

        QString filename = QString( patternname.c_str() );
        QFile file(filename);
        if ( !file.open(QIODevice::WriteOnly) )
            return NULL;

        QTextStream TextStream( &file );
        doc.save( TextStream, 1 );

        file.close();

        return 0; // ok

    }

    int LocalFileMng::loadPlayList( const std::string& patternname)
    {


        std::string playlistInfoFile = patternname;
        std::ifstream verify( playlistInfoFile.c_str() , std::ios::in | std::ios::binary );
        if ( verify == NULL ) {
            //ERRORLOG( "Load Playlist: Data file " + playlistInfoFile + " not found." );
            return NULL;
        }

        QDomDocument doc = LocalFileMng::openXmlDocument( QString( patternname.c_str() ) );

        QDomNode rootNode = doc.firstChildElement( "playlist" );        // root element
        if ( rootNode.isNull() ) {
            ERRORLOG( "Error reading playlist: playlist node not found" );
            return NULL;
        }
        QDomNode playlistNode = rootNode.firstChildElement( "Songs" );

        if ( ! playlistNode.isNull() ) {
            // new code :)
	    Tritium::Engine *eng;
	    eng = dynamic_cast<Tritium::Engine*>(m_engine);

	    if(eng) {
		eng->get_internal_playlist().clear();
		QDomNode nextNode = playlistNode.firstChildElement( "next" );
		while (  ! nextNode.isNull() ) {
		    Engine::HPlayListNode playListItem;
		    playListItem.m_hFile = LocalFileMng::readXmlString( nextNode, "song", "" );
		    playListItem.m_hScript = LocalFileMng::readXmlString( nextNode, "script", "" );
		    playListItem.m_hScriptEnabled = LocalFileMng::readXmlString( nextNode, "enabled", "" );
		    eng->get_internal_playlist().push_back( playListItem );
		    nextNode = nextNode.nextSiblingElement( "next" );
		}
	    }
        }
        return 0; // ok
    }



/* New QtXml based methods */

    QString LocalFileMng::readXmlString( QDomNode node , const QString& nodeName, const QString& defaultValue, bool bCanBeEmpty, bool bShouldExists, bool tinyXmlCompatMode)
    {
        QDomElement element = node.firstChildElement( nodeName );

        if( !node.isNull() && !element.isNull() ){
            if(  !element.text().isEmpty() ){
                return element.text();
            } else {
                if ( !bCanBeEmpty ) {
                    DEBUGLOG( "Using default value in " + nodeName );
                }
                return defaultValue;
            }
        } else {
            if(  bShouldExists ){
                DEBUGLOG( "'" + nodeName + "' node not found" );

            }
            return defaultValue;
        }
    }

    float LocalFileMng::readXmlFloat( QDomNode node , const QString& nodeName, float defaultValue, bool bCanBeEmpty, bool bShouldExists, bool tinyXmlCompatMode)
    {
        QLocale c_locale = QLocale::c();
        QDomElement element = node.firstChildElement( nodeName );

        if( !node.isNull() && !element.isNull() ){
            if(  !element.text().isEmpty() ){
                return c_locale.toFloat(element.text());
            } else {
                if ( !bCanBeEmpty ) {
                    DEBUGLOG( "Using default value in " + nodeName );
                }
                return defaultValue;
            }
        } else {
            if(  bShouldExists ){
                DEBUGLOG( "'" + nodeName + "' node not found" );
            }
            return defaultValue;
        }
    }

    int LocalFileMng::readXmlInt( QDomNode node , const QString& nodeName, int defaultValue, bool bCanBeEmpty, bool bShouldExists, bool tinyXmlCompatMode)
    {
        QLocale c_locale = QLocale::c();
        QDomElement element = node.firstChildElement( nodeName );

        if( !node.isNull() && !element.isNull() ){
            if(  !element.text().isEmpty() ){
                return c_locale.toInt( element.text() );
            } else {
                if ( !bCanBeEmpty ) {
                    DEBUGLOG( "Using default value in " + nodeName );
                }
                return defaultValue;
            }
        } else {
            if(  bShouldExists ){
                DEBUGLOG( "'" + nodeName + "' node not found" );
            }
            return defaultValue;
        }
    }

    bool LocalFileMng::readXmlBool( QDomNode node , const QString& nodeName, bool defaultValue, bool bShouldExists, bool tinyXmlCompatMode)
    {
        QDomElement element = node.firstChildElement( nodeName );

        if( !node.isNull() && !element.isNull() ){
            if(  !element.text().isEmpty() ){
                if( element.text() == "true"){
                    return true;
                } else {
                    return false;
                }
            } else {
                DEBUGLOG( "Using default value in " + nodeName );
                return defaultValue;
            }
        } else {
            if(  bShouldExists ){
                DEBUGLOG( "'" + nodeName + "' node not found" );
            }
            return defaultValue;
        }
    }


    void LocalFileMng::writeXmlString( QDomNode parent, const QString& name, const QString& text )
    {
        /*
          TiXmlElement versionNode( name.toAscii() );
          TiXmlText versionText( text.toAscii() );
          versionNode.appendChild( versionText );
          parent->appendChild( versionNode );
        */
        QDomDocument doc;
        QDomElement elem = doc.createElement( name );
        QDomText t = doc.createTextNode( text );
        elem.appendChild( t );
        parent.appendChild( elem );
    }



    void LocalFileMng::writeXmlBool( QDomNode parent, const QString& name, bool value )
    {
        if ( value ) {
            writeXmlString( parent, name, QString( "true" ) );
        } else {
            writeXmlString( parent, name, QString( "false" ) );
        }
    }

/* Convert (in-place) an XML escape sequence into a literal byte,
 * rather than the character it actually refers to.
 */
    void LocalFileMng::convertFromTinyXMLString( QByteArray* str )
    {
        /* When TinyXML encountered a non-ASCII character, it would
         * simply write the character as "&#xx;" -- where "xx" is
         * the hex character code.  However, this doesn't respect
         * any encodings (e.g. UTF-8, UTF-16).  In XML, &#xx; literally
         * means "the Unicode character # xx."  However, in a UTF-8
         * sequence, this could be an escape character that tells
         * whether we have a 2, 3, or 4-byte UTF-8 sequence.
         *
         * For example, the UTF-8 sequence 0xD184 was being written
         * by TinyXML as "&#xD1;&#x84;".  However, this is the UTF-8
         * sequence for the cyrillic small letter EF (which looks
         * kind of like a thorn or a greek phi).  This letter, in
         * XML, should be saved as &#x00000444;, or even literally
         * (no escaping).  As a consequence, when &#xD1; is read
         * by an XML parser, it will be interpreted as capital N
         * with a tilde (~).  Then &#x84; will be interpreted as
         * an unknown or control character.
         *
         * So, when we know that TinyXML wrote the file, we can
         * simply exchange these hex sequences to literal bytes.
         */
        int pos = 0;

        pos = str->indexOf("&#x");
        while( pos != -1 ) {
            if( isxdigit(str->at(pos+3))
                && isxdigit(str->at(pos+4))
                && (str->at(pos+5) == ';') ) {
                char w1 = str->at(pos+3);
                char w2 = str->at(pos+4);

                w1 = tolower(w1) - 0x30;  // '0' = 0x30
                if( w1 > 9 ) w1 -= 0x27;  // '9' = 0x39, 'a' = 0x61
                w1 = (w1 & 0xF);

                w2 = tolower(w2) - 0x30;  // '0' = 0x30
                if( w2 > 9 ) w2 -= 0x27;  // '9' = 0x39, 'a' = 0x61
                w2 = (w2 & 0xF);

                char ch = (w1 << 4) | w2;
                (*str)[pos] = ch;
                ++pos;
                str->remove(pos, 5);
            }
            pos = str->indexOf("&#x");
        }
    }

    bool LocalFileMng::checkTinyXMLCompatMode( const QString& filename )
    {
        /*
          Check if filename was created with TinyXml or QtXml
          TinyXML: return true
          QtXml: return false
        */

        QFile file( filename );

        if ( !file.open(QIODevice::ReadOnly) )
            return false;

        QString line = file.readLine();
        file.close();
        if ( line.startsWith( "<?xml" )){
            return false;
        } else  {
            WARNINGLOG( QString("File '%1' is being read in "
                                "TinyXML compatability mode")
                        .arg(filename) );
            return true;
        }



    }

    QDomDocument LocalFileMng::openXmlDocument( const QString& filename )
    {
        bool TinyXMLCompat = LocalFileMng::checkTinyXMLCompatMode( filename );

        QDomDocument doc;
        QFile file( filename );

        if ( !file.open(QIODevice::ReadOnly) )
            return QDomDocument();

        if( TinyXMLCompat ) {
            QString enc = QTextCodec::codecForLocale()->name();
            if( enc == QString("System") ) {
                enc = "UTF-8";
            }
            QByteArray line;
            QByteArray buf = QString("<?xml version='1.0' encoding='%1' ?>\n")
                .arg( enc )
                .toLocal8Bit();

            //DEBUGLOG( QString("Using '%1' encoding for TinyXML file").arg(enc) );

            while( !file.atEnd() ) {
                line = file.readLine();
                LocalFileMng::convertFromTinyXMLString( &line );
                buf += line;
            }

            if( ! doc.setContent( buf ) ) {
                file.close();
                return QDomDocument();
            }

        } else {
            if( ! doc.setContent( &file ) ) {
                file.close();
                return QDomDocument();
            }
        }
        file.close();

        return doc;
    }

} // namespace Tritium
