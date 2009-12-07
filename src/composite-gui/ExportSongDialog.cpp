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

#include <QFileDialog>
#include <QProgressBar>
#include <QLabel>
#include <QPixmap>

#include "ExportSongDialog.hpp"
#include "Skin.hpp"
#include "CompositeApp.hpp"
#include <Tritium/Song.hpp>
#include <Tritium/Engine.hpp>
#include <Tritium/IO/AudioOutput.hpp>

#include <memory>

using namespace Tritium;

ExportSongDialog::ExportSongDialog(QWidget* parent)
 : QDialog(parent)
 , m_bExporting( false )
{
	setupUi( this );
	setModal( true );
	setWindowTitle( trUtf8( "Export song" ) );
//	setIcon( QPixmap( Skin::getImagePath() + "/icon16.png" ) );

	CompositeApp::get_instance()->addEventListener( this );

	m_pSamplerateLbl->setText( trUtf8( "Sample rate: %1" ).arg( g_engine->get_audio_output()->getSampleRate() ) );
	m_pProgressBar->setValue( 0 );
}



ExportSongDialog::~ExportSongDialog()
{
	CompositeApp::get_instance()->removeEventListener( this );
}



/// \todo: memorizzare l'ultima directory usata
void ExportSongDialog::on_browseBtn_clicked()
{
//	static QString lastUsedDir = "";
	static QString lastUsedDir = QDir::homePath();

	std::auto_ptr<QFileDialog> fd( new QFileDialog );
	fd->setFileMode(QFileDialog::AnyFile);
	fd->setFilter( trUtf8("Wave file (*.wav)") );
	fd->setDirectory( lastUsedDir );
	fd->setAcceptMode( QFileDialog::AcceptSave );
	fd->setWindowTitle( trUtf8( "Export song" ) );
//	fd->setIcon( QPixmap( Skin::getImagePath() + "/icon16.png" ) );

	QString defaultFilename( g_engine->getSong()->get_name() );
	defaultFilename.replace( '*', "_" );
	defaultFilename += ".wav";

	fd->selectFile(defaultFilename);

	QString filename = "";
	if (fd->exec()) {
		filename = fd->selectedFiles().first();
	}

	if ( ! filename.isEmpty() ) {
		lastUsedDir = fd->directory().absolutePath();
		QString sNewFilename = filename;
		if ( sNewFilename.endsWith( ".wav" ) == false ) {
			filename += ".wav";
		}

		exportNameTxt->setText(filename);
	}
}



void ExportSongDialog::on_okBtn_clicked()
{
	if (m_bExporting) {
		return;
	}

	QString filename = exportNameTxt->text();
	m_bExporting = true;
	g_engine->startExportSong( filename );
}



void ExportSongDialog::on_closeBtn_clicked()
{
	g_engine->stopExportSong();
	m_bExporting = false;
	accept();

}


void ExportSongDialog::on_exportNameTxt_textChanged( const QString& )
{
	QString filename = exportNameTxt->text();
	if ( ! filename.isEmpty() ) {
		okBtn->setEnabled(true);
	}
	else {
		okBtn->setEnabled(false);
	}
}


void ExportSongDialog::progressEvent( int nValue )
{
        m_pProgressBar->setValue( nValue );
	if ( nValue == 100 ) {
	  	//INFOLOG("SONO A 100");
		
		g_engine->stopExportSong();
		m_bExporting = false;
		QFile check( exportNameTxt->text() );
		if ( ! check.exists() ) {
			QMessageBox::information( this, "Composite", trUtf8("Export failed!") );
		}
		accept();
	}
}
