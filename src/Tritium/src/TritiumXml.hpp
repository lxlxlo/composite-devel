/*
 * Copyright (c) 2010 by Gabriel M. Beddingfield <gabriel@teuton.org>
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
#ifndef TRITIUM_TRITIUMXML_HPP
#define TRITIUM_TRITIUMXML_HPP

#include <Tritium/ObjectBundle.hpp>
#include <Tritium/memory.hpp>

class QIODevice;
class QString;
class QDomDocument;
class QDomElement;
class QDomNode;

namespace Tritium
{
    class Presets;

    namespace Serialization
    {
	class TritiumXml : public ObjectBundle
	{
	protected:
	    /* These two make the code a little more readable.
	     */
	    bool& _error;
	    QString& _error_message;

	public:
	    TritiumXml() :
		ObjectBundle(),
		_error(ObjectBundle::error),
		_error_message(ObjectBundle::error_message)
		{}

	    ~TritiumXml() {}

	    bool setContent( QDomDocument& doc );
	    bool setContent( QIODevice *dev );
	    bool setContent( const QString& text );

	    void clear() {
		_error = false;
		_error_message = "";
		while( ! empty() ) {
		    pop();
		}
	    }

	    bool error() {
		return _error;
	    }

	    const QString& error_message() {
		return _error_message;
	    }

	protected:
	    bool read_tritium_node(QDomElement& tritium);
	    bool read_presets_node(QDomElement& presets);

	    /* Validation methods are in lieu of having direct parser
	     * support for validating schemas.
	     */
	    static bool validate_tritium_node(QDomElement& tritium, QString *err_msg);
	    static bool validate_presets_node(QDomElement& presets, QString *err_msg);
	    static bool validate_bank_node(QDomElement& bank, QString *err_msg);
	    static bool validate_program_node(QDomElement& program, QString *err_msg);

	    static bool validate_midi_integer_type(const QString& value,
						   const QString& name,
						   bool optional,
						   QString *err_msg);

	};

    } // namespace Serialization
} // namespace Tritium

#endif // TRITIUM_TRITIUMXML_HPP
