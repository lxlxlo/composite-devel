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
#ifndef JACKTRANSPORTMASTER_H
#define JACKTRANSPORTMASTER_H

#include <Tritium/Transport.h>

namespace H2Core
{
    struct TransportPosition;
    class JackTransportMasterPrivate;

    /**
     * This class is the interface between Hydrogen and the JACK Transport.  it
     * is *NOT* the JACK Transport Master.  It is a Jack Transport Client.  This
     * is used whether Hydrogen is the JACK transport master or not.
     *
     * When Hydrogen is a JACK transport slave, it looks like this:
     *
     * jackd --> jack_position_t --> JackTransportMaster
     *            --> H2Core::Transport --> Sequencer (Hydrogen)
     *
     * When Hydrogen is the JACK transport master, it looks like this:
     *
     * HydrogenBasicTransportMaster --> JackTimebaseCallback --> jackd
     *    --> jack_position_t --> JackTransportMaster
     *           --> H2Core::Transport --> Sequencer (Hydrogen)
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
        virtual void set_current_song(Song* s);

        // Convenience interface (mostly for GUI)
        virtual uint32_t get_current_frame(void);

    private:
        JackTransportMasterPrivate* d;
    };


}

#endif // JACKTRANSPORTMASTER_H
