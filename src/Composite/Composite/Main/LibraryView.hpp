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
#ifndef COMPOSITE_MAIN_LIBRARYVIEW_HPP
#define COMPOSITE_MAIN_LIBRARYVIEW_HPP

#include <QtGui/QTreeView>
#include <Tritium/memory.hpp>

class QPaintEvent;

namespace Composite
{
namespace Main
{
    /**
     * \brief Widget for accessing resources
     */
    class LibraryView : public QTreeView
    {
	Q_OBJECT

    public:
	LibraryView(QWidget *parent = 0);
	virtual ~LibraryView();

    };

} // namespace Main
} // namespace Composite

#endif // COMPOSITE_MAIN_LIBRARYVIEW_HPP
