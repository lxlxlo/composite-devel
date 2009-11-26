/*
 * Copyright(c) 2002-2004 by Alex >Comix< Cominu [comix@users.sourceforge.net]
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

#ifndef TRITIUM_SMF_H
#define TRITIUM_SMF_H

#include <Tritium/Object.h>
#include <Tritium/Song.h>

#include <string>
#include <cstdio>
#include <vector>

#include <Tritium/smf/SMFEvent.h>

namespace Tritium
{

class SMFHeader : public SMFBase, public Object
{
public:
	SMFHeader( int nFormat, int nTracks, int nTPQN );
	~SMFHeader();

	int m_nFormat;		///< SMF format
	int m_nTracks;		///< number of tracks
	int m_nTPQN;		///< ticks per quarter note

	virtual std::vector<char> getBuffer();
};



class SMFTrack : public SMFBase, public Object
{
public:
	SMFTrack( const QString& sTrackName );
	~SMFTrack();

	void addEvent( SMFEvent *pEvent );

	virtual std::vector<char> getBuffer();

private:
	std::vector<SMFEvent*> m_eventList;
};



class SMF : public SMFBase, public Object
{
public:
	SMF();
	~SMF();

	void addTrack( SMFTrack *pTrack );
	virtual std::vector<char> getBuffer();

private:
	std::vector<SMFTrack*> m_trackList;

	SMFHeader* m_pHeader;

};



class SMFWriter : Object
{
public:
	SMFWriter();
	~SMFWriter();

	void save( const QString& sFilename, Song *pSong );

private:
	FILE *m_file;

};

} // namespace Tritium

#endif // TRITIUM_SMF_H
