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


#ifndef TRITIUM_SAMPLER_HPP
#define TRITIUM_SAMPLER_HPP

#include <Tritium/globals.hpp>
#include <Tritium/SeqScriptIterator.hpp>

#include <inttypes.h>
#include <vector>



namespace Tritium
{

class Note;
class Sample;
class Instrument;
class InstrumentList;
class AudioOutput;
class Engine;

struct SamplerPrivate;
struct TransportPosition;

///
/// Waveform based sampler.
///
    class Sampler
    {
    public:
	float *__main_out_L;	///< sampler main out (left channel)
	float *__main_out_R;	///< sampler main out (right channel)

	Sampler(Engine* parent);
	~Sampler();

	void process( SeqScriptConstIterator beg,
		      SeqScriptConstIterator end,
		      const TransportPosition& pos,
		      uint32_t nFrames );

	void stop_playing_notes( T<Instrument>::shared_ptr instr = T<Instrument>::shared_ptr() );
	void panic();

	int get_playing_notes_number();

	void preview_sample( T<Sample>::shared_ptr sample, int length );
	void preview_instrument( T<Instrument>::shared_ptr instr );

	void makeTrackOutputQueues();

	void add_instrument( T<Instrument>::shared_ptr instr );
	T<InstrumentList>::shared_ptr get_instrument_list();

    private:
	SamplerPrivate *d;
    }; // class Sampler

} // namespace Tritium

#endif // TRITIUM_SAMPLER_HPP
