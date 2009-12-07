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

#include <cstring>
#include <cmath>
#include <sys/time.h>

#include <Tritium/Engine.hpp>
#include <Tritium/Transport.hpp>
#include <Tritium/Preferences.hpp>
#include "BeatCounter.hpp"

using namespace Tritium;

//100,000 ms in 1 second.
#define US_DIVIDER .000001

BeatCounter::BeatCounter() :
    m_ntaktoMeterCompute( 1 ),
    m_nbeatsToCount( 4 ),
    eventCount( 1 ),
    tempochangecounter( 0 ),
    beatCount( 1 ),
    lastBeatTime( 0 ),
    currentBeatTime( 0 ),
    beatDiff( 0 ),
    beatCountBpm( 120.0 ),
    m_nCoutOffset( 0 ),
    m_nStartOffset( 0 ),
    fOldBpm1( -1 ),
    fOldBpm2( -1 ),
    fOldBpm3( -1 ),
    fOldBpm4( -1 ),
    fOldBpm5( -1 ),
    fOldBpm6( -1 ),
    fOldBpm7( -1 ),
    fOldBpm8( -1 )
{
    memset((void*)beatDiffs, 0, sizeof(beatDiffs));
    gettimeofday( &oldTimeVal, 0 );
    currentTime = lastTime = oldTimeVal;
}

BeatCounter::~BeatCounter()
{
}

void BeatCounter::setOffsetAdjust()
{
    //individual fine tuning for the beatcounter
    //to adjust  ms_offset from different people and controller
    Preferences *pref = Engine::get_instance()->get_preferences();

    m_nCoutOffset = pref->m_countOffset;
    m_nStartOffset = pref->m_startOffset;
}

void BeatCounter::trigger()
{
    Engine* H2 = Engine::get_instance();
    Transport* transport = H2->get_transport();
    TransportPosition Xpos;

    // Get first time value:
    if (beatCount == 1)
	gettimeofday(&currentTime,NULL);

    eventCount++;
		
    // Set wlastTime to wcurrentTime to remind the time:		
    lastTime = currentTime;
	
    // Get new time:
    gettimeofday(&currentTime,NULL);
	

    // Build doubled time difference:
    lastBeatTime = (double)(
	lastTime.tv_sec
	+ (double)(lastTime.tv_usec * US_DIVIDER)
	+ (int)m_nCoutOffset * .0001
	);
    currentBeatTime = (double)(
	currentTime.tv_sec
	+ (double)(currentTime.tv_usec * US_DIVIDER)
	);
    beatDiff = beatCount == 1 ? 0 : currentBeatTime - lastBeatTime;
		
    //if differences are to big reset the beatconter
    if( beatDiff > 3.001 * 1/m_ntaktoMeterCompute ){
	eventCount = 1;
	beatCount = 1;
	return;
    } 
    // Only accept differences big enough
    if (beatCount == 1 || beatDiff > .001) {
	if (beatCount > 1)
	    beatDiffs[beatCount - 2] = beatDiff ;
	// Compute and reset:
	if (beatCount == m_nbeatsToCount){
	    double beatTotalDiffs = 0;
	    for(int i = 0; i < (m_nbeatsToCount - 1); i++) 
		beatTotalDiffs += beatDiffs[i];
	    double beatDiffAverage =
		beatTotalDiffs
		/ (beatCount - 1)
		* m_ntaktoMeterCompute ;
	    beatCountBpm =
		(float) ((int) (60 / beatDiffAverage * 100))
		/ 100;
	    if ( beatCountBpm > 500)
		beatCountBpm = 500; 
	    H2->setBPM( beatCountBpm );
	    if (Engine::get_instance()->get_preferences()->m_mmcsetplay
		== Preferences::SET_PLAY_OFF) {
		beatCount = 1; 
		eventCount = 1;
	    }else{
		transport->get_position(&Xpos);
		if ( Xpos.state != TransportPosition::ROLLING ){
		    unsigned bcsamplerate = Xpos.frame_rate;
		    unsigned long rtstartframe = 0;
		    if ( m_ntaktoMeterCompute <= 1){
			rtstartframe =
			    bcsamplerate
			    * beatDiffAverage
			    * ( 1/ m_ntaktoMeterCompute );
		    }else
		    {
			rtstartframe =
			    bcsamplerate
			    * beatDiffAverage
			    / m_ntaktoMeterCompute ;
		    }

		    int sleeptime =
			( (float) rtstartframe
			  / (float) bcsamplerate
			  * (int) 1000 )
			+ (int)m_nCoutOffset
			+ (int) m_nStartOffset;
                    #ifdef WIN32
		    Sleep( sleeptime );
                    #else
		    usleep( 1000 * sleeptime );
                    #endif

		    transport->start();
		}
					
		beatCount = 1; 
		eventCount = 1;
		return;
	    }
	}
	else {
	    beatCount ++;
	}				
    }
    return;
}

void BeatCounter::setTapTempo( float fInterval )
{
    float fBPM = 60000.0 / fInterval;

    if ( fabs( fOldBpm1 - fBPM ) > 20 ) {	// troppa differenza, niente media
	fOldBpm1 = fBPM;
	fOldBpm2 = fBPM;
	fOldBpm3 = fBPM;
	fOldBpm4 = fBPM;
	fOldBpm5 = fBPM;
	fOldBpm6 = fBPM;
	fOldBpm7 = fBPM;
	fOldBpm8 = fBPM;
    }

    if ( fOldBpm1 == -1 ) {
	fOldBpm1 = fBPM;
	fOldBpm2 = fBPM;
	fOldBpm3 = fBPM;
	fOldBpm4 = fBPM;
	fOldBpm5 = fBPM;
	fOldBpm6 = fBPM;
	fOldBpm7 = fBPM;
	fOldBpm8 = fBPM;
    }

    fBPM = ( fBPM + fOldBpm1 + fOldBpm2 + fOldBpm3 + fOldBpm4 + fOldBpm5
	     + fOldBpm6 + fOldBpm7 + fOldBpm8 ) / 9.0;

    fOldBpm8 = fOldBpm7;
    fOldBpm7 = fOldBpm6;
    fOldBpm6 = fOldBpm5;
    fOldBpm5 = fOldBpm4;
    fOldBpm4 = fOldBpm3;
    fOldBpm3 = fOldBpm2;
    fOldBpm2 = fOldBpm1;
    fOldBpm1 = fBPM;

    Engine::get_instance()->setBPM( fBPM );
}

void BeatCounter::onTapTempoAccelEvent()
{
    struct timeval now;
    gettimeofday(&now, NULL);

    float fInterval =
	(now.tv_sec - oldTimeVal.tv_sec) * 1000.0
	+ (now.tv_usec - oldTimeVal.tv_usec) / 1000.0;

    oldTimeVal = now;

    if ( fInterval < 1000.0 ) {
	setTapTempo( fInterval );
    }
}
