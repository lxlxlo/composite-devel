/*
 * Copyright(c) 2010 by Gabriel M. Beddingfield <gabriel@teuton.org>
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

#include <Composite/Looks/Colors.hpp>
#include <QtGui/QPalette>
#include <QtGui/QApplication>

namespace Composite
{
namespace Looks
{

    QPalette create_default_palette()
    {
	QColor white( 0xff, 0xff, 0xff, 0xff );

	// Shades of navy...

	QColor navy_light(     0x00, 0x00, 0xa0, 0xff );
	QColor navy_mid_light( 0x00, 0x00, 0x90, 0xff );
	QColor navy(           0x00, 0x00, 0x70, 0xff );
	QColor navy_mid_dark(  0x00, 0x00, 0x90, 0xff );
	QColor navy_dark(      0x00, 0x00, 0xb0, 0xff );


	QColor windowText;
	QColor button;
	QColor light;
	QColor dark;
	QColor mid;
	QColor text;
	QColor bright_text;
	QColor base;
	QColor window;

	// Active colors                Qt's description
	windowText =  white;         // Text on top of 'window' color.
	button =      navy_mid_light;// Color for a button.
	light =       navy_light;    // lighter than 'button'
	dark =        dark;          // Darker than 'button'
	mid =         navy_mid_dark; // between button and dark
	text =        white;         // foreground color for 'base'
	bright_text = white;         // diff from windowText and contrasts with dark
	base =        navy_light;    // e.g. background for text entry
	window =      navy;          // Gen'l purpose background color

	// Get a head start...
	QPalette pal = QApplication::palette();

	pal.setColorGroup( QPalette::Active,
			   windowText,
			   button,
			   light,
			   dark,
			   mid,
			   text,
			   bright_text,
			   base,
			   window
	    );

	pal.setColorGroup( QPalette::Disabled,
			   windowText,
			   button,
			   light,
			   dark,
			   mid,
			   text,
			   bright_text,
			   base,
			   window
	    );

	pal.setColorGroup( QPalette::Inactive,
			   windowText,
			   button,
			   light,
			   dark,
			   mid,
			   text,
			   bright_text,
			   base,
			   window
	    );

	pal.setColorGroup( QPalette::Normal,
			   windowText,
			   button,
			   light,
			   dark,
			   mid,
			   text,
			   bright_text,
			   base,
			   window
	    );

	return pal;
    }

} // namespace Looks
} // namespace Composite
