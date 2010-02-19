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
#include <Tritium/AudioPort.hpp>
#include <Tritium/AudioPortManager.hpp>
#include <Tritium/Note.hpp>
#include <Tritium/memory.hpp>
#include <Tritium/InstrumentList.hpp>
#include <QMutex>
#include <cassert>

namespace Tritium
{
    class Instrument;

    struct SamplerPrivate
    {
	Sampler& parent;
	Engine* engine;
	typedef std::list<Note> NoteList;
	QMutex mutex_current_notes;            // Must be locked when adding/removing elements
	NoteList current_notes;
	T<InstrumentList>::shared_ptr instrument_list;
	T<Instrument>::shared_ptr preview_instrument;         // Replaces __preview_instrument
	T<AudioPortManager>::shared_ptr port_manager;
	std::deque< T<AudioPort>::shared_ptr > instrument_ports;

	// Configuration
	int max_notes; // Maximum number of notes played at any one time
	bool per_instrument_outs; // Enable an output for each instrument.
	bool instrument_outs_prefader;

	SamplerPrivate(Sampler* par, Engine* e_par, T<AudioPortManager>::shared_ptr apm) :
	    parent( *par ),
	    engine(e_par),
	    port_manager(apm),
	    instrument_list( new InstrumentList ),
	    preview_instrument(),
	    max_notes(-1),
	    per_instrument_outs(false),
	    instrument_outs_prefader(false)
	    {
		assert(e_par);
	    }

	~SamplerPrivate() {
	    parent.clear();
	}

	// Add/Remove notes from current_notes based on event 'ev'
	void handle_event(const SeqEvent& ev);

	// These are utils for handle_event().
	void panic();  // Cease all sounc
	void handle_note_on(const SeqEvent& ev);
	void handle_note_off(const SeqEvent& ev);

	// These are primarily for preview instrument.
	void note_on(Note& note);
	void note_off(Note& note);

	// Actually render the specific note(s) to the buffers.
	int render_note(Note& note, uint32_t nFrames, uint32_t frame_rate);
	int render_note_no_resample(
	    T<Sample>::shared_ptr pSample,
	    Note& note,
	    int nFrames,
	    float cost_L,
	    float cost_R
	    );
	int render_note_resample(
	    T<Sample>::shared_ptr pSample,
	    Note& note,
	    int nFrames,
	    uint32_t frame_rate,
	    float cost_L,
	    float cost_R,
	    float fLayerPitch
	    );

    }; // class SamplerPrivate

} // namespace Tritium

#endif // TRITIUM_SAMPLERPRIVATE_HPP
