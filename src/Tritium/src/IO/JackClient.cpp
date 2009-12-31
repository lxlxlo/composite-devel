/*
 * Copyright(c) 2008,2009 by Gabriel M. Beddingfield <gabriel@teuton.org>
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
/* JackClient.cpp
 * Copyright(c) 2008 by Gabriel M. Beddingfield <gabriel@teuton.org>
 */

#include "JackClient.hpp"
#include <Tritium/Logger.hpp>
#include <Tritium/IO/JackOutput.hpp>
#include <jack/jack.h>
#include <cassert>

#ifdef JACK_SUPPORT

using namespace std;
namespace Tritium
{

jack_client_t* JackClient::ref(void)
{
	return m_client;
}

JackClient::JackClient(Engine* parent, bool init_jack) :
    m_engine(parent),
    m_client(0),
    m_audio_process(0),
    m_audio_process_arg(0),
    m_nonaudio_process(0)
{
	INFOLOG( "INIT" );
	assert(parent);
	if(init_jack)
	    open();
}

#define CLIENT_FAILURE(msg) {						\
		ERRORLOG("Could not connect to JACK server (" msg ")"); \
		if (m_client) {						\
			ERRORLOG("...but JACK returned a non-null pointer?"); \
			(m_client) = 0;					\
		}							\
		if (tries) ERRORLOG("...trying again.");		\
	}


#define CLIENT_SUCCESS(msg) {				\
		assert(m_client);			\
		INFOLOG(msg);				\
		tries = 0;				\
	}

void JackClient::open(void)
{
	if(m_client) return;

	QString sClientName = "Tritium";
	jack_status_t status;
	int tries = 2;  // Sometimes jackd doesn't stop and start fast enough.
	while ( tries > 0 ) {
		--tries;
		m_client = jack_client_open(
			sClientName.toAscii(),
			JackNullOption,
			&status);
		switch(status) {
		case JackFailure:
			CLIENT_FAILURE("unknown error");
			break;
		case JackInvalidOption:
			CLIENT_FAILURE("invalid option");
			break;
		case JackNameNotUnique:
			if (m_client) {
				sClientName = jack_get_client_name(m_client);
				CLIENT_SUCCESS(QString("Jack assigned the client name '%1'")
					       .arg(sClientName));
			} else {
				CLIENT_FAILURE("name not unique");
			}
			break;
		case JackServerStarted:
			CLIENT_SUCCESS("JACK Server started for Tritium.");
			break;
		case JackServerFailed:
			CLIENT_FAILURE("unable to connect");
			break;
		case JackServerError:
			CLIENT_FAILURE("communication error");
			break;
		case JackNoSuchClient:
			CLIENT_FAILURE("unknown client type");
			break;
		case JackLoadFailure:
			CLIENT_FAILURE("can't load internal client");
			break;
		case JackInitFailure:
			CLIENT_FAILURE("can't initialize client");
			break;
		case JackShmFailure:
			CLIENT_FAILURE("unable to access shared memory");
			break;
		case JackVersionError:
			CLIENT_FAILURE("client/server protocol version mismatch");
		default:
			if (status) {
				ERRORLOG("Unknown status with JACK server.");
				if (m_client) {
					CLIENT_SUCCESS("Client pointer is *not* null..."
						       " assuming we're OK");
				}
			} else {
				CLIENT_SUCCESS("Connected to JACK server");
			}				
		}
	}

	// Here, m_client should either be valid, or NULL.	

}

JackClient::~JackClient()
{
	INFOLOG( "DESTROY" );
	close();
}

void JackClient::raise_error(Engine::error_t err)
{
	m_engine->raiseError(err);
}

bool JackClient::jack_is_up()
{
    bool rv;
    T<AudioOutput>::shared_ptr ao = m_engine->get_audio_output();
    try {
	if( ao
	    && dynamic_cast<JackOutput*>(ao.get())
	    && ref() /* client pointer */ ) {
	    rv = true;
	} else {
	    rv = false;
	}
    } catch (...) {
	rv = false;
    }
    return rv;
}

