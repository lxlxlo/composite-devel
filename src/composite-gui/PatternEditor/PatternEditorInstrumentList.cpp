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

#include "PatternEditorInstrumentList.hpp"

#include <Tritium/EventQueue.hpp>
#include <Tritium/Engine.hpp>
#include <Tritium/Instrument.hpp>
#include <Tritium/InstrumentList.hpp>
#include <Tritium/Note.hpp>
#include <Tritium/Pattern.hpp>
#include <Tritium/Preferences.hpp>
#include <Tritium/Song.hpp>
#include <Tritium/Logger.hpp>
#include <Tritium/memory.hpp>

using namespace Tritium;

#include "PatternEditorPanel.hpp"
#include "DrumPatternEditor.hpp"
#include "../CompositeApp.hpp"
#include "../Mixer/Mixer.hpp"
#include "../widgets/Button.hpp"

#include <QtGui>
#include <cassert>

using namespace std;





InstrumentLine::InstrumentLine(QWidget* pParent)
  : PixmapWidget(pParent, "InstrumentLine")
  , m_bIsSelected(false)
{
	int h = g_engine->get_preferences()->getPatternEditorGridHeight();
	setFixedSize(181, h);

	m_pNameLbl = new QLabel(this);
	m_pNameLbl->resize( 145, h );
	m_pNameLbl->move( 10, 1 );
	QFont nameFont;
	nameFont.setPointSize( 10 );
	nameFont.setBold( true );
	m_pNameLbl->setFont(nameFont);

	m_pMuteBtn = new ToggleButton(
			this,
			"/patternEditor/btn_mute_on.png",
			"/patternEditor/btn_mute_off.png",
			"/patternEditor/btn_mute_off.png",
			QSize( 9, 9 )
	);
	//m_pMuteBtn->setText( "M" );
	m_pMuteBtn->move( 155, 5 );
	m_pMuteBtn->setPressed(false);
	connect(m_pMuteBtn, SIGNAL(clicked(Button*)), this, SLOT(muteClicked()));

	m_pSoloBtn = new ToggleButton(
			this,
			"/patternEditor/btn_solo_on.png",
			"/patternEditor/btn_solo_off.png",
			"/patternEditor/btn_solo_off.png",
			QSize( 9, 9 )
	);
	//m_pSoloBtn->setText( "S" );
	m_pSoloBtn->move( 165, 5 );
	m_pSoloBtn->setPressed(false);
	connect(m_pSoloBtn, SIGNAL(clicked(Button*)), this, SLOT(soloClicked()));



	// Popup menu
	m_pFunctionPopup = new QMenu( this );
	m_pFunctionPopup->addAction( trUtf8( "Clear notes" ), this, SLOT( functionClearNotes() ) );
	m_pFunctionPopup->addAction( trUtf8( "Fill notes" ), this, SLOT( functionFillNotes() ) );
	m_pFunctionPopup->addAction( trUtf8( "Randomize velocity" ), this, SLOT( functionRandomizeVelocity() ) );
	m_pFunctionPopup->addSeparator();
	m_pFunctionPopup->addAction( trUtf8( "Delete instrument" ), this, SLOT( functionDeleteInstrument() ) );

	m_bIsSelected = true;
	setSelected(false);
}



void InstrumentLine::setName(const QString& sName)
{
	m_pNameLbl->setText(sName);
}



void InstrumentLine::setSelected(bool bSelected)
{
	if (bSelected == m_bIsSelected) {
		return;
	}
	m_bIsSelected = bSelected;
	if (m_bIsSelected) {
		setPixmap( "/patternEditor/instrument_line_selected.png");
	}
	else {
		setPixmap( "/patternEditor/instrument_line.png");
	}
}



void InstrumentLine::setNumber(int nIndex)
{
	m_nInstrumentNumber = nIndex;
}



void InstrumentLine::setMuted(bool isMuted)
{
	m_pMuteBtn->setPressed(isMuted);
}


void InstrumentLine::setSoloed( bool soloed )
{
	m_pSoloBtn->setPressed( soloed );
}



void InstrumentLine::muteClicked()
{
	Engine *engine = g_engine;
	T<Song>::shared_ptr song = engine->getSong();
	InstrumentList *instrList = song->get_instrument_list();

	T<Instrument>::shared_ptr pInstr = instrList->get(m_nInstrumentNumber);
	pInstr->set_muted( !pInstr->is_muted());
}



