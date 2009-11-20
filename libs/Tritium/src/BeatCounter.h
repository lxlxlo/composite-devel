/*
 * Hydrogen
 * Copyright(c) 2002-2008 by Alex >Comix< Cominu [comix@users.sourceforge.net]
 *
 * http://www.hydrogen-music.org
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY, without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */
#ifndef H2CORE_BEATCOUNTER_H
#define H2CORE_BEATCOUNTER_H

#include <stdint.h>  // int32_t, uint32_t
#include <sys/time.h>

namespace H2Core
{
    class BeatCounter
    {
    public:
	BeatCounter();
	~BeatCounter();

	void setBeatsToCount(int beats) {
	    m_nbeatsToCount = beats;
	}
	int getBeatsToCount() {
	    return m_nbeatsToCount;
	}
	void setNoteLength( float len ) {
	    m_ntaktoMeterCompute = len;
	}
	float getNoteLength() {
	    return m_ntaktoMeterCompute;
	}
	int status() {
	    return eventCount;
	}
	void setOffsetAdjust();
	void trigger();

	// Tap Tempo methods
	void setTapTempo( float fInterval );
	void onTapTempoAccelEvent();

    private:

	// BeatCounter Variables
	float m_ntaktoMeterCompute;	  	///< beatcounter note length
	int m_nbeatsToCount;			///< beatcounter beats to count
	int eventCount;				///< beatcounter event
	int tempochangecounter;			///< count tempochanges for timeArray
	int beatCount;				///< beatcounter beat to count
	double beatDiffs[16];			///< beat diff
	timeval currentTime, lastTime;		///< timeval
	double lastBeatTime, currentBeatTime, beatDiff;		///< timediff
	float beatCountBpm;			///< bpm
	int m_nCoutOffset;			///ms default 0
	int m_nStartOffset;			///ms default 0

	// Tap Tempo Variables
	float fOldBpm1;
	float fOldBpm2;
	float fOldBpm3;
	float fOldBpm4;
	float fOldBpm5;
	float fOldBpm6;
	float fOldBpm7;
	float fOldBpm8;
	timeval oldTimeVal;

    };

} // namespace H2Core

#endif // H2CORE_BEATCOUNTER_H
