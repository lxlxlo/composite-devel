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

#include "AudioEnginePrivate.hpp"

#include <iostream>
using namespace std;

namespace Composite
{
namespace Models
{

    AudioEngine::AudioEngine(int argc, char* argv[], QObject *parent) :
	QAbstractItemModel(parent),
	_d( new AudioEnginePrivate(this) )
    {
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
	cout << "trigger (" << target.row() << "," << target.column() << ")" << endl;
    }

    void AudioEngine::release( const QModelIndex& target )
    {
	cout << "release (" << target.row() << "," << target.column() << ")" << endl;
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

