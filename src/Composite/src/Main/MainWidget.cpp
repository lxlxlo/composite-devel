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

#include <Composite/Main/MainWidget.hpp>
#include "MainWidgetPrivate.hpp"

#include <QtGui/QAction>
#include <QtGui/QHBoxLayout>
#include <QtGui/QMessageBox>
#include <QtGui/QPainter>
#include <QtGui/QToolBar>
#include <QtGui/QToolButton>

#include <stdexcept>

namespace Composite
{
namespace Main
{

    MainWidget::MainWidget(int /*argc*/, char* /*argv*/ [], QWidget *parent) :
        QWidget(parent),
        _d( new MainWidgetPrivate(this) )
    {
        if( _d == 0 ) {
            throw std::runtime_error("Could not allocate MainWidgetPrivate");
        }

        _d->setup_actions();
        _d->setup_widgets();
        _d->layout_widgets();
        _d->setup_signals_and_slots();

        show();
    }

    MainWidget::~MainWidget()
    {
        delete _d;
    }

    /**
     * \brief Go to the matrix view
     */
    void MainWidget::go_matrix()
    {
        QMessageBox::information(this, "go_matrix()", "go_matrix()");
    }

    /**
     * \brief Go to the edit view
     */
    void MainWidget::go_edit()
    {
        QMessageBox::information(this, "go_edit()", "go_edit()");
    }

    /**
     * \brief Transport: start playing
     */
    void MainWidget::x_play()
    {
        QMessageBox::information(this, "x_play()", "x_play()");
    }

    /**
     * \brief Transport: stop playing
     */
    void MainWidget::x_stop()
    {
        QMessageBox::information(this, "x_stop()", "x_stop()");
    }

    void MainWidget::paintEvent(QPaintEvent *event)
    {
	QPainter painter(this);

	QColor bg(0, 0, 0x70);
	QColor side(0xEE, 0, 0);

	float px = _d->_sizes.minimum_button().pixels();

	QBrush bg_brush( bg );
	QPen bg_pen( bg );

	QBrush s_brush( side );
	QPen s_pen( side );

	painter.setBrush( bg_brush );
	painter.setPen( bg_pen );
	painter.drawRect( 0, 0, width(), height() );

	painter.setBrush( s_brush );
	painter.setPen( s_pen );
	painter.drawRect( 0, 0, px, height() );
	painter.drawRect( 0, height()-px, width(), height() );

	QWidget::paintEvent(event);
    }

    /*#############################################################
      #############################################################
      ### MainWidgetPrivate                                     ###
      #############################################################
      #############################################################
    */

    /**
     * \brief Create and connect generic actions
     */
    void MainWidgetPrivate::setup_actions()
    {
        _act.go_matrix = new QAction( "M", _p );
        _act.go_matrix->setToolTip( tr("Go to matrix view") );
        _p->addAction( _act.go_matrix );
        _p->connect( _act.go_matrix, SIGNAL(triggered()),
                     _p, SLOT(go_matrix()) );

        _act.go_edit = new QAction( "E", _p );
        _act.go_edit->setToolTip( tr("Go to edit view") );
        _p->addAction( _act.go_edit );
        _p->connect( _act.go_edit, SIGNAL(triggered()),
                     _p, SLOT(go_edit()) );

        _act.x_play = new QAction( "P", _p );
        _act.x_play->setToolTip( tr("Start playing") );
        _p->addAction( _act.x_play );
        _p->connect( _act.x_play, SIGNAL(triggered()),
                     _p, SLOT(x_play()) );

        _act.x_stop = new QAction( "S", _p );
        _act.x_stop->setToolTip( tr("Stop playing") );
        _p->addAction( _act.x_stop );
        _p->connect( _act.x_stop, SIGNAL(triggered()),
                     _p, SLOT(x_stop()) );
    }

    void MainWidgetPrivate::setup_widgets()
    {
        _tbtn.go_matrix = new QToolButton(_p);
        _tbtn.go_matrix->setDefaultAction(_act.go_matrix);

        _tbtn.go_edit = new QToolButton(_p);
        _tbtn.go_edit->setDefaultAction(_act.go_edit);

        _tbtn.x_play = new QToolButton(_p);
        _tbtn.x_play->setDefaultAction(_act.x_play);

        _tbtn.x_stop = new QToolButton(_p);
        _tbtn.x_stop->setDefaultAction(_act.x_stop);
    }

    void MainWidgetPrivate::layout_widgets()
    {
        QHBoxLayout *lay = new QHBoxLayout;

        lay->addWidget(_tbtn.go_matrix);
        lay->addWidget(_tbtn.go_edit);
        lay->addStretch();
        lay->addWidget(_tbtn.x_play);
        lay->addWidget(_tbtn.x_stop);

        _p->setLayout(lay);
    }

    void MainWidgetPrivate::setup_signals_and_slots()
    {
    }

} // namespace Main
} // namespace Composite

