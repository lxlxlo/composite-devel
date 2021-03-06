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
#ifndef COMPOSITE_MODELS_AUDIOENGINE_HPP
#define COMPOSITE_MODELS_AUDIOENGINE_HPP

#include <QtCore/QAbstractItemModel>
#include <QtCore/QVariant>
#include <Tritium/memory.hpp>

class QPaintEvent;
class QString;
class QUrl;

namespace Tritium
{
    class Engine;
}

namespace Composite
{
namespace Models
{
    class AudioEnginePrivate;

    /**
     * \brief The main access to the Tritium Engine.
     */
    class AudioEngine : public QAbstractItemModel
    {
	Q_OBJECT
    private:
	AudioEnginePrivate * const _d;

    public:
	AudioEngine( Tritium::Engine *engine = 0, QObject *parent = 0);
	virtual ~AudioEngine();

	// Reimplementing QAbstractItemModel methods
	virtual int columnCount( const QModelIndex & parent = QModelIndex() ) const;
	virtual int rowCount( const QModelIndex & parent = QModelIndex() ) const;

	virtual QVariant data( const QModelIndex & index,
			       int role = Qt::DisplayRole ) const;
	virtual QModelIndex index( int row,
				    int column,
				    const QModelIndex & parent = QModelIndex() ) const ;
	virtual QModelIndex parent( const QModelIndex & index ) const;

    public slots:
	void trigger( const QModelIndex& target, float velocity );
	void release( const QModelIndex& target );
    };

} // namespace Models
} // namespace Composite

#endif // COMPOSITE_MODELS_AUDIOENGINE_HPP
