/*
 * Copyright(c) 2002-2007 by Alex >Comix< Cominu [comix@users.sourceforge.net]
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

#include "SoundLibraryRepositoryDialog.hpp"

#include <Tritium/SoundLibrary.hpp>
#include <Tritium/Preferences.hpp>
#include <Tritium/Logger.hpp>
#include <Tritium/Engine.hpp>
#include <Tritium/memory.hpp>

#include <QList>
#include <QInputDialog>
#include <QListWidgetItem>

SoundLibraryRepositoryDialog::SoundLibraryRepositoryDialog( QWidget* pParent )
 : QDialog( pParent )
{
	setupUi( this );
	DEBUGLOG( "INIT" );
	setWindowTitle( trUtf8( "Edit repository settings" ) );
	setFixedSize( width(), height() );

	updateDialog();

}


//update all values
void SoundLibraryRepositoryDialog::updateDialog(){
	
	Tritium::T<Tritium::Preferences>::shared_ptr pPref = Tritium::g_engine->get_preferences();

	/*
		Read serverList from config and put servers into the serverList
	*/
	
	std::list<QString>::const_iterator cur_Server;

	ServerListWidget->clear();
	
	for( cur_Server = pPref->sServerList.begin(); cur_Server != pPref->sServerList.end(); ++cur_Server )
	{
		ServerListWidget->addItem( *cur_Server );
	}
}



///
/// Add new server url

void SoundLibraryRepositoryDialog::on_AddBtn_clicked()
{
	Tritium::T<Tritium::Preferences>::shared_ptr pPref = Tritium::g_engine->get_preferences();
	bool ok;

	QString text = QInputDialog::getText(this, trUtf8("Edit server list"), trUtf8("URL"), QLineEdit::Normal,QString(""), &ok);
	
	if( ok && !text.isEmpty() ){
		pPref->sServerList.push_back( text );
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

	//std::list<std::string>::const_iterator cur_Server;
	Tritium::T<Tritium::Preferences>::shared_ptr pPref = Tritium::g_engine->get_preferences();

	while( ! selectedItems.isEmpty() ){

		QString selText;
	
		selText = selectedItems.takeFirst()->text();

		pPref->sServerList.remove(selText);

	}
	updateDialog();
}

SoundLibraryRepositoryDialog::~SoundLibraryRepositoryDialog()
{
	DEBUGLOG( "DESTROY" );

}
