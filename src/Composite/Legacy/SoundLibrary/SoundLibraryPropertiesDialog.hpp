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

#ifndef COMPOSITE_SOUNDLIBRARYPROPERTIESDIALOG_HPP
#define COMPOSITE_SOUNDLIBRARYPROPERTIESDIALOG_HPP

#include "config.h"
#include "ui_SoundLibraryPropertiesDialog_UI.h"
#include <Tritium/memory.hpp>

///
///
namespace Tritium
{

class Drumkit;

class SoundLibraryPropertiesDialog : public QDialog, public Ui_SoundLibraryPropertiesDialog_UI
{
	Q_OBJECT
	public:
		SoundLibraryPropertiesDialog( QWidget* pParent , T<Drumkit>::shared_ptr drumkitInfo, T<Drumkit>::shared_ptr preDrumKit );
		~SoundLibraryPropertiesDialog();

	private slots:
			void on_saveBtn_clicked();

	private:
};

}
#endif // COMPOSITE_SOUNDLIBRARYPROPERTIESDIALOG_HPP
