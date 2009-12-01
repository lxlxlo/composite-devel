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
#ifndef TRITIUM_SAMPLERPRIVATE_HPP
#define TRITIUM_SAMPLERPRIVATE_HPP

#include <Tritium/Sampler.hpp>
#include <Tritium/Note.hpp>

namespace Tritium
{
    class Instrument;

    struct SamplerPrivate
    {
	Sampler& parent;
	typedef std::list<Note> NoteList;
	NoteList current_notes;                // Replaces __playing_notes_queue
	Instrument* preview_instrument;         // Replaces __preview_instrument
#ifdef JACK_SUPPORT
	float* track_out_L[ MAX_INSTRUMENTS ];  // Replaces __track_out_L
	float* track_out_R[ MAX_INSTRUMENTS ];  // Replaces __track_out_R
#endif
	SamplerPrivate(Sampler* par) :
	    parent( *par ),
	    preview_instrument( 0 )
	    {}

	// Add/Remove notes from current_notes based on event 'ev'
	void handle_event(const SeqEvent& ev);

	// These are utils for handle_event().
	void panic();  // Cease all sounc
	void handle_note_on(const SeqEvent& ev);
	void handle_note_off(const SeqEvent& ev);

	// Actually render the specific note(s) to the buffers.
	int render_note(Note& note, uint32_t nFrames, uint32_t frame_rate);
	int render_note_no_resample(
	    Sample *pSample,
	    Note& note,
	    int nFrames,
	    float cost_L,
	    float cost_R,
	    float cost_track_L,
	    float cost_track_R,
	    float fSendFXLevel_L,
	    float fSendFXLevel_R
	    );
	int render_note_resample(
	    Sample *pSample,
	    Note& note,
	    int nFrames,
	    uint32_t frame_rate,
	    float cost_L,
	    float cost_R,
	    float cost_track_L,
	    float cost_track_R,
	    float fLayerPitch,
	    float fSendFXLevel_L,
	    float fSendFXLevel_R
	    );

    }; // class SamplerPrivate

} // namespace Tritium

#endif // TRITIUM_SAMPLERPRIVATE_HPP
