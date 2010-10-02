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
#ifndef COMPOSITE_MAIN_MATRIXVIEW_HPP
#define COMPOSITE_MAIN_MATRIXVIEW_HPP

#include <QtGui/QAbstractItemView>
#include <Tritium/memory.hpp>

class QPaintEvent;

namespace Composite
{
namespace Main
{
    /**
     * \brief A central workspace with a matrix-ey feel.
     */
    class MatrixView : public QAbstractItemView
    {
	Q_OBJECT

    public:
	MatrixView(QWidget *parent = 0);
	virtual ~MatrixView();

	// QAbstractItemView required methods:
	virtual QModelIndex indexAt( const QPoint& point ) const;
	virtual void scrollTo( const QModelIndex & index,
			       ScrollHint hint = EnsureVisible );
	virtual QRect visualRect( const QModelIndex & index ) const;

    protected:
	// QAbstractItemView required methods:
	virtual int horizontalOffset() const;
	virtual int verticalOffset() const;
	virtual bool isIndexHidden( const QModelIndex& index ) const;
	virtual QModelIndex moveCursor( CursorAction cursorAction,
					Qt::KeyboardModifiers modifiers );
	virtual void setSelection ( const QRect & rect,
				    QItemSelectionModel::SelectionFlags flags );
	virtual QRegion visualRegionForSelection ( const QItemSelection & selection ) const;

	virtual void paintEvent(QPaintEvent *event);

    protected:
	virtual void mousePressEvent(QMouseEvent *event);
	virtual void mouseReleaseEvent(QMouseEvent *event);

    };

} // namespace Main
} // namespace Composite

#endif // COMPOSITE_MAIN_MATRIXVIEW_HPP
