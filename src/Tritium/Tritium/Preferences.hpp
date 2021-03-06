/*
 * Copyright(c) 2002-2008 by Alex >Comix< Cominu [comix@users.sourceforge.net]
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

#ifndef TRITIUM_PREFERENCES_HPP
#define TRITIUM_PREFERENCES_HPP

#include <list>
#include <vector>
#include <cassert>

#include <Tritium/Action.hpp>
#include <Tritium/globals.hpp>

#include <QString>
#include <QStringList>

class QDomNode;

namespace Tritium
{

const float FALLOFF_SLOW = 	1.08f;
const float FALLOFF_NORMAL=	1.1f;
const float FALLOFF_FAST =	1.5f;

class MidiMap;

/**
\ingroup H2CORE
*/
class WindowProperties
{
public:
	int x;
	int y;
	int width;
	int height;
	bool visible;

	WindowProperties();
	~WindowProperties();

	void set(int _x, int _y, int _width, int _height, bool _visible) {
		x = _x; y = _y;
		width = _width; height = _height;
		visible = _visible;
	}
		
};


/**
\ingroup H2CORE
*/
class H2RGBColor
{
public:
	H2RGBColor( int r = -1, int g = -1, int b = -1 );
	H2RGBColor( const QString& sColor );
	~H2RGBColor();

	QString toStringFmt();

	int getRed() const {
		return m_red;
	}
	int getGreen() const {
		return m_green;
	}
	int getBlue() const {
		return m_blue;
	}

private:
	int m_red;
	int m_green;
	int m_blue;

};


/**
\ingroup H2CORE
\brief	Colors for Composite
*/
class UIStyle
{
public:
	UIStyle();
	H2RGBColor m_songEditor_backgroundColor;
	H2RGBColor m_songEditor_alternateRowColor;
	H2RGBColor m_songEditor_selectedRowColor;
	H2RGBColor m_songEditor_lineColor;
	H2RGBColor m_songEditor_textColor;
	H2RGBColor m_songEditor_pattern1Color;

	H2RGBColor m_patternEditor_backgroundColor;
	H2RGBColor m_patternEditor_alternateRowColor;
	H2RGBColor m_patternEditor_selectedRowColor;
	H2RGBColor m_patternEditor_textColor;
	H2RGBColor m_patternEditor_noteColor;
	H2RGBColor m_patternEditor_lineColor;
	H2RGBColor m_patternEditor_line1Color;
	H2RGBColor m_patternEditor_line2Color;
	H2RGBColor m_patternEditor_line3Color;
	H2RGBColor m_patternEditor_line4Color;
	H2RGBColor m_patternEditor_line5Color;
};



/**
\ingroup H2CORE
\brief	Manager for User Preferences File (singleton)
*/
class Preferences
{
public:
	enum {
		USE_JACK_TRANSPORT = 0,
		USE_JACK_TIME_MASTER = 0,
		POST_FADER = 0,
		SET_PLAY_ON = 0,
		BC_ON = 0,
		NO_JACK_TRANSPORT = 1,
		NO_JACK_TIME_MASTER = 1,
		PRE_FADER = 1,
		SET_PLAY_OFF = 1,
		BC_OFF = 1,
	};


	QString m_sPreferencesFilename;
	QString m_sPreferencesDirectory;

	QString __lastspatternDirectory;
	QString __lastsampleDirectory; // audio file browser
	bool __playsamplesonclicking; // audio file browser

	bool m_bFollowPlayhead;
	
	//soundlibrarypanel expand song and pattern item
	bool __expandSongItem;
	bool __expandPatternItem; 

	//beatcounter
	bool m_bbc;
	bool m_mmcsetplay;
	
	int m_countOffset;
	int m_startOffset;
	//~ beatcounter

	std::list<QString> sServerList;
	std::list<QString> m_patternCategories;

	//___ audio engine properties ___
	QString m_sAudioDriver;		///< Audio Driver
	bool m_bUseMetronome;		///< Use metronome?
	float m_fMetronomeVolume;	///< Metronome volume FIXME: remove this volume!!
	unsigned m_nMaxNotes;		///< max notes
	unsigned m_nBufferSize;		///< Audio buffer size
	unsigned m_nSampleRate;		///< Audio sample rate

	//___ MIDI Driver properties
	QString m_sMidiDriver;
	QString m_sMidiPortName;
	int m_nMidiChannelFilter;
	bool m_bMidiNoteOffIgnore;
	bool m_bMidiDiscardNoteAfterAction;

