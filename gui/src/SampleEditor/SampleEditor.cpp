/*
 * Hydrogen
 * Copyright(c) 2002-2008 by Alex >Comix< Cominu [comix@users.sourceforge.net]
 *
 * http://www.hydrogen-music.org
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY, without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include "SampleEditor.h"
#include "../HydrogenApp.h"
#include "InstrumentEditor/InstrumentEditor.h"
#include "InstrumentEditor/InstrumentEditorPanel.h"
#include "../widgets/Button.h"

#include "MainSampleWaveDisplay.h"
#include "DetailWaveDisplay.h"
#include "TargetWaveDisplay.h"

#include <hydrogen/data_path.h>
#include <hydrogen/h2_exception.h>
#include <hydrogen/Preferences.h>
#include <hydrogen/sample.h>
#include <hydrogen/audio_engine.h>
#include <hydrogen/hydrogen.h>

#include <QModelIndex>
#include <QTreeWidget>
#include <QMessageBox>

using namespace H2Core;
using namespace std;

SampleEditor::SampleEditor ( QWidget* pParent, Sample* Sample )
		: QDialog ( pParent )
		, Object ( "SampleEditor" )
{
	setupUi ( this );
	INFOLOG ( "INIT" );
	setWindowTitle ( trUtf8 ( "SampleEditor" ) );
	setFixedSize ( width(), height() );
	installEventFilter( this );
	m_pSampleEditorStatus = true; //set true if sample changes are save
	m_pSample = Sample;

//get all sample modificationen 
	m_sample_is_modified = m_pSample->get_sample_is_modified();
	m_sample_mode = m_pSample->get_sample_mode();
	m_start_frame = m_pSample->get_start_frame();
	m_loop_frame = m_pSample->get_loop_frame();
	m_repeats = m_pSample->get_repeats();
	m_end_frame = m_pSample->get_end_frame();
	m_fade_out_startframe = m_pSample->get_fade_out_startframe();
	m_fade_out_type = m_pSample->get_fade_out_type();

	m_ponewayStart = false;
	m_ponewayLoop = false;
	m_ponewayEnd = false;
	unsigned slframes = m_pSample->get_n_frames();
	m_pzoomfactor = 1;
	m_pdetailframe = 0;
	m_plineColor = "default";

	QApplication::setOverrideCursor(Qt::WaitCursor);
// wavedisplays
	m_divider = m_pSample->get_n_frames() / 574.0F;
	m_pMainSampleWaveDisplay = new MainSampleWaveDisplay( mainSampleview );
	m_pMainSampleWaveDisplay->updateDisplay( Sample->get_filename() );
	m_pMainSampleWaveDisplay->move( 1, 1 );

	m_pSampleAdjustView = new DetailWaveDisplay( mainSampleAdjustView );
	m_pSampleAdjustView->updateDisplay( Sample->get_filename() );
	m_pSampleAdjustView->move( 1, 1 );

	float *pSampleData = Sample->get_data_l();
	m_pTargetSampleView = new TargetWaveDisplay( targetSampleView );
	m_pTargetSampleView->updateDisplay( pSampleData, slframes );
	m_pTargetSampleView->move( 1, 1 );


	QApplication::restoreOverrideCursor();


	StartFrameSpinBox->setRange(0, slframes );
	LoopFrameSpinBox->setRange(0, slframes );
	EndFrameSpinBox->setRange(0, slframes );
	if ( !m_pSample->get_sample_is_modified() ){
		EndFrameSpinBox->setValue( slframes ); 
	}else
	{
		EndFrameSpinBox->setValue( m_end_frame );
	}

// mainSampleview = 624(575) x 265 :-)
// mainSampleAdjustView = 180 x 265 :-(
// targetSampleView = 451 x 91 :-(
// StartFrameSpinBox :-)
// LoopFrameSpinBox :-)
// ProcessingTypeComboBox :forward, reverse, pingpong :-(
// LoopCountSpinBox :-(
// EndFrameSpinBox :-)
// FadeOutFrameSpinBox :-(
// FadeOutTypeComboBox: lin, log :-(
// ApplyChangesPushButton :-()
// PlayPushButton :-(
// RestoreSamplePushButton :-(
// ClosePushButton :-()
// verticalzoomSlider

	connect( StartFrameSpinBox, SIGNAL( valueChanged( int ) ), this, SLOT( valueChangedStartFrameSpinBox(int) ) );
	connect( LoopFrameSpinBox, SIGNAL( valueChanged( int ) ), this, SLOT( valueChangedLoopFrameSpinBox(int) ) );
	connect( EndFrameSpinBox, SIGNAL( valueChanged( int ) ), this, SLOT( valueChangedEndFrameSpinBox(int) ) );

}



SampleEditor::~SampleEditor()
{
	delete m_pMainSampleWaveDisplay;
	delete m_pSampleAdjustView;
	delete m_pTargetSampleView;
	INFOLOG ( "DESTROY" );
}



void SampleEditor::on_ClosePushButton_clicked()
{
	if ( !m_pSampleEditorStatus ){
		int err = QMessageBox::information( this, "Hydrogen", tr( "Unsaved changes left. This changes will be lost. \nAre you sure?"), tr("&Ok"), tr("&Cancel"), 0, 1 );
		if ( err == 0 ){
			m_pSampleEditorStatus = true;
			accept();	
		}else
		{
			return;
		}
	}
	accept();
}



void SampleEditor::on_ApplyChangesPushButton_clicked()
{
	setAllSampleProps();	
	m_pSample->sampleEditProzess( m_pSample );
	m_pSampleEditorStatus = true;
	m_pTargetSampleView->reloadDisplay();
}



bool SampleEditor::getCloseQuestion()
{
	bool close = false;
	int err = QMessageBox::information( this, "Hydrogen", tr( "Close dialog! maybe there is some unsaved work on sample.\nAre you sure?"), tr("&Ok"), tr("&Cancel"), 0, 1 );
	if ( err == 0 ) close = true;
	return close;
}



void SampleEditor::setSampleName( QString name )
{
	QString newfilename = name.section( '/', -1 );
//	newfilename.replace( "." + newfilename.section( '.', -1 ), "");

	QString windowname = "SampleEditor " + newfilename;
	m_samplename = name;
	setWindowTitle ( windowname );
}



void SampleEditor::setAllSampleProps()
{
	if ( !m_pSampleEditorStatus ){
		m_pSample->set_sample_is_modified( m_sample_is_modified );
		m_pSample->set_sample_mode( m_sample_mode );
		m_pSample->set_start_frame( m_start_frame );
		m_pSample->set_loop_frame( m_loop_frame );
		m_pSample->set_repeats( m_repeats );
		m_pSample->set_end_frame( m_end_frame );
		m_pSample->set_fade_out_startframe( m_fade_out_startframe );
		m_pSample->set_fade_out_type( m_fade_out_type );
	}
}



void SampleEditor::mouseReleaseEvent(QMouseEvent *ev)
{

}



void SampleEditor::returnAllMainWaveDisplayValues()
{
//	QMessageBox::information ( this, "Hydrogen", trUtf8 ( "jep %1" ).arg(m_pSample->get_n_frames()));
	m_sample_is_modified = true;
	m_start_frame = m_pMainSampleWaveDisplay->m_pStartFramePosition * m_divider - 25 * m_divider;
	m_loop_frame = m_pMainSampleWaveDisplay->m_pLoopFramePosition  * m_divider - 25 * m_divider;
	m_end_frame = m_pMainSampleWaveDisplay->m_pEndFramePosition  * m_divider - 25 * m_divider ;

	StartFrameSpinBox->setValue( m_start_frame );
	LoopFrameSpinBox->setValue( m_loop_frame );
	EndFrameSpinBox->setValue( m_end_frame );
	m_ponewayStart = true;	
	m_ponewayLoop = true;
	m_ponewayEnd = true;
}



void SampleEditor::valueChangedStartFrameSpinBox( int )
{
	m_pdetailframe = StartFrameSpinBox->value();
	m_plineColor = "Start";
	if ( !m_ponewayStart ){
		m_pMainSampleWaveDisplay->m_pStartFramePosition = StartFrameSpinBox->value() / m_divider + 25 ;
		m_pMainSampleWaveDisplay->updateDisplayPointer();
		m_pSampleAdjustView->setDetailSamplePosition( m_pdetailframe, m_pzoomfactor , m_plineColor);
				
	}else
	{
		m_pSampleAdjustView->setDetailSamplePosition( m_pdetailframe, m_pzoomfactor , m_plineColor);
		m_ponewayStart = false;
	}
	//QMessageBox::information ( this, "Hydrogen", trUtf8 ( "jep %1" ).arg(StartFrameSpinBox->value() / m_divider + 25 ));
}



void SampleEditor::valueChangedLoopFrameSpinBox( int )
{	
	m_pdetailframe = LoopFrameSpinBox->value();
	m_plineColor = "Loop";
	if ( !m_ponewayLoop ){
		m_pMainSampleWaveDisplay->m_pLoopFramePosition = LoopFrameSpinBox->value() / m_divider + 25 ;
		m_pMainSampleWaveDisplay->updateDisplayPointer();
		m_pSampleAdjustView->setDetailSamplePosition( m_pdetailframe, m_pzoomfactor , m_plineColor);
	}else
	{
		m_pSampleAdjustView->setDetailSamplePosition( m_pdetailframe, m_pzoomfactor , m_plineColor);
		m_ponewayLoop = false;
	}
}



void SampleEditor::valueChangedEndFrameSpinBox( int )
{
	m_pdetailframe = EndFrameSpinBox->value();
	m_plineColor = "End";
	if ( !m_ponewayEnd ){
		m_pMainSampleWaveDisplay->m_pEndFramePosition = EndFrameSpinBox->value() / m_divider + 25 ;
		m_pMainSampleWaveDisplay->updateDisplayPointer();
		m_pSampleAdjustView->setDetailSamplePosition( m_pdetailframe, m_pzoomfactor , m_plineColor);
	}else
	{
		m_ponewayEnd = false;
		m_pSampleAdjustView->setDetailSamplePosition( m_pdetailframe, m_pzoomfactor , m_plineColor);
	}
}



void SampleEditor::on_verticalzoomSlider_valueChanged( int value )
{
	m_pzoomfactor = value / 10 +1;
	m_pSampleAdjustView->setDetailSamplePosition( m_pdetailframe, m_pzoomfactor, m_plineColor );
}



void SampleEditor::on_PlayPushButton_clicked()
{
		const int selectedlayer = InstrumentEditorPanel::getInstance()->getselectedLayer();
		const float pan_L = 0.5f;
		const float pan_R = 0.5f;
		const int nLength = -1;
		const float fPitch = 0.0f;
		Song *pSong = Hydrogen::get_instance()->getSong();
		
		Instrument *pInstr = pSong->get_instrument_list()->get( Hydrogen::get_instance()->getSelectedInstrumentNumber() );
		
		Note *pNote = new Note( pInstr, 0, pInstr->get_layer( selectedlayer )->get_end_velocity() - 0.01, pan_L, pan_R, nLength, fPitch);
		AudioEngine::get_instance()->get_sampler()->note_on(pNote);	
}
