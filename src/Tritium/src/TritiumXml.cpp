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

#include "TritiumXml.hpp"
#include <Tritium/Presets.hpp>
#include <cassert>
#include <QtXml>
#include <QString>

namespace Tritium
{
    namespace Serialization
    {

	/*============================================================
	 * "USER" METHODS
	 *============================================================
	 */

	bool TritiumXml::readContent(QIODevice *dev)
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

	    return readContent(doc);
	}

	bool TritiumXml::readContent(const QString& text)
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

	    return readContent(doc);
	}

	bool TritiumXml::readContent( QDomDocument& doc )
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
		return read_tritium_node(root);
	    } else if (root.tagName() == "presets") {
		return read_presets_node(root);
	    } else {
		_error = true;
		_error_message = QString("Invalid root document element '%1'")
		    .arg(root.tagName());
	    }
	    return false;
	}

	/**
	 * Writes the contents of TritiumXml to the string 'str'.
	 *
	 * Note that this empties the TritiumXml object.
	 */
	bool TritiumXml::writeContent( QString& str )
	{
	    QXmlStreamWriter w(&str);
	    bool rv = true;

	    w.writeStartDocument();
	    w.setAutoFormatting(true);
	    w.writeNamespace(TRITIUM_XML, "T");
	    rv = write_tritium_node_start(w);
	    if(!rv) return false;

	    while( !empty() ) {
		switch( peek_type() ) {
		case ObjectItem::Presets_t:
		    rv = write_presets_node(w);
		    break;
		default:
		    pop();
		}
		if(!rv) break;
	    }
	    if(!rv) return false;

	    rv = write_tritium_node_end(w);
	    if(!rv) return false;

	    w.writeEndDocument();

	    // VALIDATEION
	    QDomDocument doc;
	    QString e_msg;
	    int line, col;
	    rv = doc.setContent(str, true, &e_msg, &line, &col);
	    if(!rv) {
		_error = true;
		rv = false;
		_error_message = QString("Error creating Tritium XML document. "
					 "This is a bug in Tritium/Composite.  "
					 "Please report this to the developers. "
					 "Tritium internally created an invalid "
					 "XML file. The error reported was..."
					 "L%1 C%2: %3")
		    .arg(line)
		    .arg(col)
		    .arg(e_msg);
		return false;
	    }

	    QDomElement tritium = doc.documentElement();
	    rv = validate_tritium_node(tritium, &e_msg);
	    if(!rv) {
		_error = true;
		rv = false;
		_error_message = QString("Error creating Tritium XML document. "
					 "This is a bug in Tritium/Composite. "
					 "Please report this to the developers. "
					 "Tritium created a well-formed XML file, "
					 "but did not validate with the tritium "
					 "XML schema.  The error reported was... %1")
		    .arg(e_msg);
	    }
	    return rv;

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

	bool TritiumXml::validate_tritium_node(QDomElement& tritium, QString *err_msg)
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

	bool TritiumXml::validate_presets_node(QDomElement& presets, QString *err_msg)
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

	bool TritiumXml::validate_bank_node(QDomElement& bank, QString *err_msg)
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

	bool TritiumXml::validate_program_node(QDomElement& program, QString *err_msg)
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

	bool TritiumXml::validate_midi_integer_type(
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
	 * READER METHODS
	 *
	 * With the exception of the top-level elements (tritium,
	 * presets) these functions assume that the node is already
	 * validated.
	 *============================================================
	 */
	bool TritiumXml::read_tritium_node(QDomElement& tritium)
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
		    tmp = read_presets_node(e);
		    if( !tmp ) rv = false;
		} else {
		    // unknown node type
		}
	    }
	    return rv;
	}

	bool TritiumXml::read_presets_node(QDomElement& presets)
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
	    uint32_t coarse, fine, pc;
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
		    presets_obj->set_program(coarse, fine, pc, uri);
		}
	    }

	    push(presets_obj);
	    return rv;
	}

	/*============================================================
	 * WRITER METHODS
	 *
	 * You're probably wondering... "Why are we using QDomDocument
	 * for reading, but QXmlStreamWriter for writing? Wouldn't it
	 * be better to be consistent?"  Yes, but...
	 *
	 * When using XML namespaces, QDomDocument (as of Qt 4.5) will
	 * write the file like this:
	 *
	 * <foo xmlns="http://foo.com">
	 *   <bar xmlns="http://foo.com">
	 *     <bat xmlns="http://foo.com">Content</bat>
         *   </bar>
         * </foo>
	 *
	 * It does this even if you declare a namespace prefix.  Not
	 * only is this UGLY XML, it makes the file needlessly larger.
	 * Meanwhile, QXmlStreamWriter will output files like this:
	 *
	 * <x:foo xmlns:x="http://foo.com">
	 *   <x:bar>
	 *     <x:bat>Content</x:bat>
	 *   </x:bar>
	 * </x:foo>
	 *
	 * While it would be nice to /not/ have the "x:" prefix...
	 * I can live with it.  :-)
	 *
	 *============================================================
	 */

	bool TritiumXml::write_tritium_node_start(QXmlStreamWriter& w)
	{
	    w.writeStartElement(TRITIUM_XML, "tritium");
	    return true;
	}

	bool TritiumXml::write_tritium_node_end(QXmlStreamWriter& w)
	{
	    w.writeEndElement();
	    return true;
	}

	bool TritiumXml::write_presets_node(QXmlStreamWriter& w)
	{
	    assert( !empty() );
	    assert( peek_type() == ObjectItem::Presets_t );

	    w.writeStartElement(TRITIUM_XML, "presets");

	    T<Presets>::shared_ptr presets = pop<Presets>();
	    Presets::const_iterator p;
	    Bank::const_iterator b;
	    uint8_t coarse, fine, pc;
	    for( p = presets->begin() ; p != presets->end() ; ++p ) {
		coarse = p->first.coarse;
		fine = p->first.fine;
		w.writeStartElement(TRITIUM_XML, "bank");
		w.writeAttribute("coarse", QString::number(coarse));
		w.writeAttribute("fine", QString::number(fine));
		for( b = p->second.begin() ; b != p->second.end() ; ++b ) {
		    pc = b->first;
		    const QString& uri = b->second;
		    w.writeStartElement(TRITIUM_XML, "program");
		    w.writeTextElement(TRITIUM_XML, "midi_number", QString::number(pc));
		    w.writeTextElement(TRITIUM_XML, "resource", uri);
		    w.writeEndElement(); // program
		}
		w.writeEndElement(); // bank
	    }

	    w.writeEndElement(); // presets
	    return true;
	}

    } // namespace Serialization
} // namespace Tritium
