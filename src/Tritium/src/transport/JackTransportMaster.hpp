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
#ifndef TRITIUM_JACKTRANSPORTMASTER_HPP
#define TRITIUM_JACKTRANSPORTMASTER_HPP

#include <Tritium/Transport.hpp>

namespace Tritium
{
    struct TransportPosition;
    class JackTransportMasterPrivate;

    /**
     * This class is the interface between Engine and the JACK Transport.  it
     * is *NOT* the JACK Transport Master.  It is a Jack Transport Client.  This
     * is used whether Engine is the JACK transport master or not.
     *
     * When Engine is a JACK transport slave, it looks like this:
     *
     * jackd --> jack_position_t --> JackTransportMaster
     *            --> Tritium::Transport --> Sequencer (Engine)
     *
     * When Engine is the JACK transport master, it looks like this:
     *
     * EngineBasicTransportMaster --> JackTimebaseCallback --> jackd
     *    --> jack_position_t --> JackTransportMaster
     *           --> Tritium::Transport --> Sequencer (Engine)
     */
    class JackTransportMaster : public Transport
    {
    public:
        JackTransportMaster();
        virtual ~JackTransportMaster() {}

        // Normal transport controls
        virtual int locate(uint32_t frame);
        virtual int locate(uint32_t bar, uint32_t beat, uint32_t tick);
        virtual void start(void);
        virtual void stop(void);
        virtual void get_position(TransportPosition* pos);

        // Interface for sequencer.  At the end of process(), declare the number
        // of frames processed.  This is needed so that the internal transport
        // master can keep track of time.
        virtual void processed_frames(uint32_t nFrames);
        virtual void set_current_song(T<Song>::shared_ptr s);

        // Convenience interface (mostly for GUI)
        virtual uint32_t get_current_frame(void);

    private:
        JackTransportMasterPrivate* d;
    };


} // namespace Tritium

#endif // TRITIUM_JACKTRANSPORTMASTER_HPP