void InstrumentLine::soloClicked()
{
	CompositeApp::get_instance()->getMixer()->soloClicked( m_nInstrumentNumber );
}



void InstrumentLine::mousePressEvent(QMouseEvent *ev)
{
	g_engine->setSelectedInstrumentNumber( m_nInstrumentNumber );

	if ( ev->button() == Qt::LeftButton ) {
		const float velocity = 0.8f;
		const float pan_L = 0.5f;
		const float pan_R = 0.5f;
		const int nLength = -1;
		const float fPitch = 0.0f;
		T<Song>::shared_ptr pSong = g_engine->getSong();
		
		T<Instrument>::shared_ptr pInstr = pSong->get_instrument_list()->get( m_nInstrumentNumber );
		
		Note *pNote = new Note( pInstr, velocity, pan_L, pan_R, nLength, fPitch);
		g_engine->midi_noteOn(pNote);
	}
	else if (ev->button() == Qt::RightButton ) {
		m_pFunctionPopup->popup( QPoint( ev->globalX(), ev->globalY() ) );
	}

	// propago l'evento al parent: serve per il drag&drop
	PixmapWidget::mousePressEvent(ev);
}



T<Tritium::Pattern>::shared_ptr InstrumentLine::getCurrentPattern()
{
	Engine *pEngine = g_engine;
	PatternList *pPatternList = pEngine->getSong()->get_pattern_list();
	assert( pPatternList != NULL );

	int nSelectedPatternNumber = pEngine->getSelectedPatternNumber();
	if ( nSelectedPatternNumber != -1 ) {
		T<Pattern>::shared_ptr pCurrentPattern = pPatternList->get( nSelectedPatternNumber );
		return pCurrentPattern;
	}
	return T<Pattern>::shared_ptr();
}




void InstrumentLine::functionClearNotes()
{
// 	g_engine->lock( RIGHT_HERE );	// lock the audio engine

	Engine * H = g_engine;
	T<Pattern>::shared_ptr pCurrentPattern = getCurrentPattern();

	int nSelectedInstrument = m_nInstrumentNumber;
	T<Instrument>::shared_ptr pSelectedInstrument = H->getSong()->get_instrument_list()->get( nSelectedInstrument );
	
	pCurrentPattern->purge_instrument( pSelectedInstrument, H );
// 	Pattern::note_map_t::iterator pos;
// 	for ( pos = pCurrentPattern->note_map.begin(); pos != pCurrentPattern->note_map.end(); ++pos ) {
// 		Note *pNote = pos->second;
// 		assert( pNote );
// 		if ( pNote->get_instrument() != pSelectedInstrument ) {
// 			continue;
// 		}
// 
// 		delete pNote;
// 		pCurrentPattern->note_map.erase( pos );
// 	}
// 	g_engine->unlock();	// unlock the audio engine

	// this will force an update...
	g_engine->get_event_queue()->push_event( EVENT_SELECTED_INSTRUMENT_CHANGED, -1 );
}




void InstrumentLine::functionFillNotes()
{
	Engine *pEngine = g_engine;

	const float velocity = 0.8f;
	const float pan_L = 0.5f;
	const float pan_R = 0.5f;
	const float fPitch = 0.0f;
	const int nLength = -1;

	PatternEditorPanel *pPatternEditorPanel = CompositeApp::get_instance()->getPatternEditorPanel();
	DrumPatternEditor *pPatternEditor = pPatternEditorPanel->getDrumPatternEditor();
	int nBase;
	if ( pPatternEditor->isUsingTriplets() ) {
		nBase = 3;
	}
	else {
		nBase = 4;
	}
	int nResolution = 4 * MAX_NOTES / ( nBase * pPatternEditor->getResolution() );


	g_engine->lock( RIGHT_HERE );	// lock the audio engine


	T<Song>::shared_ptr pSong = pEngine->getSong();

	T<Pattern>::shared_ptr pCurrentPattern = getCurrentPattern();
	if (pCurrentPattern != NULL) {
		int nPatternSize = pCurrentPattern->get_length();
		int nSelectedInstrument = pEngine->getSelectedInstrumentNumber();

		if (nSelectedInstrument != -1) {
			T<Instrument>::shared_ptr instrRef = (pSong->get_instrument_list())->get( nSelectedInstrument );

			for (int i = 0; i < nPatternSize; i += nResolution) {
				bool noteAlreadyPresent = false;

				Pattern::note_map_t::iterator pos;
				for ( pos = pCurrentPattern->note_map.lower_bound( i ); pos != pCurrentPattern->note_map.upper_bound( i ); ++pos ) {
					Note *pNote = pos->second;
					if ( pNote->get_instrument() == instrRef ) {
						// note already exists
						noteAlreadyPresent = true;
						break;
					}
				}

				if ( noteAlreadyPresent == false ) {
					// create the new note
					Note *pNote = new Note( instrRef, velocity, pan_L, pan_R, nLength, fPitch );
					//pNote->setInstrument(instrRef);
					pCurrentPattern->note_map.insert( std::make_pair( i, pNote ) );
				}
			}
		}
	}
	g_engine->unlock();	// unlock the audio engine

	// this will force an update...
	g_engine->get_event_queue()->push_event( EVENT_SELECTED_INSTRUMENT_CHANGED, -1 );
}





