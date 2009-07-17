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
#include <hydrogen/SeqScriptIterator.h>

#include <inttypes.h>
#include <vector>



namespace H2Core
{

class Note;
class Sample;
class Instrument;
class AudioOutput;

struct SamplerPrivate;
struct TransportPosition;

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

	void process( SeqScriptConstIterator beg,
		      SeqScriptConstIterator end,
		      const TransportPosition& pos,
		      uint32_t nFrames );

	void stop_playing_notes( Instrument *instr = NULL );
	void panic();

	int get_playing_notes_number();

	void preview_sample( Sample* sample, int length );
	void preview_instrument( Instrument* instr );

	void makeTrackOutputQueues();

private:
	SamplerPrivate *d;
}; // class Sampler

} // namespace H2Core

#endif

