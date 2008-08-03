
macx-g++ {
	LIBS += -framework AudioUnit -framework AudioToolbox \
		-framework CoreServices -framework CoreAudio -framework CoreMidi

	H2DEFINES += LADSPA_SUPPORT
	H2DEFINES += FLAC_SUPPORT
#	H2DEFINES += JACK_SUPPORT
	H2DEFINES += COREAUDIO_SUPPORT
	H2DEFINES += COREMIDI_SUPPORT
}


linux-g++ {
	H2DEFINES += ALSA_SUPPORT
	H2DEFINES += JACK_SUPPORT
	H2DEFINES += LASH_SUPPORT
	H2DEFINES += FLAC_SUPPORT
	H2DEFINES += LADSPA_SUPPORT
	H2DEFINES += LRDF_SUPPORT
	H2DEFINES += OSS_SUPPORT
	system("pkg-config --atleast-version=0.102.0 jack") {
	  H2DEFINES += JACK_MIDI_SUPPORT
	  system("pkg-config --max-version=0.102.26 jack") {
	    H2DEFINES += JACK_MIDI_0_102_0
	  } else {
	    system("pkg-config --max-version=0.104.0 jack") {
	      H2DEFINES += JACK_MIDI_0_102_27
	    } else {
		system("pkg-config --atleast-version=0.105.0 jack") {
		H2DEFINES += JACK_MIDI_0_105_0
	      } else {
		error("Qmake/pkg-config is confused about the jack version")
	      }
	    }
	  }
	} else {
	  message("No MIDI support for JACK (requires 0.102.0 or greater)")
	}
}

linux-g++-64 {
	H2DEFINES += ALSA_SUPPORT
	H2DEFINES += JACK_SUPPORT
	H2DEFINES += LASH_SUPPORT
	H2DEFINES += FLAC_SUPPORT
	H2DEFINES += LADSPA_SUPPORT
	H2DEFINES += LRDF_SUPPORT
	H2DEFINES += OSS_SUPPORT
	system("pkg-config --atleast-version=0.102.0 jack") {
	  H2DEFINES += JACK_MIDI_SUPPORT
	  system("pkg-config --max-version=0.102.26 jack") {
	    H2DEFINES += JACK_MIDI_0_102_0
	  } else {
	    system("pkg-config --max-version=0.104.0 jack") {
	      H2DEFINES += JACK_MIDI_0_102_27
	    } else {
		system("pkg-config --atleast-version=0.105.0 jack") {
		H2DEFINES += JACK_MIDI_0_105_0
	      } else {
		error("Qmake/pkg-config is confused about the jack version")
	      }
	    }
	  }
	} else {
	  message("No MIDI support for JACK (requires 0.102.0 or greater)")
	}
}

win32 {
	H2DEFINES += FLAC_SUPPORT
	H2DEFINES += LADSPA_SUPPORT
	H2DEFINES += PORTAUDIO_SUPPORT
	H2DEFINES += PORTMIDI_SUPPORT
}


