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

#include "TritiumXmlReader.hpp"
#include <Tritium/Presets.hpp>
#include <cassert>
#include <QtXml>
#include <QString>

namespace Tritium
{
    namespace Serialization
    {

	bool TritiumXmlReader::setContent(QIODevice *dev)
	{
	    _error = false;
	    _error_message = "";

	    QDomDocument doc;
	    QString errMsg;
	    int errLine, errCol;
	    bool rv;
	    rv = doc.setContent(dev, true, &errMsg, &errLine, &errCol);
	    if( rv == false ) {
		_error = true;
		_error_message = QString("L%1 C%2: %3")
		    .arg(errLine)
		    .arg(errCol)
		    .arg(errMsg);
		return false;
	    }

	    return setContent(doc);
	}

	bool TritiumXmlReader::setContent(const QString& text)
	{
	    _error = false;
	    _error_message = "";

	    QDomDocument doc;
	    QString errMsg;
	    int errLine, errCol;
	    bool rv;
	    rv = doc.setContent(text, true, &errMsg, &errLine, &errCol);
	    if( rv == false ) {
		_error = true;
		_error_message = QString("L%1 C%2: %3")
		    .arg(errLine)
		    .arg(errCol)
		    .arg(errMsg);
		return false;
	    }

	    return setContent(doc);
	}

	bool TritiumXmlReader::setContent( QDomDocument& doc )
	{
	    QDomElement root = doc.documentElement();
	    if((root.namespaceURI() != TRITIUM_XML)
	       && (root.namespaceURI() != "")) {
		_error = true;
		_error_message = QString("File has incorrect XML namespace '%1'")
		    .arg(root.namespaceURI());
		return false;
	    }

	    if(root.tagName() == "tritium") {
		return handle_load_tritium_node(root);
	    } else if (root.tagName() == "presets") {
		return handle_load_presets_node(root);
	    } else {
		_error = true;
		_error_message = QString("Invalid root document element '%1'")
		    .arg(root.tagName());
	    }
	    return false;
	}

	/*============================================================
	 * VALIDATION METHODS
	 *============================================================
	 */

	static bool xml_namespace_check(QDomElement& e, QString *err_msg)
	{
	    if( (e.namespaceURI() != TRITIUM_XML)
		&& (e.namespaceURI() != "") ) {
		if(err_msg) {
		    (*err_msg) = QString("Invalid namespace for element '%1',"
					 " should be '%2'")
			.arg(e.tagName())
			.arg(TRITIUM_XML);
		}
		return false;
	    }
	    return true;
	}

	bool TritiumXmlReader::validate_tritium_node(QDomElement& tritium, QString *err_msg)
	{
	    assert(tritium.tagName() == "tritium");

	    bool rv;
	    rv = xml_namespace_check(tritium, err_msg);
	    if( !rv ) return false;

	    QDomElement e = tritium.firstChildElement();
	    for( ; ! e.isNull() ; e = e.nextSiblingElement() ) {
		if( e.namespaceURI() != tritium.namespaceURI() )
		    continue;

		rv = true;
		if(e.tagName() == "presets") {
		    rv = validate_presets_node(e, err_msg);
		}

		if( !rv ) break;
	    }
	    return rv;
	}

	bool TritiumXmlReader::validate_presets_node(QDomElement& presets, QString *err_msg)
	{
	    assert(presets.tagName() == "presets");

	    bool rv;
	    rv = xml_namespace_check(presets, err_msg);
	    if( !rv ) return false;

	    QDomElement e = presets.firstChildElement();
	    for( ; ! e.isNull() ; e = e.nextSiblingElement() ) {
		if( e.namespaceURI() != presets.namespaceURI() )
		    continue;

		rv = true;
		if(e.tagName() == "bank") {
		    rv = validate_bank_node(e, err_msg);
		}

		if( !rv ) break;
	    }
	    return rv;
	}

	bool TritiumXmlReader::validate_bank_node(QDomElement& bank, QString *err_msg)
	{
	    assert(bank.tagName() == "bank");

	    bool rv;
	    rv = xml_namespace_check(bank, err_msg);
	    if( !rv ) return false;

	    QDomAttr att = bank.attributeNode("coarse");
	    rv = validate_midi_integer_type(att.nodeValue(), "coarse", true, err_msg);
	    if( !rv ) return false;
	    att = bank.attributeNode("fine");
	    rv = validate_midi_integer_type(att.nodeValue(), "fine", true, err_msg);
	    if( !rv ) return false;

	    QDomElement e = bank.firstChildElement();
	    for( ; ! e.isNull() ; e = e.nextSiblingElement() ) {
		if( e.namespaceURI() != bank.namespaceURI() )
		    continue;

		rv = true;
		if(e.tagName() == "program") {
		    rv = validate_program_node(e, err_msg);
		}

		if( !rv ) break;
	    }
	    return rv;
	}

