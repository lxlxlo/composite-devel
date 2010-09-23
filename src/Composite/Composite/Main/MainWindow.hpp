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
#ifndef COMPOSITE_MAIN_MAINWINDOW_HPP
#define COMPOSITE_MAIN_MAINWINDOW_HPP

#include <QtGui/QMainWindow>
#include <Tritium/memory.hpp>

class QAction;
class QToolBar;
class QToolButton;

namespace Composite
{
namespace Main
{

    class MainWindow : public QMainWindow
    {
	Q_OBJECT
    public:
	MainWindow(QWidget *parent = 0);
	virtual ~MainWindow();

    private:
	void _setup_actions();
	void _setup_widgets();
	void _layout_widgets();
	void _setup_signals_and_slots();
    };

} // namespace Main
} // namespace Composite

#endif // COMPOSITE_MAIN_MAINWINDOW_HPP
