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

#include <Composite/Looks/Sizes.hpp>

#include <QtCore/QString>
#include <QtGui/QIcon>

class QAction;
class QToolButton;
class QToolBar;
class QWidget;

namespace Composite
{

namespace Widgets
{
    class Toolbar;
}

namespace Main
{
    class MatrixView;

    class MainWidgetPrivate
    {
    public:
	MainWidgetPrivate(MainWidget *parent) :
	    _p(parent),
	    _central_widget(0)
	    {}

    public:
	MainWidget * const _p;
	Composite::Looks::Sizes _sizes;
	QWidget *_central_widget;
	QWidget *_library_widget;
	QWidget *_bot_left_widget;

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
	    Widgets::Toolbar *left;
	    Widgets::Toolbar *bottom;
	} _tbar;

	struct icon_t {
	    QIcon go_matrix;
	    QIcon go_edit;

	    QIcon x_play;
	    QIcon x_stop;
	} _ico;

	struct mode_t {
	    MatrixView *matrix;
	    QWidget *edit;
	} _mode;

    public:
	void load_icons();
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

