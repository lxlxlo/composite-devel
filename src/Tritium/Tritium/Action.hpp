/*
 * Copyright(c) 2002-2008 by Alex >Comix< Cominu [comix@users.sourceforge.net]
 *
 * This file is part of Tritium
 *
 * Tritium is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Tritium is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY, without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */
#ifndef TRITIUM_ACTION_HPP
#define TRITIUM_ACTION_HPP
#include <QString>
#include <QStringList>

namespace Tritium {

    class Action
    {
    public:
	Action( QString );
			
	void setParameter1( QString text ){
	    parameter1 = text;
	}
		
	void setParameter2( QString text ){
	    parameter2 = text;
	}
		
	QString getParameter1(){
	    return parameter1;
	}
		
	QString getParameter2(){
	    return parameter2;
	}

	QString getType(){
	    return type;
	}
		


    private:
	QString type;
	QString parameter1;
	QString parameter2;
    };

    bool setAbsoluteFXLevel( int nLine, int fx_channel , int fx_param);

    class ActionManager
    {
    private:
	QStringList actionList;
	QStringList eventList;

    public:
	bool handleAction( Action * );
		
	QStringList getActionList(){
	    return actionList;
	}
		
	QStringList getEventList(){
	    return eventList;
	}

	ActionManager();
	~ActionManager();
    };

} // namespace Tritium

#endif // TRITIUM_ACTION_HPP
