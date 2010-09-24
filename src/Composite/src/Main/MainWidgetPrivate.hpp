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

#include <QtCore/QString>
#include <Composite/Looks/Sizes.hpp>

class QAction;
class QToolButton;
class QToolBar;
class QWidget;

namespace Composite
{
namespace Main
{

    class MainWidgetPrivate
    {
    public:
	MainWidgetPrivate(MainWidget *parent) :
	    _p(parent)
	    {}

    public:
	MainWidget * const _p;
	Composite::Looks::Sizes _sizes;

	struct actions_t {
	    // Go to...
	    QAction *go_matrix;
	    QAction *go_edit;

	    // Transport
	    QAction *x_play;
	    QAction *x_stop;
	} _act;

	struct tool_buttons_t {
	    // Go to...
	    QToolButton *go_matrix;
	    QToolButton *go_edit;

	    // Transport
	    QToolButton *x_play;
	    QToolButton *x_stop;
	} _tbtn;

	struct tool_bar_t {
	    QToolBar *main;
	} _tbar;

    public:
	void setup_actions();
	void setup_widgets();
	void layout_widgets();
	void setup_signals_and_slots();

    public:
	QString tr(const char * sourceText, const char * disambiguation = 0, int n = -1 ) {
	    return QObject::tr(sourceText, disambiguation, n);
	}
    };

} // namespace Main
} // namespace Composite

