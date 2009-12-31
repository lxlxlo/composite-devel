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


#include "Skin.hpp"
#include "PreferencesDialog.hpp"
#include "CompositeApp.hpp"
#include "MainForm.hpp"

#include "qmessagebox.h"
#include "qstylefactory.h"

#include <QPixmap>
#include <QFontDialog>
#include <QMessageBox>

#include "widgets/midiTable.hpp"

#include <Tritium/MidiMap.hpp>
#include <Tritium/Engine.hpp>
#include <Tritium/Preferences.hpp>
#include <Tritium/IO/MidiInput.hpp>
#include <Tritium/Logger.hpp>
#include <Tritium/memory.hpp>

using namespace Tritium;

PreferencesDialog::PreferencesDialog(QWidget* parent)
 : QDialog( parent )
{
	setupUi( this );

	setWindowTitle( trUtf8( "Preferences" ) );
//	setIcon( QPixmap( Skin::getImagePath()  + "/icon16.png" ) );

	setMinimumSize( width(), height() );
	setMaximumSize( width(), height() );

	T<Preferences>::shared_ptr pPref = g_engine->get_preferences();
	pPref->loadPreferences( false );	// reload user's preferences

	driverComboBox->clear();
	driverComboBox->addItem( "Auto" );
	driverComboBox->addItem( "JACK" );

	// Selected audio Driver
	QString sAudioDriver = pPref->m_sAudioDriver;
	if (sAudioDriver == "Auto") {
		driverComboBox->setCurrentIndex(0);
	}
	else if (sAudioDriver == "Jack") {
		driverComboBox->setCurrentIndex(1);
	}
	else {
		ERRORLOG( "Unknown audio driver from preferences [" + sAudioDriver + "]" );
	}


	m_pMidiDriverComboBox->clear();
	m_pMidiDriverComboBox->addItem( "JackMidi" );

	if ( pPref->m_sMidiDriver == "JackMidi" ) {
		m_pMidiDriverComboBox->setCurrentIndex(0);
	}
	else {
		ERRORLOG( "Unknown midi input from preferences [" + pPref->m_sMidiDriver + "]" );
	}

	m_pIgnoreNoteOffCheckBox->setChecked( pPref->m_bMidiNoteOffIgnore );

	updateDriverInfo();


	// metronome volume
	uint metronomeVol = (uint)( pPref->m_fMetronomeVolume * 100.0 );
	metronomeVolumeSpinBox->setValue(metronomeVol);

	// max voices
	maxVoicesTxt->setValue( pPref->m_nMaxNotes );

	// JACK
	trackOutsCheckBox->setChecked( pPref->m_bJackTrackOuts );
	connectDefaultsCheckBox->setChecked( pPref->m_bJackConnectDefaults );
	trackOutputComboBox->setCurrentIndex( pPref->m_nJackTrackOutputMode );
	//~ JACK


	bufferSizeSpinBox->setValue( pPref->m_nBufferSize );

	switch ( pPref->m_nSampleRate ) {
		case 44100:
			sampleRateComboBox->setCurrentIndex( 0 );
			break;
		case 48000:
			sampleRateComboBox->setCurrentIndex( 1 );
			break;
		case 88200:
			sampleRateComboBox->setCurrentIndex( 2 );
			break;
		case 96000:
			sampleRateComboBox->setCurrentIndex( 3 );
			break;
		default:
			ERRORLOG( QString("Wrong samplerate: %1").arg( pPref->m_nSampleRate ) );
	}


	// Appearance tab
	QString applicationFamily = pPref->getApplicationFontFamily();
	int applicationPointSize = pPref->getApplicationFontPointSize();

	QFont applicationFont( applicationFamily, applicationPointSize );
	applicationFontLbl->setFont( applicationFont );
	applicationFontLbl->setText( applicationFamily + QString("  %1").arg( applicationPointSize ) );

	QString mixerFamily = pPref->getMixerFontFamily();
	int mixerPointSize = pPref->getMixerFontPointSize();
	QFont mixerFont( mixerFamily, mixerPointSize );
	mixerFontLbl->setFont( mixerFont );
	mixerFontLbl->setText( mixerFamily + QString("  %1").arg( mixerPointSize ) );


	float falloffSpeed = pPref->getMixerFalloffSpeed();
	if (falloffSpeed == FALLOFF_SLOW) {
		mixerFalloffComboBox->setCurrentIndex(0);
	}
	else if (falloffSpeed == FALLOFF_NORMAL) {
		mixerFalloffComboBox->setCurrentIndex(1);
	}
	else if (falloffSpeed == FALLOFF_FAST) {
		mixerFalloffComboBox->setCurrentIndex(2);
	}
	else {
		ERRORLOG( QString("PreferencesDialog: wrong mixerFalloff value = %1").arg(falloffSpeed) );
	}

	// Style
	QStringList list = QStyleFactory::keys();
	uint i = 0;
	for ( QStringList::Iterator it = list.begin(); it != list.end(); it++) {
		styleComboBox->addItem( *it );
		//INFOLOG( "QT Stile: " + *it   );
		//string sStyle = (*it).latin1();
		QString sStyle = (*it);
		if (sStyle == pPref->getQTStyle() ) {
			styleComboBox->setCurrentIndex( i );
		}
		i++;
	}


	// midi tab
	midiPortChannelComboBox->setEnabled( false );
	midiPortComboBox->setEnabled( false );
	// list midi output ports
	midiPortComboBox->clear();
	midiPortComboBox->addItem( "None" );
	if ( g_engine->get_midi_input() ) {
		std::vector<QString> midiOutList = g_engine->get_midi_input()->getOutputPortList();

		if ( midiOutList.size() != 0 ) {
			midiPortComboBox->setEnabled( true );
			midiPortChannelComboBox->setEnabled( true );
		}
		for (uint i = 0; i < midiOutList.size(); i++) {
			QString sPortName = midiOutList[i];
			midiPortComboBox->addItem( sPortName );

			if ( sPortName == pPref->m_sMidiPortName ) {
				midiPortComboBox->setCurrentIndex( i + 1 );
			}
		}
	}

	if ( pPref->m_nMidiChannelFilter == -1 ) {
		midiPortChannelComboBox->setCurrentIndex( 0 );
	}
	else {
		midiPortChannelComboBox->setCurrentIndex( pPref->m_nMidiChannelFilter + 1 );
	}

	// General tab
	restoreLastUsedSongCheckbox->setChecked( pPref->isRestoreLastSongEnabled() );

	sBcountOffset->setValue( pPref->m_countOffset );
	sBstartOffset->setValue( pPref->m_startOffset );

	m_bNeedDriverRestart = false;
}




