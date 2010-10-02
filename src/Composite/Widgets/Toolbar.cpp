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
#include "ToolbarPrivate.hpp"

#include <QtGui/QWidget>

#include <cassert>

namespace Composite
{
namespace Widgets
{
    Toolbar::Toolbar(QWidget *parent) :
	QWidget(parent),
	_d( new ToolbarPrivate )
    {}

    Toolbar::~Toolbar()
    {
	delete _d;
    }

    Toolbar::orientation_t Toolbar::orientation()
    {
	return _d->orientation;
    }

    void Toolbar::orientation(Toolbar::orientation_t o)
    {
	if( _d->orientation != o ) {
	    _d->orientation = o;
	    update();
	}
    }

    void Toolbar::addWidget(QWidget *w)
    {
	w->setParent(this);
	_d->widgets.push_back(w);
    }

    void Toolbar::addStretch()
    {
	_d->widgets.push_back(0);
    }

    void Toolbar::paintEvent(QPaintEvent *event)
    {
	int w, h, pos;
	ToolbarPrivate::seq_t::iterator it;

	w = width();
	h = height();
	pos = 0;

	if( _d->orientation == HORIZONTAL ) {
	    for( it = _d->widgets.begin() ; it != _d->widgets.end() ; ++it ) {
		if( *it ) {
		    (*it)->resize( h, h );
		    (*it)->move( pos, 0 );
		    pos += (*it)->size().width();
		} else {
		    pos += h/2;
		}
	    }
	} else {
	    assert( _d->orientation == VERTICAL );
	    for( it = _d->widgets.begin() ; it != _d->widgets.end() ; ++it ) {
		if( *it ) {
		    (*it)->resize( w, w );
		    (*it)->move( 0, pos );
		    pos += (*it)->size().height();
		} else {
		    pos += w/2;
		}
	    }
	}

	QWidget::paintEvent(event);
    }

} // namespace Widgets
} // namespace Composite