	//___  alsa audio driver properties ___
	QString m_sAlsaAudioDevice;

	//___  jack driver properties ___
	QString m_sJackPortName1;
	QString m_sJackPortName2;
	bool m_bJackTransportMode;
	bool m_bJackConnectDefaults;
	bool m_bJackTrackOuts;
	int m_nJackTrackOutputMode;
	//jack time master
	bool m_bJackMasterMode ;
	//~ jack time master
	///Default text editor (used by Playlisteditor)
	QString m_sDefaultEditor;

	Preferences();
	~Preferences();

	/// Load the preferences file
	void loadPreferences( bool bGlobal );

	/// Save the preferences file
	void savePreferences();

	const QString& getDemoPath() {
		return demoPath;
	}
	const QString& getDataDirectory() {
		return m_sDataDirectory;
	}
	
	const QString& getDefaultEditor() {
		return m_sDefaultEditor;
	}

	void setDefaultEditor( QString editor){
		m_sDefaultEditor = editor;
	}


	// General
	void setRestoreLastSongEnabled( bool restore ) {
		restoreLastSong = restore;
	}

	void setShowDevelWarning( bool value ) {
		m_bShowDevelWarning = value;
	}

	bool getShowDevelWarning() {
		return m_bShowDevelWarning;
	}

	bool isRestoreLastSongEnabled() {
		return restoreLastSong;
	}

	void setLastSongFilename( const QString& filename ) {
		lastSongFilename = filename;
	}
	const QString& getLastSongFilename() {
		return lastSongFilename;
	}

	void setHearNewNotes( bool value ) {
		hearNewNotes = value;
	}
	bool getHearNewNotes() {
		return hearNewNotes;
	}

	void setRecordEvents( bool value ) {
		recordEvents = value;
	}
	bool getRecordEvents() {
		return recordEvents;
	}

	void setQuantizeEvents( bool value ) {
		quantizeEvents = value;
	}
	bool getQuantizeEvents() {
		return quantizeEvents;
	}

	std::vector<QString> getRecentFiles() {
		return m_recentFiles;
	}
	void setRecentFiles( std::vector<QString> recentFiles );

	QStringList getRecentFX() {
		return m_recentFX;
	}
	void setMostRecentFX( QString );

	std::vector<QString> getLadspaPath() {
		return m_ladspaPathVect;
	}
	void setLadspaPath( std::vector<QString> pathVect ) {
		m_ladspaPathVect = pathVect;
	}

	QString getLastNews() {
		return m_sLastNews;
	}
	void setLastNews( const QString& sNews ) {
		m_sLastNews = sNews;
	}


	// GUI Properties
	const QString& getQTStyle() {
		return m_sQTStyle;
	}
	void setQTStyle( const QString& sStyle ) {
		m_sQTStyle = sStyle;
	}


	const QString& getApplicationFontFamily() {
		return applicationFontFamily;
	}
	void setApplicationFontFamily( const QString& family ) {
		applicationFontFamily = family;
	}

	int getApplicationFontPointSize() {
		return applicationFontPointSize;
	}
	void setApplicationFontPointSize( int size ) {
		applicationFontPointSize = size;
	}

	QString getMixerFontFamily() {
		return mixerFontFamily;
	}
	void setMixerFontFamily( const QString& family ) {
		mixerFontFamily = family;
	}
	int getMixerFontPointSize() {
		return mixerFontPointSize;
	}
	void setMixerFontPointSize( int size ) {
		mixerFontPointSize = size;
	}
	float getMixerFalloffSpeed() {
		return mixerFalloffSpeed;
	}
	void setMixerFalloffSpeed( float value ) {
		mixerFalloffSpeed = value;
	}
	bool showInstrumentPeaks() {
		return m_bShowInstrumentPeaks;
	}
	void setInstrumentPeaks( bool value ) {
		m_bShowInstrumentPeaks = value;
	}

	int getPatternEditorGridResolution() {
		return m_nPatternEditorGridResolution;
	}
	void setPatternEditorGridResolution( int value ) {
		m_nPatternEditorGridResolution = value;
	}

	bool isPatternEditorUsingTriplets() {
		return m_bPatternEditorUsingTriplets;
	}
	void setPatternEditorUsingTriplets( bool value ) {
		m_bPatternEditorUsingTriplets = value;
	}