PreferencesDialog::~PreferencesDialog()
{	
	INFOLOG("~PREFERENCES_DIALOG");	
}



void PreferencesDialog::on_cancelBtn_clicked()
{
	T<Preferences>::shared_ptr preferencesMng = g_engine->get_preferences();
	preferencesMng->loadPreferences( false );	// reload old user's preferences

	reject();
}


void PreferencesDialog::on_okBtn_clicked()
{
	m_bNeedDriverRestart = true;

	T<Preferences>::shared_ptr pPref = g_engine->get_preferences();

	midiTable->saveMidiTable();

	// Selected audio driver
	if (driverComboBox->currentText() == "Auto" ) {
		pPref->m_sAudioDriver = "Auto";
	}
	else if (driverComboBox->currentText() == "JACK" ) {
		pPref->m_sAudioDriver = "Jack";
	}
	else {
		ERRORLOG( "[okBtnClicked] Invalid audio driver" );
	}

	// JACK
	pPref->m_bJackTrackOuts = trackOutsCheckBox->isChecked();
	pPref->m_bJackConnectDefaults = connectDefaultsCheckBox->isChecked();

	
	if (trackOutputComboBox->currentText() == "Post-Fader")
	{
		pPref->m_nJackTrackOutputMode = Preferences::POST_FADER;
	} else {
		pPref->m_nJackTrackOutputMode = Preferences::PRE_FADER;
	}
	//~ JACK

	pPref->m_nBufferSize = bufferSizeSpinBox->value();
	if ( sampleRateComboBox->currentText() == "44100" ) {
		pPref->m_nSampleRate = 44100;
	}
	else if ( sampleRateComboBox->currentText() == "48000" ) {
		pPref->m_nSampleRate = 48000;
	}
	else if ( sampleRateComboBox->currentText() == "88200" ) {
		pPref->m_nSampleRate = 88200;
	}
	else if ( sampleRateComboBox->currentText() == "96000" ) {
		pPref->m_nSampleRate = 96000;
	}


	// metronome
	pPref->m_fMetronomeVolume = (metronomeVolumeSpinBox->value()) / 100.0;

	// maxVoices
	pPref->m_nMaxNotes = maxVoicesTxt->value();



	if ( m_pMidiDriverComboBox->currentText() == "JackMidi" ) {
		pPref->m_sMidiDriver = "JackMidi";
	}

	pPref->m_bMidiNoteOffIgnore = m_pIgnoreNoteOffCheckBox->isChecked();

	// Mixer falloff
	QString falloffStr = mixerFalloffComboBox->currentText();
	if ( falloffStr== trUtf8("Slow") ) {
		pPref->setMixerFalloffSpeed(FALLOFF_SLOW);
	}
	else if ( falloffStr == trUtf8("Normal") ) {
		pPref->setMixerFalloffSpeed(FALLOFF_NORMAL);
	}
	else if ( falloffStr == trUtf8("Fast") ) {
		pPref->setMixerFalloffSpeed(FALLOFF_FAST);
	}
	else {
		ERRORLOG( "[okBtnClicked] Unknown mixerFallOffSpeed: " + falloffStr );
	}

	QString sNewMidiPortName = midiPortComboBox->currentText();

	if ( pPref->m_sMidiPortName != sNewMidiPortName ) {
		pPref->m_sMidiPortName = sNewMidiPortName;
		m_bNeedDriverRestart = true;
	}

	if ( pPref->m_nMidiChannelFilter != midiPortChannelComboBox->currentIndex() - 1 ) {
		m_bNeedDriverRestart = true;
	}
	pPref->m_nMidiChannelFilter = midiPortChannelComboBox->currentIndex() - 1;


	// General tab
	pPref->setRestoreLastSongEnabled( restoreLastUsedSongCheckbox->isChecked() );

	pPref->m_countOffset = sBcountOffset->value();
	pPref->m_startOffset = sBstartOffset->value();
	g_engine->setBcOffsetAdjust();

	pPref->savePreferences();

	

	if (m_bNeedDriverRestart) {
		(g_engine)->restartDrivers();
	}
	accept();
}



