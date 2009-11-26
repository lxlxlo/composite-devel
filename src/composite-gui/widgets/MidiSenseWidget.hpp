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
#ifndef COMPOSITE_MIDISENSEWIDGET_HPP
#define COMPOSITE_MIDISENSEWIDGET_HPP

#include <QtGui>

#include <Tritium/Object.hpp>

class MidiSenseWidget : public QDialog ,public Object
{
    Q_OBJECT
public:
    MidiSenseWidget(QWidget*);
    ~MidiSenseWidget();

    QString lastMidiEvent;
    int lastMidiEventParameter;

private slots:
    void updateMidi();

private:
    QTimer* m_pUpdateTimer;
    QLabel* m_pURLLabel;
};

#endif // COMPOSITE_MIDISENSEWIDGET_HPP
