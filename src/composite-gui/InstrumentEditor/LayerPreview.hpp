/*
 * Copyright(c) 2002-2008 by Alex >Comix< Cominu [comix@users.sourceforge.net]
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
#ifndef COMPOSITE_LAYERPREVIEW_HPP
#define COMPOSITE_LAYERPREVIEW_HPP

#include "config.h"
#include <QtGui>

#include <Tritium/Instrument.hpp>
#include "../EventListener.hpp"


class LayerPreview : public QWidget, public EventListener
{
	Q_OBJECT

	public:
		LayerPreview(QWidget* pParent);
		~LayerPreview();

		void updateAll();

		void paintEvent(QPaintEvent *ev);
		virtual void mousePressEvent(QMouseEvent *ev);
		virtual void mouseReleaseEvent(QMouseEvent *ev);
		virtual void mouseMoveEvent ( QMouseEvent *ev );

	private:
		static const int m_nLayerHeight = 10;
		QPixmap m_speakerPixmap;
		Tritium::T<Tritium::Instrument>::shared_ptr m_pInstrument;
		int m_nSelectedLayer;
		bool m_bMouseGrab;
		bool m_bGrabLeft;

		virtual void selectedInstrumentChangedEvent();
};


#endif // COMPOSITE_LAYERPREVIEW_HPP
