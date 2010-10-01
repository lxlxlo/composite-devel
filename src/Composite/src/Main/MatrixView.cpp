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

#include <QtGui/QGridLayout>
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
	return QModelIndex();
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


} // namespace Main
} // namespace Composite

