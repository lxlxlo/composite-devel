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
#ifndef TRANSPORTMASTERINTERFACE_H
#define TRANSPORTMASTERINTERFACE_H

#include <stdint.h>  // int32_t, uint32_t

namespace H2Core
{
    class Song;

    /**
     * Communicates the current position of the transport for the current
     * process() cycle.  All values refer to the first frame of the cycle.
     *
     * Unlike the JACK trnsport, all fields must be present and must be valid.
     */
    struct TransportPosition
    {
        enum { STOPPED, ROLLING } state; /// The current transport state.
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
    };

    /**
     * This is the base class for a transport master.  It will be used and
     * controlled by the main transport.  A Transport Master is the one actually
     * in charge of the bars, beats, and ticks.
     */
    class TransportMasterInterface
    {
    public:
        virtual ~TransportMasterInterface() {}

        // Normal transport controls
        virtual int locate(uint32_t frame) = 0;
        virtual int locate(uint32_t bar, uint32_t beat, uint32_t tick) = 0;
        virtual void start(void) = 0;
        virtual void stop(void) = 0;
        virtual void get_position(TransportPosition* pos) =0;

        // Interface for sequencer.  At the end of process(), declare the number
        // of frames processed.  This is needed so that the internal transport
        // master can keep track of time.
        virtual void processed_frames(uint32_t nFrames) = 0;
        virtual void set_current_song(Song* s) = 0;

        // Convenience interface (mostly for GUI)
        /**
         * Returns an approximation of the current frame number.  It is expected
         * that this function be fast... so an approximation within a process()
         * cycle or two is fine.
         */
        virtual uint32_t get_current_frame(void) = 0;
    };

}

#endif // TRANSPORTMASTERINTERFACE_H