void InstrumentLine::functionRandomizeVelocity()
{
	Engine *pEngine = g_engine;

	PatternEditorPanel *pPatternEditorPanel = CompositeApp::get_instance()->getPatternEditorPanel();
	DrumPatternEditor *pPatternEditor = pPatternEditorPanel->getDrumPatternEditor();

	g_engine->lock( RIGHT_HERE );	// lock the audio engine

	int nBase;
	if ( pPatternEditor->isUsingTriplets() ) {
		nBase = 3;
	}
	else {
		nBase = 4;
	}
	int nResolution = 4 * MAX_NOTES / ( nBase * pPatternEditor->getResolution() );

	T<Song>::shared_ptr pSong = pEngine->getSong();

	T<Pattern>::shared_ptr pCurrentPattern = getCurrentPattern();
	if (pCurrentPattern != NULL) {
		int nPatternSize = pCurrentPattern->get_length();
		int nSelectedInstrument = pEngine->getSelectedInstrumentNumber();

		if (nSelectedInstrument != -1) {
			T<Instrument>::shared_ptr instrRef = (pSong->get_instrument_list())->get( nSelectedInstrument );

			for (int i = 0; i < nPatternSize; i += nResolution) {
				Pattern::note_map_t::iterator pos;
				for ( pos = pCurrentPattern->note_map.lower_bound(i); pos != pCurrentPattern->note_map.upper_bound( i ); ++pos ) {
					Note *pNote = pos->second;
					if ( pNote->get_instrument() == instrRef ) {
						float fVal = ( rand() % 100 ) / 100.0;
						fVal = pNote->get_velocity() + ( ( fVal - 0.50 ) / 2 );
						if ( fVal < 0  ) {
							fVal = 0;
						}
						if ( fVal > 1 ) {
							fVal = 1;
						}
						pNote->set_velocity(fVal);
					}
				}
			}
		}
	}
	g_engine->unlock();	// unlock the audio engine

	// this will force an update...
	g_engine->get_event_queue()->push_event( EVENT_SELECTED_INSTRUMENT_CHANGED, -1 );

}





void InstrumentLine::functionDeleteInstrument()
{
	Engine *pEngine = g_engine;
	pEngine->removeInstrument( m_nInstrumentNumber, false );
	
	g_engine->lock( RIGHT_HERE );
#ifdef JACK_SUPPORT
	pEngine->renameJackPorts();
#endif
	g_engine->unlock();
}



//////



PatternEditorInstrumentList::PatternEditorInstrumentList( QWidget *parent, PatternEditorPanel *pPatternEditorPanel )
 : QWidget( parent )
{
	//setAttribute(Qt::WA_NoBackground);

	//INFOLOG("INIT");
	m_pPattern = NULL;
 	m_pPatternEditorPanel = pPatternEditorPanel;

	m_nGridHeight = g_engine->get_preferences()->getPatternEditorGridHeight();

	m_nEditorWidth = 181;
	m_nEditorHeight = m_nGridHeight * MAX_INSTRUMENTS;

	resize( m_nEditorWidth, m_nEditorHeight );


	setAcceptDrops(true);

	for ( int i = 0; i < MAX_INSTRUMENTS; ++i) {
		m_pInstrumentLine[i] = NULL;
	}


	updateInstrumentLines();

	m_pUpdateTimer = new QTimer( this );
	connect( m_pUpdateTimer, SIGNAL( timeout() ), this, SLOT( updateInstrumentLines() ) );
	m_pUpdateTimer->start(50);

}



