/*
 * Hydrogen
 * Copyright(c) 2002-2007 by Alex >Comix< Cominu [comix@users.sourceforge.net]
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

#include "SoundLibraryRepositoryDialog.h"

#include <hydrogen/SoundLibrary.h>
#include <hydrogen/Preferences.h>

#include <QList>
#include <QInputDialog>
#include <QListWidgetItem>

SoundLibraryRepositoryDialog::SoundLibraryRepositoryDialog( QWidget* pParent )
 : QDialog( pParent )
 , Object( "SoundLibraryRepositoryDialog" )
{
	setupUi( this );
	INFOLOG( "INIT" );
	setWindowTitle( trUtf8( "Edit repository settings" ) );
	setFixedSize( width(), height() );

	updateDialog();

}


//update all values
void SoundLibraryRepositoryDialog::updateDialog(){
	
	H2Core::Preferences *pPref = H2Core::Preferences::getInstance();

	/*
		Read serverList from config and put servers into the serverList
	*/
	
	std::list<std::string>::const_iterator cur_Server;

	ServerListWidget->clear();
	
	for( cur_Server = pPref->sServerList.begin(); cur_Server != pPref->sServerList.end(); ++cur_Server )
	{
		ServerListWidget->addItem(QString(cur_Server->c_str()));
	}
}



///
/// Add new server url

void SoundLibraryRepositoryDialog::on_AddBtn_clicked()
{
	H2Core::Preferences *pPref = H2Core::Preferences::getInstance();
	bool ok;

	QString text = QInputDialog::getText(this, trUtf8("Edit server list"), trUtf8("URL"), QLineEdit::Normal,QString(""), &ok);
	
	if( ok and !text.isEmpty() ){
		pPref->sServerList.push_back(text.toStdString());
	}

	updateDialog();
}

///
/// Delete serverList entry
///
void SoundLibraryRepositoryDialog::on_DeleteBtn_clicked()
{
	QList<QListWidgetItem *> selectedItems;
	selectedItems = ServerListWidget->selectedItems();

	std::list<std::string>::const_iterator cur_Server;
	H2Core::Preferences *pPref = H2Core::Preferences::getInstance();

	while( ! selectedItems.isEmpty() ){

		QString selText;
	
		selText = selectedItems.takeFirst()->text();

		pPref->sServerList.erase(std::remove(pPref->sServerList.begin(), pPref->sServerList.end(), selText.toStdString()),pPref->sServerList.end());
	}
	updateDialog();
}

SoundLibraryRepositoryDialog::~SoundLibraryRepositoryDialog()
{
	INFOLOG( "DESTROY" );

}