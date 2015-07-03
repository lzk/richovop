#!/usr/bin/python
#coding=utf-8

# from xml.dom.minidom import parse
import xml.dom.minidom

def add_dict_lang(dict ,xml_file ,tag_message='message' ,tag_source='source' ,tag_translation='translation'):
	DOMTree = xml.dom.minidom.parse(xml_file)
	qph = DOMTree.documentElement
	language = qph.getAttribute('language')
	messages = qph.getElementsByTagName(tag_message)
	for message in messages:
		source = message.getElementsByTagName(tag_source)[0].childNodes[0]
		nodes_translation = message.getElementsByTagName(tag_translation)[0].childNodes
		if nodes_translation:
			if dict.has_key(source.data):
				if dict[source.data].has_key(language):
					print('a same ID:' + source.data + ' has two ' + language +' string,replace it')
					print(source.data + ':' + dict[source.data][language]+'-->' + nodes_translation[0].data)
					dict[source.data][language] = nodes_translation[0].data
				else:
					dict[source.data][language] = nodes_translation[0].data
					print dict[source.data]
			else:
				dict[source.data] = {language:nodes_translation[0].data}
		else:
			if dict.has_key(source.data):
				if dict[source.data].has_key(language):
					print('a same ID:'+source.data +' has two ' +language +' string,reserve it')
					print(dict[source.data][language]+'<--'+'')
				else:
					dict[source.data][language] = ''
					print dict[source.data]
			else:
				dict[source.data] = {language:''}
	return language


def add_dict_lang_ts(dict ,xml_file):
	return add_dict_lang(dict ,xml_file)

def add_dict_lang_qph(dict ,xml_file):
	return add_dict_lang(dict ,xml_file ,'phrase' ,'source' ,'target')


def get_rc_str(str ,values ,langugage):
	str += '\n\nLANGUAGE %s' % langugage
	# str += langugage
	str +=\
'''
STRINGTABLE
BEGIN
'''
	for value in values:
		str += '\t"%s"\t"%s"\n' % (value[0] ,value[1][langugage])
	str += 'END\n\n'
	return str

def to_rc_file(dict ,languages):
	str =''
	values=dict.items()
	for language in languages:
		str=get_rc_str(str ,values ,language)

	fo = open('test.rc' ,'w')
	fo.write(str.encode('utf-8'))
	fo.close()
	return


def match_dict(str ,dict ,dict_spec ,language):
	str += '\n%s  spec IDs:  %d ,res IDS:  %d' % (language ,len(dict_spec) ,len(dict))
	str += '\n-------------%s-------------\n' % language
	mismatch=0
	miss=0
	for spec_key in dict_spec.keys():
		s_key=spec_key.decode('utf-8')
		if dict.has_key(s_key):
			if dict_spec[spec_key][language].decode('utf-8') == dict[s_key][language]:
				continue
			else:
				str +='\n[Mismatch]'
				str +='\nID:"%s"' % s_key
				str += '\nspec:"%s"' % dict_spec[spec_key][language].decode('utf-8')
				str += '\nres:"%s"' % dict[s_key][language]
				mismatch+=1
		else:
			str +='\n[Miss]'
			str +='\nID:"%s"' % s_key
			str += '\nspec:"%s"' % dict_spec[spec_key][language].decode('utf-8')
			str += '\nres:'
			miss+=1
	str+='\nMisMatch: %d,Miss: %d' % (mismatch ,miss)
	return str

def to_match_file(dict ,languages):
	from test import spec_string
	str =''
	for language in languages:
		str=match_dict(str ,dict ,spec_string ,language)

	fo = open('test.match' ,'w')
	fo.write(str.encode('utf-8'))
	fo.close()
	return

import sys
argv=sys.argv
_program_name=sys.argv[0].rpartition('/')
if _program_name[0] == sys.argv[0]:
	program_name=_program_name[0]
else:
	program_name=_program_name[2]

argv.remove(sys.argv[0])
qphs=set(argv)
if not qphs:
	print('there are no qph files')
	sys.exit(-1)

dict={}
languages=()
for qph in qphs:
	langugage = add_dict_lang_qph(dict ,qph)
	languages+=(langugage,)
if program_name == 'change_to_rc_file':
	to_rc_file(dict ,languages)
	print('change to rc file complete')
elif program_name == 'match_file':
	to_match_file(dict ,languages)
	print('match file complete')
else:
	to_rc_file(dict ,languages)
	to_match_file(dict ,languages)
	print('change to rc file and match file complete')

print('你好,goodbye!!!!!')