PatternEditorInstrumentList::~PatternEditorInstrumentList()
{
	//INFOLOG( "DESTROY" );
	m_pUpdateTimer->stop();
}




///
/// Create a new InstrumentLine
///
InstrumentLine* PatternEditorInstrumentList::createInstrumentLine()
{
	InstrumentLine *pLine = new InstrumentLine(this);
	return pLine;
}


void PatternEditorInstrumentList::moveInstrumentLine( int nSourceInstrument , int nTargetInstrument )
{
		Engine *engine = g_engine;
		g_engine->lock( RIGHT_HERE );

		T<Song>::shared_ptr pSong = engine->getSong();
		InstrumentList *pInstrumentList = pSong->get_instrument_list();

		if ( ( nTargetInstrument > (int)pInstrumentList->get_size() ) || ( nTargetInstrument < 0) ) {
			g_engine->unlock();
			return;
		}


		// move instruments...

		T<Instrument>::shared_ptr pSourceInstr = pInstrumentList->get(nSourceInstrument);
		if ( nSourceInstrument < nTargetInstrument) {
			for (int nInstr = nSourceInstrument; nInstr < nTargetInstrument; nInstr++) {
				T<Instrument>::shared_ptr pInstr = pInstrumentList->get(nInstr + 1);
				pInstrumentList->replace( pInstr, nInstr );
			}
			pInstrumentList->replace( pSourceInstr, nTargetInstrument );
		}
		else {
			for (int nInstr = nSourceInstrument; nInstr >= nTargetInstrument; nInstr--) {
				T<Instrument>::shared_ptr pInstr = pInstrumentList->get(nInstr - 1);
				pInstrumentList->replace( pInstr, nInstr );
			}
			pInstrumentList->replace( pSourceInstr, nTargetInstrument );
		}

		#ifdef JACK_SUPPORT
		engine->renameJackPorts();
		#endif

		g_engine->unlock();
		engine->setSelectedInstrumentNumber( nTargetInstrument );

		pSong->set_modified( true );
}

///
/// Update every InstrumentLine, create or destroy lines if necessary.
///
void PatternEditorInstrumentList::updateInstrumentLines()
{
	//INFOLOG( "Update lines" );

	Engine *pEngine = g_engine;
	T<Song>::shared_ptr pSong = pEngine->getSong();
	InstrumentList *pInstrList = pSong->get_instrument_list();
	Mixer * mixer = CompositeApp::get_instance()->getMixer();

	unsigned nSelectedInstr = pEngine->getSelectedInstrumentNumber();

	unsigned nInstruments = pInstrList->get_size();
	for ( unsigned nInstr = 0; nInstr < MAX_INSTRUMENTS; ++nInstr ) {
		if ( nInstr >= nInstruments ) {	// unused instrument! let's hide and destroy the mixerline!
			if ( m_pInstrumentLine[ nInstr ] ) {
				delete m_pInstrumentLine[ nInstr ];
				m_pInstrumentLine[ nInstr ] = NULL;

				int newHeight = m_nGridHeight * nInstruments;
				resize( width(), newHeight );

			}
			continue;
		}
		else {
			if ( m_pInstrumentLine[ nInstr ] == NULL ) {
				// the instrument line doesn't exists..I'll create a new one!
				m_pInstrumentLine[ nInstr ] = createInstrumentLine();
				m_pInstrumentLine[nInstr]->move( 0, m_nGridHeight * nInstr );
				m_pInstrumentLine[nInstr]->show();

				int newHeight = m_nGridHeight * nInstruments;
				resize( width(), newHeight );
			}
			InstrumentLine *pLine = m_pInstrumentLine[ nInstr ];
			T<Instrument>::shared_ptr pInstr = pInstrList->get(nInstr);
			assert(pInstr);

			pLine->setNumber(nInstr);
			pLine->setName( pInstr->get_name() );
			pLine->setSelected( nInstr == nSelectedInstr );
			pLine->setMuted( pInstr->is_muted() );
			if ( mixer ) {
				pLine->setSoloed( mixer->isSoloClicked( nInstr ) );
			}

		}
	}

}




