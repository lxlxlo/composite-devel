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

#include <Composite/Widgets/Toolbar.hpp>
#include <deque>

#include <QtGui/QWidget>

namespace Composite
{
namespace Widgets
{

    class ToolbarPrivate
    {
    public:
	ToolbarPrivate() :
	    orientation( Toolbar::HORIZONTAL )
	    {
	    }

    public:
	typedef std::deque<QWidget*> seq_t;

	Toolbar::orientation_t orientation;

	/* A null pointer indicates "stretch":
	 */
	seq_t widgets;
    };

} // namespace Widgets
} // namespace Composite

