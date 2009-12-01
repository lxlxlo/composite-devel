/*
 * Copyright(c) 2009 by Gabriel M. Beddingfield <gabriel@teuton.org>
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
#ifndef TRITIUM_SONGPRIVATE_HPP
#define TRITIUM_SONGPRIVATE_HPP

#include <Tritium/Song.hpp>
#include <QString>
namespace Tritium
{
    class PatternList;
    class PatternModeManager;

    /**
     * \brief Internal data/implementation of Tritium::Song.
     */
    class Song::SongPrivate
    {
    public:
	bool is_muted;
	unsigned resolution;		///< Resolution of the song (number of ticks per quarter)
	float bpm;			///< Beats per minute
	bool is_modified;
	QString name;		///< song name
	QString author;	///< author of the song
	QString license;	///< license of the song

	float volume;						///< volume of the song (0.0..1.0)
	float metronome_volume;				///< Metronome volume
	QString notes;
	PatternList *pattern_list;				///< Pattern list
	Song::pattern_group_t* pattern_group_sequence;	///< Sequence of pattern groups
	InstrumentList *instrument_list;			///< Instrument list
	QString filename;
	bool is_loop_enabled;
	float humanize_time_value;
	float humanize_velocity_value;
	float swing_factor;

	SongMode song_mode;

	PatternModeManager* pat_mode;

	SongPrivate(const QString& name,
		    const QString& author,
		    float bpm,
		    float volumne);
	~SongPrivate();
    };

} // namespace Tritium

#endif // TRITIUM_SONGPRIVATE_HPP
