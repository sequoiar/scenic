#!/usr/bin/env python
# -*- coding: utf-8 -*-

import pyjsdoc
import sys

for arg in sys.argv[1:]:
	try:
		module = pyjsdoc.FileDoc(arg, pyjsdoc.read_file(arg))
	except:
		print 'Problem with ' + arg
	else:
		name = module.name.title().rpartition('.')[0]
		caption = name + ' Documentation'
		print caption
		print ('=' * len(caption)) + '\n'
		print 'This page contains the ' + name + ' Javascript Module documentation.\n'
		title = 'The :mod:`' + name.lower() + '` Module'
		print title
		print '-' * len(title)
		print '\n'
		
		for klass in module.classes:
			klass_str = '.. class:: ' + klass.name
			if len(klass.constructors) > 0:
				params = klass.get_method(klass.constructors[0].name).params
				param_list = []
				for param in params:
					if param.name != 'self':
						param_list.append(param.name)
				if param_list:
					klass_str += '(' + ', '.join(param_list) + ')'
			print klass_str
			if klass.parsed.has_key('member') and klass.parsed['member']:
				print '\n   Bases: :class:`' + klass.parsed['member'] + '`'
			print '\n   ' + klass.doc.replace('\n', '\n  ') + '\n'
			if param_list:
				for param in params:
					if param.name != 'self':
						if param.doc:
							print '   :param ' + param.name + ': ' + param.doc
						if param.type:
							print '   :type ' + param.name + ': ' + param.type
			print '\n'
			
			for meth in klass.methods:
				if meth.name != '__init__':
					param_list = []
					for param in meth.params:
						if param.name != 'self':
							param_list.append(param.name)
					print '   .. method:: ' + meth.name + '(' + ', '.join(param_list) + ')\n'
					print '      ' + meth.doc.replace('\n', '\n     ') + '\n'
					
					for param in meth.params:
						if param.name != 'self':
							if param.doc:
								print '      :param ' + param.name + ': ' + param.doc
								if param.type:
									print '      :type ' + param.name + ': ' + param.type
							elif param.type:
								print '      :param ' + param.name + ':'
								print '      :type ' + param.name + ': ' + param.type
					if meth.return_val.doc:
						print '      :return: ' + meth.return_val.doc
					if meth.return_val.type:
						print '      :rtype: ' + meth.return_val.type
					print '\n'
			print ''
		print ''
		
		for func in module.functions:
			func_str = '.. function:: ' + func.name
			param_list = []
			for param in func.params:
				param_list.append(param.name)
			if param_list:
				func_str += '(' + ', '.join(param_list) + ')'
			else:
				func_str += '()'
			print func_str
			print '\n   ' + func.doc.replace('\n', '\n  ') + '\n'
			if param_list:
				for param in func.params:
					if param.doc:
						print '   :param ' + param.name + ': ' + param.doc
						if param.type:
							print '   :type ' + param.name + ': ' + param.type
					elif param.type:
						print '   :param ' + param.name + ':'
						print '   :type ' + param.name + ': ' + param.type
			if func.return_val.doc:
				print '   :return: ' + func.return_val.doc
			if func.return_val.type:
				print '   :rtype: ' + func.return_val.type
			print '\n'
			
		