void PreferencesDialog::on_driverComboBox_activated( int /*index*/ )
{
	QString selectedDriver = driverComboBox->currentText();
#ifdef JACK_SUPPORT
	if (m_pMidiDriverComboBox->currentText() == "JackMidi"
	    && selectedDriver != "JACK") {
		QMessageBox::StandardButtons res;
		res = QMessageBox::question(
			this,
			tr("Jack driver mismatch"),
			tr("The JACK MIDI driver requires the JACK Audio driver."
			   "  <b>Do you really want to change the audio driver?</b>"
			   "  If yes, the MIDI driver will be set to Alsa."),
			QMessageBox::Yes|QMessageBox::No,
			QMessageBox::Yes);
		if (res == QMessageBox::Yes) {
			int index = m_pMidiDriverComboBox->findText("Alsa");
			if (index < 0) index = 0;
			m_pMidiDriverComboBox->setCurrentIndex(index);
		} else {
			int index = driverComboBox->findText("JACK");
			if (index < 0) index = 0;
			driverComboBox->setCurrentIndex(index);
		}			
	}
#endif // JACK_SUPPORT
	updateDriverInfo();
	m_bNeedDriverRestart = true;
}


void PreferencesDialog::updateDriverInfo()
{
	T<Preferences>::shared_ptr pPref = g_engine->get_preferences();
	QString info;

	bool bJack_support = false;
	#ifdef JACK_SUPPORT
	bJack_support = true;
	#endif

	if ( driverComboBox->currentText() == "Auto" ) {
		info += trUtf8("<b>Automatic driver selection</b>");

		m_pAudioDeviceTxt->setEnabled(false);
		m_pAudioDeviceTxt->setText( "" );
		bufferSizeSpinBox->setEnabled( false );
		sampleRateComboBox->setEnabled( false );
		trackOutputComboBox->setEnabled( false );
		connectDefaultsCheckBox->setEnabled( false );
	}
	else if ( driverComboBox->currentText() == "JACK" ) {	// JACK
		info += trUtf8("<b>Jack Audio Connection Kit Driver</b><br>Low latency audio driver");
		if ( !bJack_support ) {
			info += trUtf8("<br><b><font color=\"red\">Not compiled</font></b>");
		}
		m_pAudioDeviceTxt->setEnabled(false);
		m_pAudioDeviceTxt->setText( "" );
		bufferSizeSpinBox->setEnabled(false);
		sampleRateComboBox->setEnabled(false);
		trackOutputComboBox->setEnabled( true );
		connectDefaultsCheckBox->setEnabled(true);
		trackOutsCheckBox->setEnabled( true );
	}
	else {
		QString selectedDriver = driverComboBox->currentText();
		ERRORLOG( "Unknown driver = " + selectedDriver );
	}

	metronomeVolumeSpinBox->setEnabled(true);
	bufferSizeSpinBox->setValue( pPref->m_nBufferSize );

	driverInfoLbl->setText(info);
}



