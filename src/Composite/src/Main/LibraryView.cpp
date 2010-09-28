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

#include <QtGui/QFileSystemModel>

#include <stdexcept>

namespace Composite
{
namespace Main
{

    LibraryView::LibraryView(QWidget *parent) :
	QTreeView(parent)
    {
	QFileSystemModel *mod = new QFileSystemModel(this);
	mod->setRootPath("/");
	setModel(mod);
	for( int k = 1; k < mod->columnCount() ; ++k ) {
	    setColumnHidden(k, true);
	}
	setDragDropMode(QAbstractItemView::DragOnly);
    }

    LibraryView::~LibraryView()
    {
    }

} // namespace Main
} // namespace Composite

