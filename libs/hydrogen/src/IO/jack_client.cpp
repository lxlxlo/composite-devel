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
/* JackClient.cpp
 * Copyright(c) 2008 by Gabriel M. Beddingfield <gabriel@teuton.org>
 */

#include "JackClient.h"
#include <jack/jack.h>
#include <hydrogen/Object.h>

#ifdef JACK_SUPPORT

#ifdef LASH_SUPPORT
#include <hydrogen/LashClient.h>
#endif

using namespace std;
namespace H2Core
{

JackClient* JackClient::instance = NULL;

JackClient* JackClient::get_instance(bool init_jack)
{
	if (instance == NULL) {
		instance = new JackClient;
	}
	if (init_jack && (instance->m_client == NULL)) {
		instance->open();
	}
	return instance;
}

jack_client_t* JackClient::ref(void)
{
	return m_client;
}

JackClient::JackClient()
	: Object( "JackClient" ),
	  m_client(0),
	  m_audio_process(0),
	  m_nonaudio_process(0)
{
	INFOLOG( "INIT" );
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

	QString sClientName = "Hydrogen";
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
			CLIENT_SUCCESS("JACK Server started for Hydrogen.");
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

#ifdef LASH_SUPPORT
	LashClient* lashClient = LashClient::getInstance();
	if (lashClient && lashClient->isConnected())
	{
		lashClient->sendJackClientName();
	}
#endif
}

JackClient::~JackClient()
{
	INFOLOG( "DESTROY" );
	close();
}

void JackClient::close(void)
{
	int rv;
	if(m_client) {
		rv = jack_deactivate(m_client); // return value ignored
		if (rv) WARNINGLOG("jack_deactive(m_client) reported an error");
		jack_client_close(m_client);  // Ignore return value
		if (rv) WARNINGLOG("jack_client_close(m_client) reported an error");
		m_client = 0;
	}    
}

int JackClient::setAudioProcessCallback(JackProcessCallback process)
{
	if (jack_deactivate(m_client)) {
		ERRORLOG("Could not deactivate JACK client.");
	}
	int rv = jack_set_process_callback(m_client, process, 0);
	if (!rv) {
		INFOLOG("JACK Callback changed.");
		m_audio_process = process;
	}
	if (jack_activate(m_client)) {
		ERRORLOG("Could not activate JACK client");
	}
	return rv;
}

int JackClient::setNonAudioProcessCallback(JackProcessCallback process)
{
	if (jack_deactivate(m_client)) {
		ERRORLOG("Could not deactivate JACK client.");
	}
	int rv = 0;
	if (!m_audio_process) {
		INFOLOG("No current audio process callback... setting the non-audio one.");
		rv = jack_set_process_callback(m_client, process, 0);
	}
	if (!rv) {
		INFOLOG("Non-audio process callback changed.");
		m_nonaudio_process = process;
	} else {
		ERRORLOG("Could not set the non-audio process callback.");
	}
	if (jack_activate(m_client)) {
		ERRORLOG("Could not activate JACK client");
	}
	return rv;
}

int JackClient::clearAudioProcessCallback(void)
{
	int rv = 0;
	if (!m_audio_process) {
		return rv;
	}
	if (jack_deactivate(m_client)) {
		ERRORLOG("Could not deactivate JACK client");
	}
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
	if (jack_activate(m_client)) {
		ERRORLOG("Could not activate JACK client.");
	}		
	m_audio_process = 0;
	return rv;
}

int JackClient::clearNonAudioProcessCallback(void)
{
	int rv = 0;
	if (!m_audio_process) {
		if (jack_deactivate(m_client)) {
			ERRORLOG("Could not deactivate JACK client.");
		}
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
	while (*port_names) {
		ports.push_back(QString(*port_names));
		++port_names;
	}
	free((void*)port_names);
	return ports;
}

} // namespace H2Core

#endif // JACK_SUPPORT
