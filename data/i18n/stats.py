#!/usr/bin/env python

from xml.dom import minidom, Node


def parse_ts(filename):
	doc = minidom.parse(filename)
	root_node = doc.documentElement
	
	total = 0
	unfinished = 0
	obsolete = 0
	
	for node in root_node.childNodes:
		if node.nodeName == "context":
			context_node = node
			
			for context_child in context_node.childNodes:
				if context_child.nodeName == "message":
					message_node = context_child
					
					for message_node_child in message_node.childNodes:
						if message_node_child.nodeName == "translation":
							#print "translation"
							translation_node = message_node_child
							total += 1
							
							if translation_node.attributes:
								translation_type = translation_node.attributes["type"].value
								#print translation_type
								if translation_type == "unfinished":
									unfinished += 1
								elif translation_type == "obsolete":
									obsolete += 1

	#print "Total: %d" % (total - obsolete)
	#print "obsolete %d" % obsolete
	#print "Unfinished: %d" % unfinished

	completed = (total - obsolete) - unfinished
	perc = 100.0 / (total - obsolete) * completed
	
	print "|| %s\t|| [%d/%d]\t|| %d%%\t||" % (filename, completed, total - obsolete, perc)



parse_ts("composite.de.ts")
parse_ts("composite.ca.ts")
parse_ts("composite.fr.ts")
parse_ts("composite.it.ts")
parse_ts("composite.nl.ts")
parse_ts("composite.pt_BR.ts")
parse_ts("composite.sv.ts")
parse_ts("composite.es.ts")
parse_ts("composite.hu_HU.ts")
parse_ts("composite.ja.ts")
parse_ts("composite.pl.ts")
parse_ts("composite.ru.ts")
parse_ts("composite.hr.ts")
