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
#ifndef TRITIUM_SIMPLETRANSPORTMASTER_HPP
#define TRITIUM_SIMPLETRANPOSRTMASTER_HPP

#include <Tritium/Transport.hpp>

namespace Tritium
{
    struct TransportPosition;
    class SimpleTransportMasterPrivate;

    /**
     * This defines a very simple transport master for Tritium.
     */
    class SimpleTransportMaster : public Transport
    {
    public:
        SimpleTransportMaster();
        virtual ~SimpleTransportMaster() {}

        // Normal transport controls
        int locate(uint32_t frame);
        int locate(uint32_t bar, uint32_t beat, uint32_t tick);
        void start(void);
        void stop(void);
        void get_position(TransportPosition* pos);

        // Interface for sequencer.  At the end of process(), declare the number
        // of frames processed.  This is needed so that the internal transport
        // master can keep track of time.
        void processed_frames(uint32_t nFrames);
        void set_current_song(T<Song>::shared_ptr s);

        // Convenience interface (mostly for GUI)
        virtual uint32_t get_current_frame(void);
	virtual TransportPosition::State get_state();

    private:
        SimpleTransportMasterPrivate* d;
    };


} // namespace Tritium

#endif // TRITIUM_SIMPLETRANSPORTMASTER_HPP
