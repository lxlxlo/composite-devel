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

#include <QTimer>
#include <QPainter>

#include <Tritium/Preferences.hpp>
#include <Tritium/Engine.hpp>
#include <Tritium/Transport.hpp>
#include <Tritium/Pattern.hpp>
#include <Tritium/Logger.hpp>
using namespace Tritium;

#include "PatternEditorRuler.hpp"
#include "PatternEditorPanel.hpp"
#include "../CompositeApp.hpp"
#include "../Skin.hpp"


PatternEditorRuler::PatternEditorRuler( QWidget* parent )
 : QWidget( parent )
{
	setAttribute(Qt::WA_NoBackground);

	//infoLog( "INIT" );

	T<Preferences>::shared_ptr pPref = g_engine->get_preferences();

	//QColor backgroundColor(230, 230, 230);
	UIStyle *pStyle = pPref->getDefaultUIStyle();
	QColor backgroundColor( pStyle->m_patternEditor_backgroundColor.getRed(), pStyle->m_patternEditor_backgroundColor.getGreen(), pStyle->m_patternEditor_backgroundColor.getBlue() );


	m_pPattern.reset();
	m_nGridWidth = g_engine->get_preferences()->getPatternEditorGridWidth();

	m_nRulerWidth = 20 + m_nGridWidth * ( MAX_NOTES * 4 );
	m_nRulerHeight = 25;

	resize( m_nRulerWidth, m_nRulerHeight );
//	setFixedSize( size() );

	bool ok = m_tickPosition.load( Skin::getImagePath() + "/patternEditor/tickPosition.png" );
	if( ok == false ){
		ERRORLOG( "Error loading pixmap " );
	}

	m_pBackground = new QPixmap( m_nRulerWidth, m_nRulerHeight );
	m_pBackground->fill( backgroundColor );

	m_pTimer = new QTimer(this);
	connect(m_pTimer, SIGNAL(timeout()), this, SLOT(updateEditor()));

	CompositeApp::get_instance()->addEventListener( this );
}



PatternEditorRuler::~PatternEditorRuler() {
	//infoLog( "DESTROY");
}



void PatternEditorRuler::updateStart(bool start) {
	if (start) {
		m_pTimer->start(50);	// update ruler at 20 fps
	}
	else {
		m_pTimer->stop();
	}
}



void PatternEditorRuler::showEvent ( QShowEvent * /*ev*/ )
{
	updateEditor();
	updateStart(true);
}



void PatternEditorRuler::hideEvent ( QHideEvent * /*ev*/ )
{
	updateStart(false);
}



void PatternEditorRuler::updateEditor( bool bRedrawAll )
{
	static int oldNTicks = 0;

	Engine *pEngine = g_engine;
	PatternList *pPatternList = pEngine->getSong()->get_pattern_list();
	int nSelectedPatternNumber = pEngine->getSelectedPatternNumber();
	if ( (nSelectedPatternNumber != -1) && ( (uint)nSelectedPatternNumber < pPatternList->get_size() )  ) {
		m_pPattern = pPatternList->get( nSelectedPatternNumber );
	}
	else {
		m_pPattern.reset();
	}


	bool bActive = false;	// is the pattern playing now?
	T<PatternList>::shared_ptr pList = pEngine->getCurrentPatternList();
	for (uint i = 0; i < pList->get_size(); i++) {
		if ( m_pPattern == pList->get(i) ) {
			bActive = true;
			break;
		}
	}

	TransportPosition::State state = pEngine->get_transport()->get_state();
	if ( ( state == TransportPosition::ROLLING ) && (bActive) ) {
		m_nTicks = pEngine->getTickPosition();
	}
	else {
		m_nTicks = -1;	// hide the tickPosition
	}


	if (oldNTicks != m_nTicks) {
		// redraw all
		bRedrawAll = true;
		//update( 0, 0, width(), height() );
	}
	oldNTicks = m_nTicks;

	if (bRedrawAll) {
		update( 0, 0, width(), height() );
	}
}



