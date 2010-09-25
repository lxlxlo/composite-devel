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

#include <Composite/Main/LibraryView.hpp>

#include <QtGui/QPainter>

#include <stdexcept>

namespace Composite
{
namespace Main
{

    LibraryView::LibraryView(QWidget *parent) :
	QWidget(parent)
    {
    }

    LibraryView::~LibraryView()
    {
    }

    void LibraryView::paintEvent(QPaintEvent *)
    {
	QPainter painter(this);

	QBrush br( QColor(0xff, 0xff, 0xff) );
	QPen pen( QColor(0, 0, 0) );
	pen.setWidth( 1 );

	painter.setBrush( br );
	painter.setPen( pen );

	painter.drawRect( 0, 0, width()-1, height()-1 );
    }

} // namespace Main
} // namespace Composite

