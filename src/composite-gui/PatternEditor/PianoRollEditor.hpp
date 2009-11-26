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
/*
#ifndef COMPOSITE_PIANOROLLEDITOR_HPP
#define COMPOSITE_PIANOROLLEDITOR_HPP

#include <Tritium/Object.hpp>
#include "../EventListener.hpp"

#include <QtGui>

namespace Tritium
{
	class Pattern;
	class Note;
}

class PianoRollEditor: public QWidget, public EventListener, public Object
{
	public:
		PianoRollEditor( QWidget *pParent );
		~PianoRollEditor();


		// Implements EventListener interface
		virtual void selectedPatternChangedEvent();
		virtual void selectedInstrumentChangedEvent();
		//~ Implements EventListener interface




	private:
		Tritium::Pattern *m_pPattern;

		unsigned m_nRowHeight;
		unsigned m_nOctaves;

		QPixmap *m_pBackground;
		QPixmap *m_pTemp;

		void createBackground();
		void drawPattern();
		void drawNote( Tritium::Note *pNote, QPainter *pPainter );

		virtual void paintEvent(QPaintEvent *ev);
		virtual void mousePressEvent(QMouseEvent *ev);
		virtual void mouseReleaseEvent(QMouseEvent *ev);

};

#endif // COMPOSITE_PIANOROLLEDITOR_HPP
*/