void PatternEditorInstrumentList::dragEnterEvent(QDragEnterEvent *event)
{
	INFOLOG( "[dragEnterEvent]" );
	if ( event->mimeData()->hasFormat("text/plain") ) {
		T<Song>::shared_ptr song = (g_engine)->getSong();
		int nInstruments = song->get_instrument_list()->get_size();
		if ( nInstruments < MAX_INSTRUMENTS ) {
			event->acceptProposedAction();
		}
	}
}


void PatternEditorInstrumentList::dropEvent(QDropEvent *event)
{
	//WARNINGLOG("Drop!");
	QString sText = event->mimeData()->text();
	ERRORLOG(sText);
	

	if(sText.startsWith("Songs:") || sText.startsWith("Patterns:") || sText.startsWith("move pattern:") || sText.startsWith("drag pattern:")) return;

	if (sText.startsWith("move instrument:")) {

		Engine *engine = g_engine;
		int nSourceInstrument = engine->getSelectedInstrumentNumber();

		int nTargetInstrument = event->pos().y() / m_nGridHeight;

		if ( nSourceInstrument == nTargetInstrument ) {
			event->acceptProposedAction();
			return;
		}

		moveInstrumentLine( nSourceInstrument , nTargetInstrument );

		event->acceptProposedAction();
	}
	else {
		

		QStringList tokens = sText.split( "::" );
		QString sDrumkitName = tokens.at( 0 );
		QString sInstrumentName = tokens.at( 1 );
		
		T<Instrument>::shared_ptr pNewInstrument = Instrument::load_instrument( g_engine, sDrumkitName, sInstrumentName );
		Engine *pEngine = g_engine;

		// create a new valid ID for this instrument
		int nID = -1;
		for ( uint i = 0; i < pEngine->getSong()->get_instrument_list()->get_size(); ++i ) {
			T<Instrument>::shared_ptr pInstr = pEngine->getSong()->get_instrument_list()->get( i );
			if ( pInstr->get_id().toInt() > nID ) {
				nID = pInstr->get_id().toInt();
			}
		}
		++nID;

		pNewInstrument->set_id( QString("%1").arg( nID ) );

		g_engine->lock( RIGHT_HERE );
		pEngine->getSong()->get_instrument_list()->add( pNewInstrument );

		#ifdef JACK_SUPPORT
		pEngine->renameJackPorts();
		#endif

		g_engine->unlock();

	
		int nTargetInstrument = event->pos().y() / m_nGridHeight;

		/*
		    "X > 181": border between the instrument names on the left and the grid
		    Because the right part of the grid starts above the name column, we have to subtract the difference 
		*/

		if (  event->pos().x() > 181 ) nTargetInstrument = ( event->pos().y() - 90 )  / m_nGridHeight ;

		//move instrument to the position where it was dropped
		moveInstrumentLine(pEngine->getSong()->get_instrument_list()->get_size() - 1 , nTargetInstrument );



		// select the new instrument
		pEngine->setSelectedInstrumentNumber(nTargetInstrument);

		event->acceptProposedAction();
	}
}



void PatternEditorInstrumentList::mousePressEvent(QMouseEvent *event)
{
	if (event->button() == Qt::LeftButton) {
		__drag_start_position = event->pos();
	}

}



void PatternEditorInstrumentList::mouseMoveEvent(QMouseEvent *event)
{
	if (!(event->buttons() & Qt::LeftButton)) {
		return;
	}
	if ( abs(event->pos().y() - __drag_start_position.y()) < (int)m_nGridHeight) {
		return;
	}

	Engine *pEngine = g_engine;
	int nSelectedInstr = pEngine->getSelectedInstrumentNumber();
	T<Instrument>::shared_ptr pInstr = pEngine->getSong()->get_instrument_list()->get(nSelectedInstr);

	QString sText = QString("move instrument:%1").arg( pInstr->get_name() );

	QDrag *pDrag = new QDrag(this);
	QMimeData *pMimeData = new QMimeData;

	pMimeData->setText( sText );
	pDrag->setMimeData( pMimeData);
	//drag->setPixmap(iconPixmap);

	pDrag->start( Qt::CopyAction | Qt::MoveAction );

	// propago l'evento
	QWidget::mouseMoveEvent(event);
}