void PreferencesDialog::on_selectApplicationFontBtn_clicked()
{
	T<Preferences>::shared_ptr preferencesMng = g_engine->get_preferences();

	QString family = preferencesMng->getApplicationFontFamily();
	int pointSize = preferencesMng->getApplicationFontPointSize();

	bool ok;
	QFont font = QFontDialog::getFont( &ok, QFont( family, pointSize ), this );
	if ( ok ) {
		// font is set to the font the user selected
		family = font.family();
		pointSize = font.pointSize();
		QString familyStr = family;
		preferencesMng->setApplicationFontFamily(familyStr);
		preferencesMng->setApplicationFontPointSize(pointSize);
	} else {
		// the user cancelled the dialog; font is set to the initial
		// value, in this case Times, 12.
	}

	QFont newFont(family, pointSize);
	applicationFontLbl->setFont(newFont);
	applicationFontLbl->setText(family + QString("  %1").arg(pointSize));
}




void PreferencesDialog::on_bufferSizeSpinBox_valueChanged( int /*i*/ )
{
	m_bNeedDriverRestart = true;
}




void PreferencesDialog::on_sampleRateComboBox_editTextChanged( const QString&  )
{
	m_bNeedDriverRestart = true;
}



void PreferencesDialog::on_restartDriverBtn_clicked()
{
	g_engine->restartDrivers();
	m_bNeedDriverRestart = false;
}



void PreferencesDialog::on_selectMixerFontBtn_clicked()
{
	T<Preferences>::shared_ptr preferencesMng = g_engine->get_preferences();

	QString family = preferencesMng->getMixerFontFamily();
	int pointSize = preferencesMng->getMixerFontPointSize();

	bool ok;
	QFont font = QFontDialog::getFont( &ok, QFont( family, pointSize ), this );
	if ( ok ) {
		// font is set to the font the user selected
		family = font.family();
		pointSize = font.pointSize();
		QString familyStr = family;
		preferencesMng->setMixerFontFamily(familyStr);
		preferencesMng->setMixerFontPointSize(pointSize);
	}
	QFont newFont(family, pointSize);
	mixerFontLbl->setFont(newFont);
	mixerFontLbl->setText(family + QString("  %1").arg(pointSize));
}

void PreferencesDialog::on_m_pMidiDriverComboBox_currentIndexChanged( const QString& midi_driver )
{
#ifdef JACK_SUPPORT
	if (midi_driver == "JackMidi"
	    && driverComboBox->currentText() != "JACK") {
		QMessageBox::StandardButtons res;
		res = QMessageBox::question(
			this,
			tr("Jack driver mismatch"),
			tr("The Jack MIDI driver requires the Jack Audio driver."
			   "  <b>Do you really want to change the MIDI driver?</b>"
			   "  If yes, the Audio driver will be set to JACK."
			   "  If no, the MIDI driver will be set to Alsa."),
			QMessageBox::Yes|QMessageBox::No,
			QMessageBox::Yes);
		if (res == QMessageBox::Yes) {
			int index = driverComboBox->findText("JACK");
			if (index < 0) index = 0;
			driverComboBox->setCurrentIndex(index);
		}
		#warning "XXX TODO: ...else??"
	}
#endif // JACK_SUPPORT
	m_bNeedDriverRestart = true;
}

void PreferencesDialog::on_midiPortComboBox_activated( int /*index*/ )
{
	m_bNeedDriverRestart = true;
}



void PreferencesDialog::on_styleComboBox_activated( int /*index*/ )
{
	QApplication *pQApp = (CompositeApp::get_instance())->getMainForm()->m_pQApp;
	QString sStyle = styleComboBox->currentText();
	pQApp->setStyle( sStyle );

	T<Preferences>::shared_ptr pPref = g_engine->get_preferences();
	pPref->setQTStyle( sStyle );
}
