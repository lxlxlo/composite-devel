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

#include <Tritium/SimpleStereoSample.hpp>
#include <Tritium/Logger.hpp>
#include <Tritium/Preferences.hpp>
#include "FLACFile.hpp"
#include <sndfile.h>
#include <iostream>
#include <fstream>

using namespace std;

namespace Tritium
{

SimpleStereoSample::SimpleStereoSample(
	unsigned frames,
	const QString& filename,
	unsigned sample_rate,
	float* data_l,
	float* data_r
	)
	: __data_l( data_l )
	, __data_r( data_r )
	, __sample_rate( sample_rate )
	, __filename( filename )
	, __n_frames( frames )
{
    __url = QUrl::fromLocalFile(__filename);
}



SimpleStereoSample::~SimpleStereoSample()
{
	delete[] __data_l;
	delete[] __data_r;
	//DEBUGLOG( "DESTROY " + m_sFilename);
}

const QUrl& SimpleStereoSample::source_url() const
{
    return __url;
}


T<SimpleStereoSample>::shared_ptr SimpleStereoSample::load( const QString& filename )
{
	// is it a flac file?
	if ( ( filename.endsWith( "flac") ) || ( filename.endsWith( "FLAC" )) ) {
		return load_flac( filename );
	} else {
		return load_wave( filename );
	}
}



/// load a FLAC file
T<SimpleStereoSample>::shared_ptr SimpleStereoSample::load_flac( const QString& filename )
{
#ifdef FLAC_SUPPORT
	FLACFile file;
	return file.load_simple( filename );
#else
	ERRORLOG("[loadFLAC] FLAC support was disabled during compilation");
	return T<SimpleStereoSample>::shared_ptr();
#endif
}



T<SimpleStereoSample>::shared_ptr SimpleStereoSample::load_wave( const QString& filename )
{
	// file exists?
	if ( QFile( filename ).exists() == false ) {
		ERRORLOG( QString( "[SimpleStereoSample::load] Load sample: File %1 not found" ).arg( filename ) );
		return T<SimpleStereoSample>::shared_ptr();
	}


	SF_INFO soundInfo;
	SNDFILE* file = sf_open( filename.toLocal8Bit(), SFM_READ, &soundInfo );
	if ( !file ) {
		ERRORLOG( QString( "[SimpleStereoSample::load] Error loading file %1" ).arg( filename ) );
	}


	float *pTmpBuffer = new float[ soundInfo.frames * soundInfo.channels ];

	//int res = sf_read_float( file, pTmpBuffer, soundInfo.frames * soundInfo.channels );
	sf_read_float( file, pTmpBuffer, soundInfo.frames * soundInfo.channels );
	sf_close( file );

	float *data_l = new float[ soundInfo.frames ];
	float *data_r = new float[ soundInfo.frames ];

	if ( soundInfo.channels == 1 ) {	// MONO sample
		for ( long int i = 0; i < soundInfo.frames; i++ ) {
			data_l[i] = pTmpBuffer[i];
			data_r[i] = pTmpBuffer[i];
		}
	} else if ( soundInfo.channels == 2 ) { // STEREO sample
		for ( long int i = 0; i < soundInfo.frames; i++ ) {
			data_l[i] = pTmpBuffer[i * 2];
			data_r[i] = pTmpBuffer[i * 2 + 1];
		}
	}
	delete[] pTmpBuffer;


	T<SimpleStereoSample>::shared_ptr pSample(
	    new SimpleStereoSample(
		soundInfo.frames,
		filename,
		soundInfo.samplerate,
		data_l,
		data_r
		)
	    );
	return pSample;
}



} // namespace Tritium

