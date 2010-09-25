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
#ifndef COMPOSITE_WIDGETS_TOOLBAR_HPP
#define COMPOSITE_WIDGETS_TOOLBAR_HPP

#include <QtGui/QWidget>

namespace Composite
{
namespace Widgets
{
    class ToolbarPrivate;

    /**
     * \brief Manages a linear grouping of widgets
     *
     * This class is designed and intended to be used for the main
     * navigation and transport controls of the Main GUI.
     */
    class Toolbar : public QWidget
    {
	Q_OBJECT
    private:
	ToolbarPrivate * const _d;

    public:
	Toolbar(QWidget *parent = 0);
	virtual ~Toolbar();

	typedef enum {
	    HORIZONTAL=0,
	    VERTICAL=1
	} orientation_t;

	orientation_t orientation();
	void orientation(orientation_t o);

	void addWidget(QWidget *w);
	void addStretch();

    protected:
	virtual void paintEvent(QPaintEvent *event);

    };

} // namespace Widgets
} // namespace Composite

#endif // COMPOSITE_WIDGETS_TOOLBAR_HPP
