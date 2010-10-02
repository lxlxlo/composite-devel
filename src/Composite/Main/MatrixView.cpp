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

#include <Composite/Main/MatrixView.hpp>
#include <Composite/Models/AudioEngine.hpp>

#include <QtGui/QGridLayout>
#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>
#include <QtGui/QPushButton>

#include <stdexcept>

namespace Composite
{
namespace Main
{

    MatrixView::MatrixView(QWidget *parent) :
	QAbstractItemView(parent)
    {
    }

    MatrixView::~MatrixView()
    {
    }

    QModelIndex MatrixView::indexAt(const QPoint& point) const
    {
	QAbstractItemModel *mod = model();
	float x, y;
	x = float(point.x()) / float(width());
	y = float(point.y()) / float(height());

	int col, row;
	col = x * mod->columnCount();
	row = y * mod->rowCount();

	return mod->index(row, col);
    }

    void MatrixView::scrollTo(const QModelIndex& index, ScrollHint hint )
    {
    }

    QRect MatrixView::visualRect(const QModelIndex& index) const
    {
	return QRect();
    }

    int MatrixView::horizontalOffset() const
    {
	return 0;
    }

    int MatrixView::verticalOffset() const
    {
	return 0;
    }

    bool MatrixView::isIndexHidden(const QModelIndex& index) const
    {
	return false;
    }

    QModelIndex MatrixView::moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers)
    {
	return QModelIndex();
    }

    void MatrixView::setSelection(const QRect& rect, QItemSelectionModel::SelectionFlags flags)
    {
    }

    QRegion MatrixView::visualRegionForSelection(const QItemSelection& selection) const
    {
	return QRegion();
    }

    void MatrixView::mousePressEvent(QMouseEvent *ev)
    {
	QModelIndex ix = indexAt( ev->pos() );
	Composite::Models::AudioEngine *mod = dynamic_cast<Composite::Models::AudioEngine*>( model() );
	if( !mod ) {
	    ev->ignore();
	    return;
	}

	mod->trigger(ix, 1.0f);
	ev->accept();
    }

    void MatrixView::mouseReleaseEvent(QMouseEvent *ev)
    {
	QModelIndex ix = indexAt( ev->pos() );
	Composite::Models::AudioEngine *mod = dynamic_cast<Composite::Models::AudioEngine*>( model() );
	if( !mod ) {
	    ev->ignore();
	    return;
	}

	mod->release(ix);
	ev->accept();
    }

    void MatrixView::paintEvent(QPaintEvent * /*event*/ )
    {
	QAbstractItemModel *mod = model();
	QPalette pal = palette();
	QPainter painter( viewport() );

	pal.setCurrentColorGroup( QPalette::Active );

	painter.setBrush( pal.window() );
	painter.drawRect( 0, 0, width(), height() );

	if( !mod )
	    return;

	int cols = mod->columnCount();
	int rows = mod->rowCount();

	float col_wid = float(width()) / cols;
	float row_ht = float(height()) / rows;

	// Draw a grid
	painter.setBrush( pal.mid() );
	painter.setPen( pal.color(QPalette::Mid) );
	int x, y;
	y = 0;
	for( int k = 0 ; k <= cols ; ++k ) {
	    x = ( col_wid * k ) + 0.5f;
	    painter.drawLine( x, y, x, height() );
	}
	x = 0;
	for( int k = 0 ; k <= rows ; ++k ) {
	    y = ( row_ht * k ) + 0.5f;
	    painter.drawLine( x, y, width(), y );
	}

    }

} // namespace Main
} // namespace Composite

