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

#include <Composite/Main/MainWindow.hpp>
#include <Composite/Main/MainWidget.hpp>

#include <QtGui/QAction>
#include <QtGui/QHBoxLayout>
#include <QtGui/QMessageBox>
#include <QtGui/QToolBar>
#include <QtGui/QToolButton>

namespace Composite
{
namespace Main
{

    MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent)
    {
	_setup_actions();
	_setup_widgets();
	_layout_widgets();
	_setup_signals_and_slots();
	show();
    }

    MainWindow::~MainWindow()
    {
    }

    /**
     * \brief Create and connect generic actions
     */
    void MainWindow::_setup_actions()
    {
    }

    /**
     * \brief Create (but don't connect) various widgets
     */
    void MainWindow::_setup_widgets()
    {
	MainWidget *mw;
	mw = new Composite::Main::MainWidget(this);
	setCentralWidget(mw);
    }

    void MainWindow::_layout_widgets()
    {
    }

    void MainWindow::_setup_signals_and_slots()
    {
    }

} // namespace Main
} // namespace Composite