void JackClient::close(void)
{
	int rv;
	if(m_client) {
		deactivate();
		jack_client_close(m_client);  // Ignore return value
		if (rv) WARNINGLOG("jack_client_close(m_client) reported an error");
		m_client = 0;
	}    
}

void JackClient::activate()
{
	if(m_client) {
		int rv;
		rv = jack_activate(m_client);
		if(rv) {
			ERRORLOG("Could not activate JACK client.");
		}
	}
}

void JackClient::deactivate()
{
	if(m_client) {
		int rv;
		rv = jack_deactivate(m_client);
		if(rv) {
			ERRORLOG("Could not deactivate JACK client.");
		}
	}
}
		

int JackClient::setAudioProcessCallback(JackProcessCallback process, void* arg)
{
	deactivate();
	int rv = jack_set_process_callback(m_client, process, arg);
	if (!rv) {
		INFOLOG("JACK Callback changed.");
		m_audio_process = process;
		m_audio_process_arg = arg;
	}
	return rv;
}

int JackClient::setNonAudioProcessCallback(JackProcessCallback process)
{
	deactivate();
	int rv = 0;
	if (!m_audio_process) {
		INFOLOG("No current audio process callback... setting the non-audio one.");
		assert(m_audio_process_arg);
		rv = jack_set_process_callback(m_client, process, m_audio_process_arg);
	}
	if (!rv) {
		INFOLOG("Non-audio process callback changed.");
		m_nonaudio_process = process;
	} else {
		ERRORLOG("Could not set the non-audio process callback.");
	}
	return rv;
}

int JackClient::clearAudioProcessCallback(void)
{
	int rv = 0;
	if (!m_audio_process) {
		return rv;
	}
	deactivate();
	// make sure the process cycle is over before killing anything
	if (m_nonaudio_process) {
		INFOLOG("Switching to non-audio process");
		rv = jack_set_process_callback(m_client, m_nonaudio_process, 0);
	}
	if (m_nonaudio_process && rv) {
		ERRORLOG("Could not switch to non-audio process");
		rv = jack_set_process_callback(m_client, 0, 0);
		m_nonaudio_process = 0;
		if (rv) ERRORLOG("JACK returned an error when clearing the process callback.");
	}
	m_audio_process = 0;
	return rv;
}

int JackClient::clearNonAudioProcessCallback(void)
{
	int rv = 0;
	if (!m_audio_process) {
		deactivate();
		rv = jack_set_process_callback(m_client, 0, 0);
		if (rv) {
			ERRORLOG("JACK returned an error when clearing out the process callback.");
		}
	}
	m_nonaudio_process = 0;
	return rv;
}

void JackClient::subscribe(void* child_obj)
{
	m_children.insert(child_obj);
	INFOLOG(QString("JackClient subscribers: %1").arg(m_children.size()));
}

void JackClient::unsubscribe(void* child_obj)
{
	INFOLOG(QString("JackClient subscribers (before): %1").arg(m_children.size()));
	if (m_children.size() == 0)
		return;
	std::set<void*>::iterator pos = m_children.find(child_obj);
	if (pos != m_children.end()) {
		m_children.erase(pos);
	}
	INFOLOG(QString("JackClient subscribers (after): %1").arg(m_children.size()));
	if (m_children.size() == 0) {
		INFOLOG("JackClient is closing.");
		close();
	}
}

std::vector<QString> JackClient::getMidiOutputPortList(void)
{
	vector<QString> ports;
	const char **port_names = 0;
	port_names = jack_get_ports(m_client,
				    0,
				    JACK_DEFAULT_MIDI_TYPE,
				    JackPortIsOutput);
	if (!port_names) return ports;
	int k = 0;
	while (port_names[k]) {
		ports.push_back(QString(port_names[k]));
		++k;
	}
	free((void*)port_names);
	return ports;
}

} // namespace Tritium

#endif // JACK_SUPPORT