	bool TritiumXmlReader::validate_program_node(QDomElement& program, QString *err_msg)
	{
	    assert(program.tagName() == "program");

	    bool rv;
	    rv = xml_namespace_check(program, err_msg);
	    if( !rv ) return false;

	    QDomElement e;
	    e = program.firstChildElement();
	    rv = xml_namespace_check(e, err_msg);
	    if( !rv ) return false;
	    if( e.tagName() != "midi_number" ) {
		rv = false;
		if(err_msg) {
		    (*err_msg) = QString("Invalid <program> node.  "
					 "Expected <midi_number>, got <%1>")
			.arg(e.tagName());
		}
		return rv;
	    }
	    rv = validate_midi_integer_type(e.text(), "midi_number", false, err_msg);
	    if( !rv ) return false;

	    e = e.nextSiblingElement();
	    rv = xml_namespace_check(e, err_msg);
	    if( !rv ) return false;
	    if( e.tagName() != "resource" ) {
		rv = false;
		if(err_msg) {
		    (*err_msg) = QString("Invalid <program> node.  "
					 "Expected <resource>, got <%1>")
			.arg(e.tagName());
		}
		return rv;
	    }
	    return rv;
	}

	bool TritiumXmlReader::validate_midi_integer_type(
	    const QString& value,
	    const QString& name,
	    bool optional,
	    QString *err_msg)
	{
	    bool rv = true;
	    bool int_ok;
	    unsigned val;

	    if( value.isEmpty() ) {
		if(optional) {
		    rv = true;
		} else {
		    rv = false;
		    if(err_msg) {
			(*err_msg) = QString("Value missing for '%1'."
			    " Should be from 0 through 127.")
			    .arg(name);
		    }
		}
		return rv;
	    }

	    rv = true;
	    val = value.toUInt(&int_ok);
	    if(!int_ok) {
		rv = false;
		if(err_msg) {
		    (*err_msg) = QString("Invalid node value for '%1'."
					 "Expected integer 0-127, got '%2'.")
			.arg(name)
			.arg(value);
		}
	    } else if (val > 127) {
		rv = false;
		if(err_msg) {
		    (*err_msg) = QString("Invalid node value for '%1'."
					 "Expected integer 0-127, got '%2'.")
			.arg(name)
			.arg(val);
		}
	    }

	    return rv;
	}

	/*============================================================
	 * HANDLER METHODS
	 *
	 * With the exception of the top-level elements (tritium,
	 * presets) these functions assume that the node is already
	 * validated.
	 *============================================================
	 */
	bool TritiumXmlReader::handle_load_tritium_node(QDomElement& tritium)
	{
	    if(tritium.tagName() != "tritium") {
		_error = true;
		_error_message = "Not a <tritium> node";
		return false;
	    }

	    bool rv;
	    QString err_msg;
	    rv = validate_tritium_node(tritium, &err_msg);
	    if( !rv ) {
		_error = true;
		_error_message = err_msg;
		return false;
	    }

	    QDomElement e = tritium.firstChildElement();
	    bool tmp;

	    for( ; ! e.isNull() ; e = e.nextSiblingElement() ) {
		if(e.tagName() == "presets") {
		    tmp = handle_load_presets_node(e);
		    if( !tmp ) rv = false;
		} else {
		    // unknown node type
		}
	    }
	    
	}

	bool TritiumXmlReader::handle_load_presets_node(QDomElement& presets)
	{
	    if(presets.tagName() != "presets") {
		_error = true;
		_error_message = "Not a <presets> node";
		return false;
	    }

	    bool rv;
	    QString err_msg;

	    rv = validate_presets_node(presets, &err_msg);
	    if( ! rv ) {
		_error = true;
		_error_message = err_msg;
		return rv;
	    }

	    QDomElement bank, program, midi_number, resource;
	    QString uri;
	    uint32_t bank_no, coarse, fine, pc;
	    T<Presets>::shared_ptr presets_obj(new Presets);

	    if( ! presets_obj ) {
		_error = true;
		_error_message = QString("Could not allocate a Presets object.");
		return false;
	    }

	    bank = presets.firstChildElement("bank");
	    for( ; ! bank.isNull() ; bank = bank.nextSiblingElement("bank") ) {
		coarse = bank.attribute("coarse", "0").toUInt();
		fine = bank.attribute("fine", "0").toUInt();
		program.clear();
		program = bank.firstChildElement("program");
		for( ; ! program.isNull() ; program = program.nextSiblingElement() ) {
		    midi_number = program.firstChildElement("midi_number");
		    resource = program.firstChildElement("resource");

		    pc = midi_number.text().toUInt();
		    uri = resource.text();
		    presets_obj->program(coarse, fine, pc, uri);
		}
	    }

	    _bdl.push(presets_obj);
	    return rv;
	}

    } // namespace Serialization
} // namespace Tritium
