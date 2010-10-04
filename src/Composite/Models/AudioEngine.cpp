/*
 * Copyright(c) 2010 by Gabriel M. Beddingfield <gabriel@teuton.org>
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

#include <Composite/Models/AudioEngine.hpp>

#include <Tritium/Engine.hpp>
#include <Tritium/Instrument.hpp>
#include <Tritium/InstrumentList.hpp>
#include <Tritium/Note.hpp>
#include <Tritium/Sampler.hpp>

#include "AudioEnginePrivate.hpp"

#include <iostream>
using namespace std;

using namespace Tritium;

namespace Composite
{
namespace Models
{

    AudioEngine::AudioEngine(Tritium::Engine *eng, QObject *parent) :
	QAbstractItemModel(parent),
	_d( new AudioEnginePrivate(this) )
    {
	_d->_engine = eng;
    }

    AudioEngine::~AudioEngine()
    {
    }

    int AudioEngine::columnCount( const QModelIndex & parent ) const
    {
	return 4;
    }

    int AudioEngine::rowCount( const QModelIndex & parent ) const
    {
	return 6;
    }

    QVariant AudioEngine::data( const QModelIndex & index,
			       int role ) const
    {
	return QVariant();
    }

    QModelIndex AudioEngine::index( int row,
				    int column,
				    const QModelIndex & parent ) const
    {
	QModelIndex ix = createIndex(row, column, 0);
	return ix;
    }

    QModelIndex AudioEngine::parent( const QModelIndex & index ) const
    {
	return QModelIndex();
    }

    void AudioEngine::trigger( const QModelIndex& target, float velocity )
    {
	T<Sampler>::shared_ptr samp = _d->_engine->get_sampler();
	T<InstrumentList>::shared_ptr insts = samp->get_instrument_list();
	T<Instrument>::shared_ptr sound;

	int index = 4*target.row() + target.column();
	if( index < insts->get_size() ) {
	    sound = insts->get( index );
	    Note *n = new Note( sound, velocity );
	    _d->_engine->midi_noteOn(n);
	}
    }

    void AudioEngine::release( const QModelIndex& target )
    {
	T<Sampler>::shared_ptr samp = _d->_engine->get_sampler();
	T<InstrumentList>::shared_ptr insts = samp->get_instrument_list();
	T<Instrument>::shared_ptr sound;

	int index = 4*target.row() + target.column();
	if( index < insts->get_size() ) {
	    sound = insts->get( index );
	    Note *n = new Note( sound, 0.0f );
	    _d->_engine->midi_noteOff(n);
	}
    }

    ////////////////////////////////////////////////////////////////////
    // AudioEnginePrivate
    ////////////////////////////////////////////////////////////////////

    AudioEnginePrivate::AudioEnginePrivate(AudioEngine *p) :
	_parent(p)
    {
    }

    AudioEnginePrivate::~AudioEnginePrivate()
    {
    }


} // namespace Models
} // namespace Composite

