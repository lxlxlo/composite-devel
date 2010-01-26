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

    LocalFileMng::LocalFileMng(Engine* parent) :
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
	serializer->load_file(filename, bdl, m_engine);

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
		}
		break;
	    default:
		ERRORLOG("Loading pattern also loaded an unexpected type.");
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
        T<Instrument>::shared_ptr instr = song->get_instrument_list()->get( 0 );
        assert( instr );
	QString drumkit_name = instr->get_drumkit_name();
	QString data_directory = m_engine->get_preferences()->getDataDirectory();

	QString sPatternDir = data_directory + "patterns/" + drumkit_name;
        INFOLOG( "[savePattern]" + sPatternDir );

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
            WARNINGLOG( "Pattern Save unknown status");
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

        INFOLOG( sOrigFilename + " --> " + sDestFilename );

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
            INFOLOG( QString( "No patterns for drumkit '%1'." ).arg( sDrumkit ) );
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
                if ( ( sFile == "." ) || ( sFile == ".." ) || ( sFile == "CVS" )  || ( sFile == ".svn" ) ||
                     (sFile =="songs" ) || ( sFile == "patterns" )  || (sFile == "drumkits" || sFile == "playlists" ) || (sFile == "scripts" )) {
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
	serializer->load_file(drumkitInfoFile, bdl, m_engine);

	while( ! bdl.done ) {
	    sleep(1);
	}

	T<Drumkit>::shared_ptr rv;

	if( bdl.error ) {
	    ERRORLOG(bdl.error_message);
	    return rv;
	}

	T<InstrumentList>::auto_ptr inst_list(new InstrumentList);

	while( ! bdl.empty() ) {
	    switch(bdl.peek_type()) {
	    case ObjectItem::Drumkit_t:
		if( ! rv ) {
		    rv = bdl.pop<Drumkit>();
		} else {
		    ERRORLOG("Loading drumkit returned more than one.");
		}
	    case ObjectItem::Instrument_t:
		inst_list->add( bdl.pop<Instrument>() );
		break;
	    default:
		ERRORLOG("Loading pattern also loaded an unexpected type.");
	    }
	}

	if( ! rv ) {
	    ERRORLOG("A drumkit object was not returned.");
	} else {
	    rv->setInstrumentList(inst_list.release());
	}

	return rv;
    }



    int LocalFileMng::saveDrumkit( T<Drumkit>::shared_ptr info )
    {
        INFOLOG( "[saveDrumkit]" );
        info->dump();   // debug

        QVector<QString> tempVector(16);

        QString sDrumkitDir = m_engine->get_preferences()->getDataDirectory() + "drumkits/" + info->getName();

        // check if the directory exists
        QDir dir( sDrumkitDir );
        if ( !dir.exists() ) {
            dir.mkdir( sDrumkitDir );// create the drumkit directory
            //mkdir( sDrumkitDir.c_str(), S_IRWXU );
        } else {

            //warningLog( "[saveDrumkit] Cleaning directory " + sDrumkitDir );
            // clear all the old files in the directory
            //string clearCmd = "rm -f " + sDrumkitDir + "/*";
            //system( clearCmd.c_str() );
        }


        // create the drumkit.xml file
        QString sDrumkitXmlFilename = sDrumkitDir + QString( "/drumkit.xml" );

        QDomDocument doc;
        QDomProcessingInstruction header = doc.createProcessingInstruction( "xml", "version=\"1.0\" encoding=\"UTF-8\"");
        doc.appendChild( header );

        QDomElement rootNode = doc.createElement( "drumkit_info" );

        writeXmlString( rootNode, "name", info->getName() );    // name
        writeXmlString( rootNode, "author", info->getAuthor() );        // author
        writeXmlString( rootNode, "info", info->getInfo() );    // info
        writeXmlString( rootNode, "license", info->getLicense() );      // license

        //QDomNode instrumentListNode( "instrumentList" );              // instrument list
        QDomElement instrumentListNode = doc.createElement( "instrumentList" );

        unsigned nInstrument = info->getInstrumentList()->get_size();
        // INSTRUMENT NODE
        for ( unsigned i = 0; i < nInstrument; i++ ) {
            T<Instrument>::shared_ptr instr = info->getInstrumentList()->get( i );

            for ( unsigned nLayer = 0; nLayer < MAX_LAYERS; nLayer++ ) {
                InstrumentLayer *pLayer = instr->get_layer( nLayer );
                if ( pLayer ) {
                    T<Sample>::shared_ptr pSample = pLayer->get_sample();
                    QString sOrigFilename = pSample->get_filename();

                    QString sDestFilename = sOrigFilename;

                    /*
                      Till rev. 743, the samples got copied into the
                      root of the drumkit folder.

                      Now the sample gets only copied to the folder
                      if it doesn't reside in a subfolder of the drumkit dir.
                    */

                    if( sOrigFilename.startsWith( sDrumkitDir ) ){
                        INFOLOG("sample is already in drumkit dir");
                        tempVector[ nLayer ] = sDestFilename.remove( sDrumkitDir + "/" );
                    } else {
                        int nPos = sDestFilename.lastIndexOf( '/' );
                        sDestFilename = sDestFilename.mid( nPos + 1, sDestFilename.size() - nPos - 1 );
                        sDestFilename = sDrumkitDir + "/" + sDestFilename;

                        fileCopy( sOrigFilename, sDestFilename );
                        tempVector[ nLayer ] = sDestFilename.remove( sDrumkitDir + "/" );
                    }
                }
            }

            QDomNode instrumentNode = doc.createElement( "instrument" );

            LocalFileMng::writeXmlString( instrumentNode, "id", instr->get_id() );
            LocalFileMng::writeXmlString( instrumentNode, "name", instr->get_name() );
            LocalFileMng::writeXmlString( instrumentNode, "volume", QString("%1").arg( instr->get_volume() ) );
            LocalFileMng::writeXmlBool( instrumentNode, "isMuted", instr->is_muted() );
            LocalFileMng::writeXmlString( instrumentNode, "pan_L", QString("%1").arg( instr->get_pan_l() ) );
            LocalFileMng::writeXmlString( instrumentNode, "pan_R", QString("%1").arg( instr->get_pan_r() ) );
            LocalFileMng::writeXmlString( instrumentNode, "randomPitchFactor", QString("%1").arg( instr->get_random_pitch_factor() ) );
            LocalFileMng::writeXmlString( instrumentNode, "gain", QString("%1").arg( instr->get_gain() ) );

            LocalFileMng::writeXmlBool( instrumentNode, "filterActive", instr->is_filter_active() );
            LocalFileMng::writeXmlString( instrumentNode, "filterCutoff", QString("%1").arg( instr->get_filter_cutoff() ) );
            LocalFileMng::writeXmlString( instrumentNode, "filterResonance", QString("%1").arg( instr->get_filter_resonance() ) );

            LocalFileMng::writeXmlString( instrumentNode, "Attack", QString("%1").arg( instr->get_adsr()->__attack ) );
            LocalFileMng::writeXmlString( instrumentNode, "Decay", QString("%1").arg( instr->get_adsr()->__decay ) );
            LocalFileMng::writeXmlString( instrumentNode, "Sustain", QString("%1").arg( instr->get_adsr()->__sustain ) );
            LocalFileMng::writeXmlString( instrumentNode, "Release", QString("%1").arg( instr->get_adsr()->__release ) );

            LocalFileMng::writeXmlString( instrumentNode, "muteGroup", QString("%1").arg( instr->get_mute_group() ) );

            for ( unsigned nLayer = 0; nLayer < MAX_LAYERS; nLayer++ ) {
                InstrumentLayer *pLayer = instr->get_layer( nLayer );
                if ( pLayer == NULL ) continue;
                // Sample *pSample = pLayer->get_sample();

                QDomNode layerNode = doc.createElement( "layer" );
                LocalFileMng::writeXmlString( layerNode, "filename", tempVector[ nLayer ] );
                LocalFileMng::writeXmlString( layerNode, "min", QString("%1").arg( pLayer->get_min_velocity() ) );
                LocalFileMng::writeXmlString( layerNode, "max", QString("%1").arg( pLayer->get_max_velocity() ) );
                LocalFileMng::writeXmlString( layerNode, "gain", QString("%1").arg( pLayer->get_gain() ) );
                LocalFileMng::writeXmlString( layerNode, "pitch", QString("%1").arg( pLayer->get_pitch() ) );

                instrumentNode.appendChild( layerNode );
            }

            instrumentListNode.appendChild( instrumentNode );
        }

        rootNode.appendChild( instrumentListNode );

        doc.appendChild( rootNode );

        QFile file( sDrumkitXmlFilename );
        if ( !file.open(QIODevice::WriteOnly) )
            return 0;

        QTextStream TextStream( &file );
        doc.save( TextStream, 1 );

        file.close();

        return 0; // ok
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
        for ( uint i = 0; i < m_engine->get_internal_playlist().size(); ++i ){
            QDomNode nextNode = doc.createElement( "next" );

            LocalFileMng::writeXmlString ( nextNode, "song", m_engine->get_internal_playlist()[i].m_hFile );

            LocalFileMng::writeXmlString ( nextNode, "script", m_engine->get_internal_playlist()[i].m_hScript );

            LocalFileMng::writeXmlString ( nextNode, "enabled", m_engine->get_internal_playlist()[i].m_hScriptEnabled );

            playlistNode.appendChild( nextNode );
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

        m_engine->get_internal_playlist().clear();

        QDomNode rootNode = doc.firstChildElement( "playlist" );        // root element
        if ( rootNode.isNull() ) {
            ERRORLOG( "Error reading playlist: playlist node not found" );
            return NULL;
        }
        QDomNode playlistNode = rootNode.firstChildElement( "Songs" );

        if ( ! playlistNode.isNull() ) {
            // new code :)
            m_engine->get_internal_playlist().clear();
            QDomNode nextNode = playlistNode.firstChildElement( "next" );
            while (  ! nextNode.isNull() ) {
                Engine::HPlayListNode playListItem;
                playListItem.m_hFile = LocalFileMng::readXmlString( nextNode, "song", "" );
                playListItem.m_hScript = LocalFileMng::readXmlString( nextNode, "script", "" );
                playListItem.m_hScriptEnabled = LocalFileMng::readXmlString( nextNode, "enabled", "" );
                m_engine->get_internal_playlist().push_back( playListItem );
                nextNode = nextNode.nextSiblingElement( "next" );
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
                    WARNINGLOG( "Using default value in " + nodeName );
                }
                return defaultValue;
            }
        } else {
            if(  bShouldExists ){
                WARNINGLOG( "'" + nodeName + "' node not found" );

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
                    WARNINGLOG( "Using default value in " + nodeName );
                }
                return defaultValue;
            }
        } else {
            if(  bShouldExists ){
                WARNINGLOG( "'" + nodeName + "' node not found" );
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
                    WARNINGLOG( "Using default value in " + nodeName );
                }
                return defaultValue;
            }
        } else {
            if(  bShouldExists ){
                WARNINGLOG( "'" + nodeName + "' node not found" );
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
                WARNINGLOG( "Using default value in " + nodeName );
                return defaultValue;
            }
        } else {
            if(  bShouldExists ){
                WARNINGLOG( "'" + nodeName + "' node not found" );
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

            //INFOLOG( QString("Using '%1' encoding for TinyXML file").arg(enc) );

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




//-----------------------------------------------------------------------------
//      Implementation of SongWriter class
//-----------------------------------------------------------------------------


    SongWriter::SongWriter()
    {
//      infoLog("init");
    }



    SongWriter::~SongWriter()
    {
//      infoLog("destroy");
    }


// Returns 0 on success, passes the TinyXml error code otherwise.
    int SongWriter::writeSong( Engine* engine, Song& song, const QString& filename )
    {
        INFOLOG( "Saving song " + filename );
        int rv = 0; // return value

        // FIXME: has the file write-permssion?
        // FIXME: verificare che il file non sia gia' esistente
        // FIXME: effettuare copia di backup per il file gia' esistente


        QDomDocument doc;
        QDomProcessingInstruction header = doc.createProcessingInstruction( "xml", "version=\"1.0\" encoding=\"UTF-8\"");
        doc.appendChild( header );

        QDomNode songNode = doc.createElement( "song" );

        LocalFileMng::writeXmlString( songNode, "version", QString( get_version().c_str() ) );
        LocalFileMng::writeXmlString( songNode, "bpm", QString("%1").arg( song.get_bpm() ) );
        LocalFileMng::writeXmlString( songNode, "volume", QString("%1").arg( song.get_volume() ) );
        LocalFileMng::writeXmlString( songNode, "metronomeVolume", QString("%1").arg( song.get_metronome_volume() ) );
        LocalFileMng::writeXmlString( songNode, "name", song.get_name() );
        LocalFileMng::writeXmlString( songNode, "author", song.get_author() );
        LocalFileMng::writeXmlString( songNode, "notes", song.get_notes() );
        LocalFileMng::writeXmlString( songNode, "license", song.get_license() );
        LocalFileMng::writeXmlBool( songNode, "loopEnabled", song.is_loop_enabled() );

        if ( song.get_mode() == Song::SONG_MODE ) {
            LocalFileMng::writeXmlString( songNode, "mode", QString( "song" ) );
        } else {
            LocalFileMng::writeXmlString( songNode, "mode", QString( "pattern" ) );
        }

        LocalFileMng::writeXmlString( songNode, "humanize_time", QString("%1").arg( song.get_humanize_time_value() ) );
        LocalFileMng::writeXmlString( songNode, "humanize_velocity", QString("%1").arg( song.get_humanize_velocity_value() ) );
        LocalFileMng::writeXmlString( songNode, "swing_factor", QString("%1").arg( song.get_swing_factor() ) );

        /*      LocalFileMng::writeXmlBool( &songNode, "delayFXEnabled", song.m_bDelayFXEnabled );
                LocalFileMng::writeXmlString( &songNode, "delayFXWetLevel", QString("%1").arg( song.m_fDelayFXWetLevel ) );
                LocalFileMng::writeXmlString( &songNode, "delayFXFeedback", QString("%1").arg( song.m_fDelayFXFeedback ) );
                LocalFileMng::writeXmlString( &songNode, "delayFXTime", QString("%1").arg( song.m_nDelayFXTime ) );
        */

        // instrument list
        QDomNode instrumentListNode = doc.createElement( "instrumentList" );
        unsigned nInstrument = song.get_instrument_list()->get_size();

        // INSTRUMENT NODE
        for ( unsigned i = 0; i < nInstrument; i++ ) {
            T<Instrument>::shared_ptr instr = song.get_instrument_list()->get( i );
            assert( instr );

            QDomNode instrumentNode = doc.createElement( "instrument" );

            LocalFileMng::writeXmlString( instrumentNode, "id", instr->get_id() );
            LocalFileMng::writeXmlString( instrumentNode, "drumkit", instr->get_drumkit_name() );
            LocalFileMng::writeXmlString( instrumentNode, "name", instr->get_name() );
            LocalFileMng::writeXmlString( instrumentNode, "volume", QString("%1").arg( instr->get_volume() ) );
            LocalFileMng::writeXmlBool( instrumentNode, "isMuted", instr->is_muted() );
            LocalFileMng::writeXmlString( instrumentNode, "pan_L", QString("%1").arg( instr->get_pan_l() ) );
            LocalFileMng::writeXmlString( instrumentNode, "pan_R", QString("%1").arg( instr->get_pan_r() ) );
            LocalFileMng::writeXmlString( instrumentNode, "gain", QString("%1").arg( instr->get_gain() ) );

            LocalFileMng::writeXmlBool( instrumentNode, "filterActive", instr->is_filter_active() );
            LocalFileMng::writeXmlString( instrumentNode, "filterCutoff", QString("%1").arg( instr->get_filter_cutoff() ) );
            LocalFileMng::writeXmlString( instrumentNode, "filterResonance", QString("%1").arg( instr->get_filter_resonance() ) );

            LocalFileMng::writeXmlString( instrumentNode, "FX1Level", QString("%1").arg( instr->get_fx_level( 0 ) ) );
            LocalFileMng::writeXmlString( instrumentNode, "FX2Level", QString("%1").arg( instr->get_fx_level( 1 ) ) );
            LocalFileMng::writeXmlString( instrumentNode, "FX3Level", QString("%1").arg( instr->get_fx_level( 2 ) ) );
            LocalFileMng::writeXmlString( instrumentNode, "FX4Level", QString("%1").arg( instr->get_fx_level( 3 ) ) );

            assert( instr->get_adsr() );
            LocalFileMng::writeXmlString( instrumentNode, "Attack", QString("%1").arg( instr->get_adsr()->__attack ) );
            LocalFileMng::writeXmlString( instrumentNode, "Decay", QString("%1").arg( instr->get_adsr()->__decay ) );
            LocalFileMng::writeXmlString( instrumentNode, "Sustain", QString("%1").arg( instr->get_adsr()->__sustain ) );
            LocalFileMng::writeXmlString( instrumentNode, "Release", QString("%1").arg( instr->get_adsr()->__release ) );

            LocalFileMng::writeXmlString( instrumentNode, "randomPitchFactor", QString("%1").arg( instr->get_random_pitch_factor() ) );

            LocalFileMng::writeXmlString( instrumentNode, "muteGroup", QString("%1").arg( instr->get_mute_group() ) );

            for ( unsigned nLayer = 0; nLayer < MAX_LAYERS; nLayer++ ) {
                InstrumentLayer *pLayer = instr->get_layer( nLayer );
                if ( pLayer == NULL ) continue;
                T<Sample>::shared_ptr pSample = pLayer->get_sample();
                if ( pSample == NULL ) continue;

                QString sFilename = pSample->get_filename();
                if ( !instr->get_drumkit_name().isEmpty() ) {
                    // se e' specificato un drumkit, considero solo il nome del file senza il path
                    int nPos = sFilename.lastIndexOf( "/" );
                    sFilename = sFilename.mid( nPos + 1, sFilename.length() );
                }
                QDomNode layerNode = doc.createElement( "layer" );
                LocalFileMng::writeXmlString( layerNode, "filename", sFilename );
                LocalFileMng::writeXmlString( layerNode, "min", QString("%1").arg( pLayer->get_min_velocity() ) );
                LocalFileMng::writeXmlString( layerNode, "max", QString("%1").arg( pLayer->get_max_velocity() ) );
                LocalFileMng::writeXmlString( layerNode, "gain", QString("%1").arg( pLayer->get_gain() ) );
                LocalFileMng::writeXmlString( layerNode, "pitch", QString("%1").arg( pLayer->get_pitch() ) );

                instrumentNode.appendChild( layerNode );
            }

            instrumentListNode.appendChild( instrumentNode );
        }
        songNode.appendChild( instrumentListNode );


        // pattern list
        QDomNode patternListNode = doc.createElement( "patternList" );

        unsigned nPatterns = song.get_pattern_list()->get_size();
        for ( unsigned i = 0; i < nPatterns; i++ ) {
            T<Pattern>::shared_ptr pat = song.get_pattern_list()->get( i );

            // pattern
            QDomNode patternNode = doc.createElement( "pattern" );
            LocalFileMng::writeXmlString( patternNode, "name", pat->get_name() );
            LocalFileMng::writeXmlString( patternNode, "category", pat->get_category() );
            LocalFileMng::writeXmlString( patternNode, "size", QString("%1").arg( pat->get_length() ) );

            QDomNode noteListNode = doc.createElement( "noteList" );
            Pattern::note_map_t::iterator pos;
            for ( pos = pat->note_map.begin(); pos != pat->note_map.end(); ++pos ) {
                Note *pNote = pos->second;
                assert( pNote );

                QDomNode noteNode = doc.createElement( "note" );
                LocalFileMng::writeXmlString( noteNode, "position", QString("%1").arg( pos->first ) );
                LocalFileMng::writeXmlString( noteNode, "leadlag", QString("%1").arg( pNote->get_leadlag() ) );
                LocalFileMng::writeXmlString( noteNode, "velocity", QString("%1").arg( pNote->get_velocity() ) );
                LocalFileMng::writeXmlString( noteNode, "pan_L", QString("%1").arg( pNote->get_pan_l() ) );
                LocalFileMng::writeXmlString( noteNode, "pan_R", QString("%1").arg( pNote->get_pan_r() ) );
                LocalFileMng::writeXmlString( noteNode, "pitch", QString("%1").arg( pNote->get_pitch() ) );

                LocalFileMng::writeXmlString( noteNode, "key", Note::keyToString( pNote->m_noteKey ) );

                LocalFileMng::writeXmlString( noteNode, "length", QString("%1").arg( pNote->get_length() ) );
                LocalFileMng::writeXmlString( noteNode, "instrument", pNote->get_instrument()->get_id() );
                noteListNode.appendChild( noteNode );
            }
            patternNode.appendChild( noteListNode );

            patternListNode.appendChild( patternNode );
        }
        songNode.appendChild( patternListNode );


        // pattern sequence
        QDomNode patternSequenceNode = doc.createElement( "patternSequence" );

        unsigned nPatternGroups = song.get_pattern_group_vector()->size();
        for ( unsigned i = 0; i < nPatternGroups; i++ ) {
            QDomNode groupNode = doc.createElement( "group" );

            T<PatternList>::shared_ptr pList = ( *song.get_pattern_group_vector() )[i];
            for ( unsigned j = 0; j < pList->get_size(); j++ ) {
                T<Pattern>::shared_ptr pPattern = pList->get( j );
                LocalFileMng::writeXmlString( groupNode, "patternID", pPattern->get_name() );
            }
            patternSequenceNode.appendChild( groupNode );
        }

        songNode.appendChild( patternSequenceNode );


        // LADSPA FX
        QDomNode ladspaFxNode = doc.createElement( "ladspa" );

        for ( unsigned nFX = 0; nFX < MAX_FX; nFX++ ) {
            QDomNode fxNode = doc.createElement( "fx" );

#ifdef LADSPA_SUPPORT
            T<LadspaFX>::shared_ptr pFX = engine->get_effects()->getLadspaFX( nFX );
            if ( pFX ) {
                LocalFileMng::writeXmlString( fxNode, "name", pFX->getPluginLabel() );
                LocalFileMng::writeXmlString( fxNode, "filename", pFX->getLibraryPath() );
                LocalFileMng::writeXmlBool( fxNode, "enabled", pFX->isEnabled() );
                LocalFileMng::writeXmlString( fxNode, "volume", QString("%1").arg( pFX->getVolume() ) );
                for ( unsigned nControl = 0; nControl < pFX->inputControlPorts.size(); nControl++ ) {
                    LadspaControlPort *pControlPort = pFX->inputControlPorts[ nControl ];
                    QDomNode controlPortNode = doc.createElement( "inputControlPort" );
                    LocalFileMng::writeXmlString( controlPortNode, "name", pControlPort->sName );
                    LocalFileMng::writeXmlString( controlPortNode, "value", QString("%1").arg( pControlPort->fControlValue ) );
                    fxNode.appendChild( controlPortNode );
                }
                for ( unsigned nControl = 0; nControl < pFX->outputControlPorts.size(); nControl++ ) {
                    LadspaControlPort *pControlPort = pFX->inputControlPorts[ nControl ];
                    QDomNode controlPortNode = doc.createElement( "outputControlPort" );
                    LocalFileMng::writeXmlString( controlPortNode, "name", pControlPort->sName );
                    LocalFileMng::writeXmlString( controlPortNode, "value", QString("%1").arg( pControlPort->fControlValue ) );
                    fxNode.appendChild( controlPortNode );
                }
            }
#else
            if ( false ) {
            }
#endif
            else {
                LocalFileMng::writeXmlString( fxNode, "name", QString( "no plugin" ) );
                LocalFileMng::writeXmlString( fxNode, "filename", QString( "-" ) );
                LocalFileMng::writeXmlBool( fxNode, "enabled", false );
                LocalFileMng::writeXmlString( fxNode, "volume", "0.0" );
            }
            ladspaFxNode.appendChild( fxNode );
        }

        songNode.appendChild( ladspaFxNode );
        doc.appendChild( songNode );

        QFile file(filename);
        if ( !file.open(QIODevice::WriteOnly) )
            rv = 1;

        QTextStream TextStream( &file );
        doc.save( TextStream, 1 );

        file.close();



        if( rv ) {
            WARNINGLOG("File save reported an error.");
        } else {
            song.set_modified(false);
            INFOLOG("Save was successful.");
        }
        song.set_filename( filename );

        return rv;
    }

} // namespace Tritium
