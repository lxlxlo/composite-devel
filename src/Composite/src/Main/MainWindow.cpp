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
     * \brief Go to the matrix view
     */
    void MainWindow::go_matrix()
    {
	QMessageBox::information(this, "go_matrix()", "go_matrix()");
    }

    /**
     * \brief Go to the edit view
     */
    void MainWindow::go_edit()
    {
	QMessageBox::information(this, "go_edit()", "go_edit()");
    }

    /**
     * \brief Transport: start playing
     */
    void MainWindow::x_play()
    {
	QMessageBox::information(this, "x_play()", "x_play()");
    }

    /**
     * \brief Transport: stop playing
     */
    void MainWindow::x_stop()
    {
	QMessageBox::information(this, "x_stop()", "x_stop()");
    }

    /**
     * \brief Create and connect generic actions
     */
    void MainWindow::_setup_actions()
    {
	_act.go_matrix = new QAction( "M", this );
	_act.go_matrix->setToolTip( tr("Go to matrix view") );
	addAction( _act.go_matrix );
	connect( _act.go_matrix, SIGNAL(triggered()),
		 this, SLOT(go_matrix()) );

	_act.go_edit = new QAction( "E", this );
	_act.go_edit->setToolTip( tr("Go to edit view") );
	addAction( _act.go_edit );
	connect( _act.go_edit, SIGNAL(triggered()),
		 this, SLOT(go_edit()) );

	_act.x_play = new QAction( "P", this );
	_act.x_play->setToolTip( tr("Start playing") );
	addAction( _act.x_play );
	connect( _act.x_play, SIGNAL(triggered()),
		 this, SLOT(x_play()) );

	_act.x_stop = new QAction( "S", this );
	_act.x_stop->setToolTip( tr("Stop playing") );
	addAction( _act.x_stop );
	connect( _act.x_stop, SIGNAL(triggered()),
		 this, SLOT(x_stop()) );

	_act._end = 0;
    }

    void MainWindow::_setup_widgets()
    {
#if 0
	_tbtn.go_matrix = new QToolButton(this);
	_tbtn.go_matrix->setDefaultAction(_act.go_matrix);

	_tbtn.go_edit = new QToolButton(this);
	_tbtn.go_edit->setDefaultAction(_act.go_edit);

	_tbtn.x_play = new QToolButton(this);
	_tbtn.x_play->setDefaultAction(_act.x_play);

	_tbtn.x_stop = new QToolButton(this);
	_tbtn.x_stop->setDefaultAction(_act.x_stop);
#else
	_tbtn.go_matrix = 0;
	_tbtn.go_edit = 0;
	_tbtn.x_play = 0;
	_tbtn.x_stop = 0;
#endif
	_tbtn._end = 0;

    }

    void MainWindow::_layout_widgets()
    {
	_tbar.main = new QToolBar(this);

	_tbar.main->addAction( _act.go_matrix );
	_tbar.main->addAction( _act.go_edit );
	_tbar.main->addSeparator();
	_tbar.main->addAction( _act.x_play );
	_tbar.main->addAction( _act.x_stop );

	addToolBar(_tbar.main);
    }

    void MainWindow::_setup_signals_and_slots()
    {
    }

} // namespace Main
} // namespace Composite

