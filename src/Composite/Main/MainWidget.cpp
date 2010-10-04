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

#include <Composite/Widgets/Toolbar.hpp>

#include <Composite/Main/MatrixView.hpp>
#include <Composite/Main/LibraryView.hpp>

#include <Composite/Models/AudioEngine.hpp>

#include <QtGui/QAction>
#include <QtGui/QHBoxLayout>
#include <QtGui/QMessageBox>
#include <QtGui/QPainter>
#include <QtGui/QToolBar>
#include <QtGui/QToolButton>
#include <QtGui/QPushButton>
#include <QtGui/QPalette>

#include <stdexcept>

using Composite::Widgets::Toolbar;

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

	resize(600, 400);

	_d->_engine.reset( new Composite::Models::AudioEngine(0, this) );

	_d->load_icons();
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

    void MainWidget::set_audio_engine( Tritium::Engine *eng )
    {
	Tritium::T<Models::AudioEngine>::shared_ptr mod( new Models::AudioEngine(eng, this) );
	_d->_mode.matrix->setModel( mod.get() );
	_d->_engine = mod;
    }

    /**
     * \brief Go to the matrix view
     */
    void MainWidget::go_matrix()
    {
	_d->_central_widget->hide();
	_d->_central_widget = _d->_mode.matrix;
	_d->_central_widget->show();
	update();
    }

    /**
     * \brief Go to the edit view
     */
    void MainWidget::go_edit()
    {
	_d->_central_widget->hide();
	_d->_central_widget = _d->_mode.edit;
	_d->_central_widget->show();
	update();
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

	//
	// Establish partitions
	//
	float px = _d->_sizes.minimum_button().pixels();
	float h1, h2; // horizontal boundaries
	float v1; // vertical boundaries
	h1 = px;
	h2 = width() * 3.0 / 4.0;
	v1 = height() - px;

	//
	// Set regions
	//
	QRectF left, right, bottom, corner, central;
	left.setCoords(     0,  0,      h1,       v1 );
	right.setCoords(   h2,  0, width(), height() );
	bottom.setCoords(  h1, v1,      h2, height() );
	corner.setCoords(   0, v1,      h1, height() );
	central.setCoords( h1,  0,      h2,       v1 );

	QPalette pal = palette();
	QColor bg = pal.color( QPalette::Active, QPalette::Window );
	QColor side = pal.color( QPalette::Active, QPalette::Dark );

	//QColor bg(0, 0, 0x70);
	//QColor side(0xEE, 0, 0);

	QBrush bg_brush( bg );
	QPen bg_pen( bg );

	QBrush s_brush( side );
	QPen s_pen( side );

	painter.setBrush( bg_brush );
	painter.setPen( bg_pen );
	painter.drawRect( 0, 0, width(), height() );

	painter.setBrush( s_brush );
	painter.setPen( s_pen );
	painter.drawRect( left );
	painter.drawRect( bottom );
	painter.drawRect( corner );

	_d->_tbar.left->setGeometry( left.toRect() );
	_d->_tbar.bottom->setGeometry( bottom.toRect() );
	_d->_central_widget->setGeometry( central.toRect() );
	_d->_library_widget->setGeometry( right.toRect() );
	_d->_bot_left_widget->setGeometry( corner.toRect() );

	QWidget::paintEvent(event);
    }

    /*#############################################################
      #############################################################
      ### MainWidgetPrivate                                     ###
      #############################################################
      #############################################################
    */

    /**
     * \brief Load all the icons that we need
     */
    void MainWidgetPrivate::load_icons()
    {
	_ico.go_matrix.addFile(":img/icon_matrix.png");
	_ico.go_edit.addFile(":img/icon_edit.png");
	_ico.x_play.addFile(":img/icon_play.png");
	_ico.x_stop.addFile(":img/icon_stop.png");
    }

    /**
     * \brief Create and connect generic actions
     */
    void MainWidgetPrivate::setup_actions()
    {
	QSize max(128, 128);

        _act.go_matrix = new QAction( "M", _p );
        _act.go_matrix->setToolTip( tr("Go to matrix view") );
	_act.go_matrix->setIcon( _ico.go_matrix );
        _p->addAction( _act.go_matrix );
        _p->connect( _act.go_matrix, SIGNAL(triggered()),
                     _p, SLOT(go_matrix()) );

        _act.go_edit = new QAction( "E", _p );
        _act.go_edit->setToolTip( tr("Go to edit view") );
	_act.go_edit->setIcon( _ico.go_edit );
        _p->addAction( _act.go_edit );
        _p->connect( _act.go_edit, SIGNAL(triggered()),
                     _p, SLOT(go_edit()) );

        _act.x_play = new QAction( "P", _p );
        _act.x_play->setToolTip( tr("Start playing") );
	_act.x_play->setIcon( _ico.x_play );
        _p->addAction( _act.x_play );
        _p->connect( _act.x_play, SIGNAL(triggered()),
                     _p, SLOT(x_play()) );

        _act.x_stop = new QAction( "S", _p );
        _act.x_stop->setToolTip( tr("Stop playing") );
	_act.x_stop->setIcon( _ico.x_stop );
        _p->addAction( _act.x_stop );
        _p->connect( _act.x_stop, SIGNAL(triggered()),
                     _p, SLOT(x_stop()) );
    }

    static
    void setup_tool_button(QToolButton **dest, QAction *act, QWidget *parent)
    {
	*dest = new QToolButton(parent);
	(*dest)->setDefaultAction(act);
	(*dest)->setAutoRaise(true);
	(*dest)->setContentsMargins(0, 0, 0, 0);
	(*dest)->setIconSize( QSize(128, 128) );
	(*dest)->setFocusPolicy( Qt::NoFocus );
    }

    void MainWidgetPrivate::setup_widgets()
    {
	setup_tool_button( &_tbtn.go_matrix, _act.go_matrix, _p );
	setup_tool_button( &_tbtn.go_edit,   _act.go_edit,   _p );
	setup_tool_button( &_tbtn.x_play,    _act.x_play,    _p );
	setup_tool_button( &_tbtn.x_stop,    _act.x_stop,    _p );

	_mode.matrix = new MatrixView(_p);
	_mode.matrix->hide();
	_mode.matrix->setModel( _engine.get() );
	_mode.edit = new QPushButton("EDIT ME!!", _p);
	_mode.edit->hide();
	_library_widget = new LibraryView(_p);
	_bot_left_widget = _tbtn.go_edit;
    }

    void MainWidgetPrivate::layout_widgets()
    {
	_tbar.left = new Toolbar(_p);
	_tbar.left->orientation( Toolbar::VERTICAL );
	_tbar.left->addWidget( _tbtn.go_matrix );

	_tbar.bottom = new Toolbar(_p);
	_tbar.bottom->orientation( Toolbar::HORIZONTAL );
	_tbar.bottom->addWidget( _tbtn.x_play );
	_tbar.bottom->addWidget( _tbtn.x_stop );

	_central_widget = _mode.matrix;
	_central_widget->show();
    }

    void MainWidgetPrivate::setup_signals_and_slots()
    {
    }

} // namespace Main
} // namespace Composite