void PatternEditorRuler::paintEvent( QPaintEvent *ev)
{
	if (!isVisible()) {
		return;
	}

	QPainter painter(this);

	painter.drawPixmap( ev->rect(), *m_pBackground, ev->rect() );

	// gray background for unusable section of pattern
	if (m_pPattern) {
		int nXStart = 20 + m_pPattern->get_length() * m_nGridWidth;
		if ( (m_nRulerWidth - nXStart) != 0 ) {
			painter.fillRect( nXStart, 0, m_nRulerWidth - nXStart, m_nRulerHeight, QColor(170,170,170) );
		}
	}

	// numbers
	QColor textColor( 100, 100, 100 );
	QColor lineColor( 170, 170, 170 );

	T<Preferences>::shared_ptr pref = g_engine->get_preferences();
	QString family = pref->getApplicationFontFamily();
	int size = pref->getApplicationFontPointSize();
	QFont font( family, size );
	painter.setFont(font);
	painter.drawLine( 0, 0, m_nRulerWidth, 0 );
	painter.drawLine( 0, m_nRulerHeight - 1, m_nRulerWidth - 1, m_nRulerHeight - 1);

	uint nQuarter = 48;

	for ( int i = 0; i < 64 ; i++ ) {
		int nText_x = 20 + nQuarter / 4 * i * m_nGridWidth;
		if ( ( i % 4 ) == 0 ) {
			painter.setPen( textColor );
			painter.drawText( nText_x - 30, 0, 60, m_nRulerHeight, Qt::AlignCenter, QString("%1").arg(i / 4 + 1) );
//			ERRORLOG(QString("nText_x: %1, true, value: %2").arg(nText_x).arg(i / 4 + 1));
		}
		else {
			painter.setPen( lineColor );
			painter.drawLine( nText_x, ( m_nRulerHeight - 5 ) / 2, nText_x, m_nRulerHeight - ( (m_nRulerHeight - 5 ) / 2 ) );
		}
	}

	// draw tickPosition
	if (m_nTicks != -1) {
		uint x = (uint)( 20 + m_nTicks * m_nGridWidth - 5 - 11 / 2.0 );
		painter.drawPixmap( QRect( x, height() / 2, 11, 8 ), m_tickPosition, QRect( 0, 0, 11, 8 ) );

	}
}



void PatternEditorRuler::zoomIn()
{
	if (m_nGridWidth >= 3){
		m_nGridWidth *= 2;
	}else
	{
		m_nGridWidth *= 1.5;
	}
	m_nRulerWidth = 20 + m_nGridWidth * ( MAX_NOTES * 4 );
	resize(  QSize(m_nRulerWidth, m_nRulerHeight ));
	delete m_pBackground;
	m_pBackground = new QPixmap( m_nRulerWidth, m_nRulerHeight );
	UIStyle *pStyle = g_engine->get_preferences()->getDefaultUIStyle();
	QColor backgroundColor( pStyle->m_patternEditor_backgroundColor.getRed(), pStyle->m_patternEditor_backgroundColor.getGreen(), pStyle->m_patternEditor_backgroundColor.getBlue() );
	m_pBackground->fill( backgroundColor );
	update();
}


void PatternEditorRuler::zoomOut()
{
	if ( m_nGridWidth > 1.5 ) {
		if (m_nGridWidth > 3){
			m_nGridWidth /= 2;
		}else
		{
			m_nGridWidth /= 1.5;
		}
	m_nRulerWidth = 20 + m_nGridWidth * ( MAX_NOTES * 4 );
	resize( QSize(m_nRulerWidth, m_nRulerHeight) );
	delete m_pBackground;
	m_pBackground = new QPixmap( m_nRulerWidth, m_nRulerHeight );
	UIStyle *pStyle = g_engine->get_preferences()->getDefaultUIStyle();
	QColor backgroundColor( pStyle->m_patternEditor_backgroundColor.getRed(), pStyle->m_patternEditor_backgroundColor.getGreen(), pStyle->m_patternEditor_backgroundColor.getBlue() );
	m_pBackground->fill( backgroundColor );
	update();
	}
}


void PatternEditorRuler::selectedPatternChangedEvent()
{
	updateEditor( true );
}
