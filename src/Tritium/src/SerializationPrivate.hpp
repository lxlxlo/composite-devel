/*
 * Copyright(c) 2009 by Gabriel M. Beddingfield <gabriel@teuton.org>
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
#ifndef TRITIUM_SERIALIZATIONPRIVATE_HPP
#define TRITIUM_SERIALIZATIONPRIVATE_HPP

#include <QDomNode>
#include <QString>
#include <Tritium/memory.hpp>

namespace Tritium
{
    class Song;
    class Pattern;
    class InstrumentList;
    class Engine;

    /**
     * \brief Load a Hydrogen .h2song into memory.
     */
    class SongReader
    {
    public:
	SongReader();
	~SongReader();
	T<Song>::shared_ptr readSong( Engine* engine, const QString& filename );

    private:
	QString m_sSongVersion;

	/// Dato un XmlNode restituisce un oggetto Pattern
	T<Pattern>::shared_ptr getPattern( QDomNode pattern, InstrumentList* instrList );
    };

} // namespace Tritium

#endif // TRITIUM_SERIALIZATIONPRIVATE_HPP
