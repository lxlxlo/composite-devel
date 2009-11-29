/*
 * Copyright(c) 2002-2008 by Alex >Comix< Cominu [comix@users.sourceforge.net]
 *
 * This file is part of Composite
 *
 * Composite is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Composite is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY, without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include "AudioEngineInfoForm.hpp"

#include <QtGui>

#include "HydrogenApp.hpp"

#include <Tritium/Pattern.hpp>
#include <Tritium/Preferences.hpp>
#include <Tritium/Hydrogen.hpp>
#include <Tritium/Transport.hpp>
#include <Tritium/IO/MidiInput.hpp>
#include <Tritium/IO/AudioOutput.hpp>
#include <Tritium/sampler/Sampler.hpp>
#include <Tritium/AudioEngine.hpp>
using namespace Tritium;

#include "Skin.hpp"

AudioEngineInfoForm::AudioEngineInfoForm(QWidget* parent)
 : QWidget( parent )
{
	setupUi( this );

	setMinimumSize( width(), height() );	// not resizable
	setMaximumSize( width(), height() );	// not resizable

	setWindowTitle( trUtf8( "Audio Engine Info" ) );

	updateInfo();
	//currentPatternLbl->setText("NULL pattern");

	timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), this, SLOT(updateInfo()));

	HydrogenApp::get_instance()->addEventListener( this );
	updateAudioEngineState();
}



/**
 * Destructor
 */
AudioEngineInfoForm::~AudioEngineInfoForm()
{
}




/**
 * show event
 */
void AudioEngineInfoForm::showEvent ( QShowEvent* )
{
	updateInfo();
	timer->start(200);
}




/**
 * hide event
 */
void AudioEngineInfoForm::hideEvent ( QHideEvent* )
{
	timer->stop();
}




void AudioEngineInfoForm::updateInfo()
{
	Hydrogen *pEngine = Hydrogen::get_instance();
	Song *song = pEngine->getSong();

	// Song position
	QString sSongPos = "N/A";
	if ( pEngine->getPatternPos() != -1 ) {
		sSongPos = QString::number( pEngine->getPatternPos() );
	}
	m_pSongPositionLbl->setText( sSongPos );


	// Audio engine Playing notes
	char tmp[100];

	// Process time
	int perc = 0;
	if ( pEngine->getMaxProcessTime() != 0.0 ) {
		perc= (int)( pEngine->getProcessTime() / ( pEngine->getMaxProcessTime() / 100.0 ) );
	}
	sprintf(tmp, "%#.2f / %#.2f  (%d%%)", pEngine->getProcessTime(), pEngine->getMaxProcessTime(), perc );
	processTimeLbl->setText(tmp);

	// Song state
	if (song == NULL) {
		songStateLbl->setText( "NULL song" );
	}
	else {
		if (song->__is_modified) {
			songStateLbl->setText( "Modified" );
		}
		else {
			songStateLbl->setText( "Saved" );
		}
	}

	// tick number
	sprintf(tmp, "%03d", (int)pEngine->getTickPosition() );
	nTicksLbl->setText(tmp);



	// Audio driver info
	AudioOutput *driver = pEngine->getAudioOutput();
	if (driver) {
		QString audioDriverName = "Jack";
		driverLbl->setText(audioDriverName);

		// Audio driver buffer size
		bufferSizeLbl->setText(
                    QString("%1").arg(driver->getBufferSize())
                    );

		// Audio driver sampleRate
		sampleRateLbl->setText(
                    QString("%1").arg(driver->getSampleRate())
                    );

		// Number of frames
		nFramesLbl->setText(
                    QString("%1").arg(Hydrogen::get_instance()->get_transport()->get_current_frame())
                    );
	}
	else {
		driverLbl->setText( "NULL driver" );
		bufferSizeLbl->setText( "N/A" );
		sampleRateLbl->setText( "N/A" );
		nFramesLbl->setText( "N/A" );
	}
	nRealtimeFramesLbl->setText( QString( "%1" ).arg( pEngine->getRealtimeFrames() ) );


	// Midi driver info
	MidiInput *pMidiDriver = pEngine->getMidiInput();
	if (pMidiDriver) {
		midiDriverName->setText( "Jack" );
	}
	else {
		midiDriverName->setText("No MIDI driver support");
	}

	m_pMidiDeviceName->setText( Preferences::get_instance()->m_sMidiPortName );


	int nSelectedPatternNumber = pEngine->getSelectedPatternNumber();
	if (nSelectedPatternNumber == -1) {
		selectedPatLbl->setText( "N/A");
	}
	else {
		selectedPatLbl->setText( QString("%1").arg(nSelectedPatternNumber) );
	}

	int nSelectedInstrumentNumber = pEngine->getSelectedInstrumentNumber();
	if (nSelectedInstrumentNumber == -1) {
		m_pSelectedInstrLbl->setText( "N/A" );
	}
	else {
		m_pSelectedInstrLbl->setText( QString("%1").arg(nSelectedInstrumentNumber) );
	}


	QString currentPatternName;
	PatternList *pPatternList = Hydrogen::get_instance()->getCurrentPatternList();
	if (pPatternList) {
		currentPatternLbl->setText( QString::number(pPatternList->get_size()) );
	}
	else {
		currentPatternLbl->setText( "N/A" );
	}

	// SAMPLER
	Sampler *pSampler = AudioEngine::get_instance()->get_sampler();
	sampler_playingNotesLbl->setText(QString( "%1 / %2" ).arg(pSampler->get_playing_notes_number()).arg(Preferences::get_instance()->m_nMaxNotes));

}






/**
 * Update engineStateLbl with the current audio engine state
 */
void AudioEngineInfoForm::updateAudioEngineState() {
	// Audio Engine state
	QString stateTxt;
	int state = Hydrogen::get_instance()->getState();
	TransportPosition::State xstate = Hydrogen::get_instance()->get_transport()->get_state();
	switch (state) {
	case STATE_UNINITIALIZED:
		stateTxt = "Uninitialized";
		break;

	case STATE_INITIALIZED:
		stateTxt = "Initialized";
		break;

	case STATE_PREPARED:
		stateTxt = "Prepared";
		break;

	case STATE_READY:
		if( xstate == TransportPosition::ROLLING ) {
			stateTxt = "Ready/Playing";
		} else {
			stateTxt = "Ready/Stopped";
		}
		break;

	default:
		stateTxt = "Unknown!?";
		break;
	}
	engineStateLbl->setText(stateTxt);
}


void AudioEngineInfoForm::stateChangedEvent( int )
{
	updateAudioEngineState();
}


void AudioEngineInfoForm::patternChangedEvent()
{
	updateAudioEngineState();
}

