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

#ifndef COMPOSITE_LADSPAFXSELECTOR_HPP
#define COMPOSITE_LADSPAFXSELECTOR_HPP

#include "config.h"

#include "ui_LadspaFXSelector_UI.h"

#include <Tritium/Object.hpp>

#include <QtGui>
#include <string>
#include <vector>

namespace Tritium {
	class LadspaFXInfo;
	class LadspaFXGroup;
}

class LadspaFXSelector : public QDialog, public Ui_LadspaFXSelector_UI, public Object
{
	Q_OBJECT

	public:
		LadspaFXSelector(int nLadspaFX);
		~LadspaFXSelector();

		QString getSelectedFX();

	private slots:
		void on_m_pGroupsListView_currentItemChanged( QTreeWidgetItem * current, QTreeWidgetItem * previous );
		void pluginSelected();

	private:
		QTreeWidgetItem* m_pCurrentItem;
		QString m_sSelectedPluginName;
		void buildLadspaGroups();

#ifdef LADSPA_SUPPORT
		void addGroup(QTreeWidgetItem *parent, Tritium::LadspaFXGroup *pGroup);
		void addGroup( QTreeWidget *parent, Tritium::LadspaFXGroup *pGroup );
		void buildGroup(QTreeWidgetItem *pNewItem, Tritium::LadspaFXGroup *pGroup);

		std::vector<Tritium::LadspaFXInfo*> findPluginsInGroup( const QString& sSelectedGroup, Tritium::LadspaFXGroup *pGroup );
#endif

};


#endif // COMPOSITE_LADSPAFXSELECTOR_HPP
