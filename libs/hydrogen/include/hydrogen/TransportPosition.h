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
#ifndef H2CORE_TRANSPORTPOSITION_H
#define H2CORE_TRANSPORTPOSITION_H

#include <stdint.h>  // int32_t, uint32_t

namespace H2Core
{
    /**
     * Communicates the current position of the transport for the current
     * process() cycle.  All values refer to the first frame of the cycle.
     *
     * Unlike the JACK trnsport, all fields must be present and must be valid.
     */
    struct TransportPosition
    {
        enum State { STOPPED, ROLLING } state; /// The current transport state.
        bool new_position;        /// True if this position is not a continuation
                                  /// of the last one (i.e. we've relocated)
        uint32_t frame;           /// The current frame of the transport.  When
                                  /// sequencing, this is just FYI.  All
                                  /// sequencing shall be done based on the
                                  /// other fields (esp. B:b:t).
        uint32_t frame_rate;      /// The audio sample rate (frames per second)
        int32_t bar;              /// The current measure (1, 2, 3...)
        int32_t beat;             /// The current beat in measure (1, 2, 3...)
        int32_t tick;             /// The current tick in beat (0, 1, 2...)
        uint32_t bbt_offset;      /// bar, beat, and tick refer to bbt_offset
                                  /// frames BEFORE the current process cycle.
        uint32_t bar_start_tick;  /// Absolute number of ticks elapsed in song
                                  /// at the start of this bar.
        uint8_t beats_per_bar;    /// The top number in the time signature
        uint8_t beat_type;        /// The bottom number in the time signature
        uint32_t ticks_per_beat;  /// Number of ticks in a single beat
        double beats_per_minute;  /// The song tempo (beats per minute)

	/**
	 * Constructors
	 */
	TransportPosition();
	TransportPosition(const TransportPosition& orig);

        /**
         * Round struct so that bbt_offset is 0 and frame refers to the exact
         * B:b.t.  Can be rounded to nearest bar, beat, or tick.
         */
        typedef enum { BAR, BEAT, TICK } snap_type;
        void round(snap_type s);
        void floor(snap_type s);
        void ceil(snap_type s);

        /**
         * Increment/decrement one tick.  Note that this does not adjust
         * bbt_offset to zero.  Use round(), floor(), or ceil() for that.
         */
        TransportPosition& operator++();
        TransportPosition& operator--();

        /**
	 * Advance/rewind one tick.  Note that this does not adjust
	 * bbt_offset to zero.  Use round(), floor(), or ceil() for that.
	 */
	friend TransportPosition operator+(const TransportPosition& pos, int ticks);
	friend TransportPosition operator-(const TransportPosition& pos, int ticks);
	TransportPosition& operator+=(int ticks);
	TransportPosition& operator-=(int ticks);

	/**
	 * If bar, beat, tick, and bbt_offset are not in-bounds, this
	 * adjusts each field so that they are.
	 *
	 * For example, if you are in 4/4.48 time and the current
	 * B:b:t is 5:6.-5, this function will normalize the tick
	 * (5:5.0) and then normalize the beat, giving 6:1.0.
	 */
	void normalize();

        /**
         * Convenience calculations
         */
        inline double frames_per_tick() const {
            return double(frame_rate) * 60.0 / beats_per_minute / double(ticks_per_beat);
        }

        inline uint32_t tick_in_bar() const {
            return (beat-1) * ticks_per_beat + tick;
        }

    };

TransportPosition operator+(const TransportPosition& pos, int ticks);
TransportPosition operator-(const TransportPosition& pos, int ticks);

}

#endif // H2CORE_TRANSPORTPOSITION_H
