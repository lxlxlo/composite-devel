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


#ifndef SAMPLER_H
#define SAMPLER_H

#include <hydrogen/Object.h>
#include <hydrogen/globals.h>

#include <inttypes.h>
#include <vector>



namespace H2Core
{

class Note;
class Song;
class Sample;
class Instrument;
class AudioOutput;

///
/// Waveform based sampler.
///
class Sampler : public Object
{
public:
	float *__main_out_L;	///< sampler main out (left channel)
	float *__main_out_R;	///< sampler main out (right channel)

	Sampler();
	~Sampler();

	void process( uint32_t nFrames, Song* pSong );

	/// Start playing a note
	void note_on( Note *note );

	/// Stop playing a note.
	void note_off( Note *note );

	void stop_playing_notes( Instrument *instr = NULL );

	int get_playing_notes_number() {
		return __playing_notes_queue.size();
	}

	void preview_sample( Sample* sample );
	void preview_instrument( Instrument* instr );

	void set_audio_output( AudioOutput* audio_output );
	void makeTrackOutputQueues();


private:
	std::vector<Note*> __playing_notes_queue;
	static Sampler* __instance;
	AudioOutput* __audio_output;

	/// Instrument used for the preview feature.
	Instrument* __preview_instrument;

#ifdef JACK_SUPPORT
	float* __track_out_L[ MAX_INSTRUMENTS ];
	float* __track_out_R[ MAX_INSTRUMENTS ];
#endif

	unsigned __render_note( Note* pNote, unsigned nBufferSize, Song* pSong );

	int __render_note_no_resample(
	    Sample *pSample,
	    Note *pNote,
	    int nBufferSize,
	    int nInitialSilence,
	    float cost_L,
	    float cost_R,
	    float cost_track_L,
	    float cost_track_R,
	    float fSendFXLevel_L,
	    float fSendFXLevel_R,
	    Song* pSong
	);

	int __render_note_resample(
	    Sample *pSample,
	    Note *pNote,
	    int nBufferSize,
	    int nInitialSilence,
	    float cost_L,
	    float cost_R,
	    float cost_track_L,
	    float cost_track_R,
	    float fLayerPitch,
	    float fSendFXLevel_L,
	    float fSendFXLevel_R,
	    Song* pSong
	);
};

} // namespace

#endif

