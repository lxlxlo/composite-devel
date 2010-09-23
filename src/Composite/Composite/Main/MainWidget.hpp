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
#ifndef COMPOSITE_MAIN_MAINWIDGET_HPP
#define COMPOSITE_MAIN_MAINWIDGET_HPP

#include <QtGui/QWidget>
#include <Tritium/memory.hpp>

class QAction;
class QToolButton;
class QToolBar;

namespace Composite
{
namespace Main
{
    /**
     * \brief The central workspace for Composite
     */
    class MainWidget : public QWidget
    {
	Q_OBJECT
    public:
	MainWidget(QWidget *parent = 0);
	virtual ~MainWidget();

    public slots:
	void go_matrix();
	void go_edit();

	void x_play();
	void x_stop();

    private:
	void _setup_actions();
	void _setup_widgets();
	void _layout_widgets();
	void _setup_signals_and_slots();

    private:
	struct actions_t {
	    // Go to...
	    QAction *go_matrix;
	    QAction *go_edit;

	    // Transport
	    QAction *x_play;
	    QAction *x_stop;

	    QAction *_end; // should be null
	} _act;

	struct tool_buttons_t {
	    // Go to...
	    QToolButton *go_matrix;
	    QToolButton *go_edit;

	    // Transport
	    QToolButton *x_play;
	    QToolButton *x_stop;

	    QToolButton *_end; // should be null
	} _tbtn;

	struct tool_bar_t {
	    QToolBar *main;
	} _tbar;

    };

} // namespace Main
} // namespace Composite

#endif // COMPOSITE_MAIN_MAINWIDGET_HPP