	bool isFXTabVisible() {
		return m_bIsFXTabVisible;
	}
	void setFXTabVisible( bool value ) {
		m_bIsFXTabVisible = value;
	}

	unsigned getPatternEditorGridHeight() {
		return m_nPatternEditorGridHeight;
	}
	void setPatternEditorGridHeight( unsigned value ) {
		m_nPatternEditorGridHeight = value;
	}

	unsigned getPatternEditorGridWidth() {
		return m_nPatternEditorGridWidth;
	}
	void setPatternEditorGridWidth( unsigned value ) {
		m_nPatternEditorGridWidth = value;
	}

	WindowProperties getMainFormProperties() {
		return mainFormProperties;
	}
	void setMainFormProperties( const WindowProperties& prop ) {
		mainFormProperties = prop;
	}

	WindowProperties getMixerProperties() {
		return mixerProperties;
	}
	void setMixerProperties( const WindowProperties& prop ) {
		mixerProperties = prop;
	}

	WindowProperties getPatternEditorProperties() {
		return patternEditorProperties;
	}
	void setPatternEditorProperties( const WindowProperties& prop ) {
		patternEditorProperties = prop;
	}

	WindowProperties getSongEditorProperties() {
		return songEditorProperties;
	}
	void setSongEditorProperties( const WindowProperties& prop ) {
		songEditorProperties = prop;
	}


	WindowProperties getAudioEngineInfoProperties() {
		return audioEngineInfoProperties;
	}
	void setAudioEngineInfoProperties( const WindowProperties& prop ) {
		audioEngineInfoProperties = prop;
	}

	WindowProperties getLadspaProperties( unsigned nFX ) {
		return m_ladspaProperties[nFX];
	}
	void setLadspaProperties( unsigned nFX, const WindowProperties& prop ) {
		m_ladspaProperties[nFX] = prop;
	}

	UIStyle* getDefaultUIStyle() {
		return m_pDefaultUIStyle;
	}
	
	bool patternModePlaysSelected() {
		return m_bPatternModePlaysSelected;
	}
	void setPatternModePlaysSelected( bool b ) {
		m_bPatternModePlaysSelected = b;
	}

	MidiMap* get_midi_map();

private:
	QString m_sDataDirectory;


	/** directory of demo songs */
	QString demoPath;

	//___ General properties ___
	bool m_bPatternModePlaysSelected; /// Behaviour of Pattern Mode
	bool restoreLastSong;		///< Restore last song?
	bool m_bUseLash;
	bool m_bShowDevelWarning;	///< Show development version warning?
	QString lastSongFilename;	///< Last song used
	bool hearNewNotes;
	std::vector<QString> m_recentFiles;
	QStringList m_recentFX;
	std::vector<QString> m_ladspaPathVect;
	bool quantizeEvents;
	bool recordEvents;
	QString m_sLastNews;
	MidiMap* m_midi_map;

	//___ GUI properties ___
	QString m_sQTStyle;

	QString applicationFontFamily;
	int applicationFontPointSize;
	QString mixerFontFamily;
	int mixerFontPointSize;
	float mixerFalloffSpeed;
	int m_nPatternEditorGridResolution;
	bool m_bPatternEditorUsingTriplets;
	bool m_bShowInstrumentPeaks;
	bool m_bIsFXTabVisible;
	unsigned m_nPatternEditorGridHeight;
	unsigned m_nPatternEditorGridWidth;
	WindowProperties mainFormProperties;
	WindowProperties mixerProperties;
	WindowProperties patternEditorProperties;
	WindowProperties songEditorProperties;
	WindowProperties drumkitManagerProperties;
	WindowProperties audioEngineInfoProperties;
	WindowProperties m_ladspaProperties[MAX_FX];

	UIStyle*  m_pDefaultUIStyle;

	/// Create preferences directory
	void createPreferencesDirectory();

	/// Create data directory
	void createDataDirectory();

	/// Create soundLibrary directory
	void createSoundLibraryDirectories();

	WindowProperties readWindowProperties( const QDomNode& parent, const QString& windowName, WindowProperties defaultProp );
	void writeWindowProperties( QDomNode& parent, const QString& windowName, const WindowProperties& prop );

	void writeUIStyle( QDomNode& parent );
	void readUIStyle( const QDomNode& parent );
};

} // namespace Tritium

#endif // TRITIUM_PREFERENCES_HPP

