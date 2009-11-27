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

#ifndef COMPOSITE_SOUNDLIBRARYEXPORTDIALOG_HPP
#define COMPOSITE_SOUNDLIBRARYEXPORTDIALOG_HPP

#include "config.h"
#include "ui_SoundLibraryExportDialog_UI.h"

#include <Tritium/Logger.hpp>
#include <Tritium/Song.hpp>
#include <Tritium/SoundLibrary.hpp>

#include <vector>

///
///
///
class SoundLibraryExportDialog : public QDialog, public Ui_SoundLibraryExportDialog_UI, public Object
{
	Q_OBJECT
	public:
		SoundLibraryExportDialog( QWidget* pParent );
		~SoundLibraryExportDialog();

	private slots:
			void on_exportBtn_clicked();
			void on_browseBtn_clicked();
			void on_drumkitPathTxt_textChanged( QString str );
			void updateDrumkitList();
	private:
			std::vector<Tritium::Drumkit*> drumkitInfoList;
};


#endif // COMPOSITE_SOUNDLIBRARYEXPORTDIALOG_HPP
