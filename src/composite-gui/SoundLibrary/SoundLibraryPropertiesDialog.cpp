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

#include <QtGui>

#include "../HydrogenApp.hpp"
#include "SoundLibraryPropertiesDialog.hpp"
#include "../InstrumentRack.hpp"
#include "SoundLibraryPanel.hpp"
#include <Tritium/SoundLibrary.hpp>
#include <Tritium/Engine.hpp>
#include <Tritium/Logger.hpp>

namespace Tritium
{

//globals
Drumkit *drumkitinfo = NULL ;
Drumkit *predrumkit = NULL;
QString oldName;

SoundLibraryPropertiesDialog::SoundLibraryPropertiesDialog( QWidget* pParent, Drumkit *drumkitInfo, Drumkit *preDrumKit )
 : QDialog( pParent )
{
	setupUi( this );
	INFOLOG( "INIT" );
	setWindowTitle( trUtf8( "SoundLibrary Properties" ) );	
	setFixedSize( width(), height() );
	predrumkit = preDrumKit;

	//display the current drumkit infos into the qlineedit
	if ( drumkitInfo != NULL ){
		drumkitinfo = drumkitInfo;
		nameTxt->setText( QString( drumkitInfo->getName() ) );
		oldName = drumkitInfo->getName();
		authorTxt->setText( QString( drumkitInfo->getAuthor() ) );
		infoTxt->append( QString( drumkitInfo->getInfo() ) );
		licenseTxt->setText( QString( drumkitInfo->getLicense() ) );
	}

}




SoundLibraryPropertiesDialog::~SoundLibraryPropertiesDialog()
{
	INFOLOG( "DESTROY" );

}



void SoundLibraryPropertiesDialog::on_saveBtn_clicked()
{
	
	bool reload = false;
	 
	if ( saveChanges_checkBox->isChecked() ){
		//test if the drumkit is loaded 
		if ( Engine::get_instance()->getCurrentDrumkitname() != drumkitinfo->getName() ){
			QMessageBox::information( this, "Composite", trUtf8 ( "This is not possible, you can only save changes inside instruments to the current loaded sound library"));
			saveChanges_checkBox->setChecked( false );
			return;
		}
		reload = true;
	}
	
	//load the selected drumkit to save it correct.... later the old drumkit will be reloaded 
	if ( drumkitinfo != NULL && ( !saveChanges_checkBox->isChecked() ) ){
		if ( Engine::get_instance()->getCurrentDrumkitname() != drumkitinfo->getName() ){
			Engine::get_instance()->loadDrumkit( drumkitinfo );
			Engine::get_instance()->getSong()->set_modified( true );
		}
	}
		
	//check the drumkit name. if the name is a new one, one qmessagebox with question "are you sure" will displayed.
	if ( nameTxt->text() != oldName  ){
		int res = QMessageBox::information( this, "Composite", tr( "Warning! Changing the drumkit name will result in creating a new drumkit with this name.\nAre you sure?"), tr("&Ok"), tr("&Cancel"), 0, 1 );
		if ( res == 1 ) {
			return;
		}
		else
		{	
			reload = true;
		}
	}

	//save the drumkit	
	Tritium::Drumkit::save(
			nameTxt->text(),
			authorTxt->text(),
			infoTxt->toHtml(),
			licenseTxt->text()
	);
	
	//check the name and set the drumkitinfo to current drumkit
	if ( drumkitinfo != NULL && !nameTxt->text().isEmpty() ){
		drumkitinfo->setName( nameTxt->text() );
		drumkitinfo->setAuthor( authorTxt->text() );
		drumkitinfo->setInfo( infoTxt->toHtml() );
		drumkitinfo->setLicense( licenseTxt->text() );
	}
	

	//check pre loaded drumkit name  and reload the old drumkit 
	if ( predrumkit != NULL ){
		if ( predrumkit->getName() !=  Engine::get_instance()->getCurrentDrumkitname() ){
			Engine::get_instance()->loadDrumkit( predrumkit );
			Engine::get_instance()->getSong()->set_modified( true );
		}
	}

	//reload if necessary
	if ( reload == true ){
		HydrogenApp::get_instance()->getInstrumentRack()->getSoundLibraryPanel()->test_expandedItems();
		HydrogenApp::get_instance()->getInstrumentRack()->getSoundLibraryPanel()->updateDrumkitList();
	}

	accept();
	
}

}
