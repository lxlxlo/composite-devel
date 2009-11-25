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
#ifndef H2CORE_TRANSPORT_H
#define H2CORE_TRANSPORT_H

#include <stdint.h>  // int32_t, uint32_t
#include <Tritium/TransportPosition.h>

namespace Tritium
{
    class Song;

    /**
     * This is the base class for a transport master.  It will be used and
     * controlled by the main transport.  A Transport Master is the one actually
     * in charge of the bars, beats, and ticks.
     */
    class Transport
    {
    public:
        virtual ~Transport() {}

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
	virtual TransportPosition::State get_state() = 0;
    };

}

#endif // H2CORE_TRANSPORT_H
