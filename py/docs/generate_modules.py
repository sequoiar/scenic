#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Sropulpof
# Copyright (C) 2008 Société des arts technologiques (SAT)
# http://www.sat.qc.ca
# All rights reserved.
#
# This file is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# Sropulpof is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Sropulpof.  If not, see <http:#www.gnu.org/licenses/>.

"""
This script parse a directory tree looking for python modules and packages and
create ReST files appropriately to create code documentation with Sphinx.
It also create a modules index. 
"""

import os
import sys


# automodule options
options = ['members',
            'undoc-members',
#            'inherited-members', # disable because there's a bug in sphinx
            'show-inheritance']


def write_directive(module):
    """Create the automodule directive and add the options"""
    directive = '.. automodule:: %s\n' % module
    for option in options:
        directive += '    :%s:\n' % option
    return directive

def write_heading(module, kind='Module'):
    """Create the page heading."""
    module = module.title()
    heading = title_line(module + ' Documentation', '=')
    heading += 'This page contains the %s %s documentation.\n\n' % (module, kind)
    return heading

def write_sub(module, kind='Module'):
    """Create the module subtitle"""
    sub = title_line('The :mod:`%s` %s' % (module, kind), '-')
    return sub

def title_line(title, char):
    """ Underline the title with the character pass, with the right length."""
    return '%s\n%s\n\n' % (title, len(title) * char)

def create_module_file(root, module):
    """Build the text of the file and write the file."""
    name = '%s.txt' % module.lower()
    if os.path.isfile(name):
        print 'File %s already exists.' % name
    elif check_for_code('%s/%s.py' % (root, module)):   # don't build the file if there's no code in it
        print 'Creating file %s for module.' % name
        text = write_heading(module)
        text += write_sub(module)
        text += write_directive(module)

        # write the file        
        fd = open(name, 'w')
        fd.write(text)
        fd.close()

def create_package_file(root, subroot, py_files, subs=None):
    """Build the text of the file and write the file."""
    package = root.rpartition('/')[2].lower()
    name = '%s.txt' % subroot
    if os.path.isfile(name):
        print 'File %s already exists.' % name
    else:
        print 'Creating file %s for package.' % name
        text = write_heading(package, 'Package')
        if subs == None:
            subs = []
        else:
            # build a list of directories that are package (they contain an __init_.py file)
            subs = [sub for sub in subs if os.path.isfile('%s/%s/__init__.py' % (root, sub))]
            # if there's some package directories, add a TOC for theses subpackages
            if subs:
                text += title_line('Subpackages', '-')
                text += '.. toctree::\n\n'
                for sub in subs:
                    text += '    %s.%s\n' % (subroot, sub)
                text += '\n'
                    
        # add each package's module
        for py_file in py_files:
            if not check_for_code('%s/%s' % (root, py_file)):
                # don't build the file if there's no code in it
                continue
            py_file = py_file[: - 3]
            py_path = '%s.%s' % (subroot, py_file)
            kind = "Modules"
            if py_file == '__init__':
                py_file = package
                py_path = package
                kind = "Package"
            text += write_sub(py_path, kind)
            text += write_directive(py_path)
            text += '\n'

        # write the file
        fd = open(name, 'w')
        fd.write(text)
        fd.close()

def check_for_code(module):
    """
    Check if there's at least one class or one function in the module.
    """
    fd = open(module, 'r')
    for line in fd:
        if line.startswith('def ') or line.startswith('class '):
            fd.close()
            return True
    fd.close()
    return False
        
def recurse_tree(path, excludes):
    """
    Look for every file in the directory tree and create the corresponding
    ReST files.
    """
    toc = []
    excludes = format_excludes(path, excludes)
    tree = os.walk(sys.argv[1], False)
    for root, subs, files in tree:
        # keep only the Python script files
        py_files = check_py_file(files)
        # remove hidden ('.') and private ('_') directories
        subs = [sub for sub in subs if sub[0] not in ['.', '_']]
        # check if there's valid files to process
        if "/." in root or "/_" in root \
        or not py_files \
        or check_excludes(root, excludes):
            continue
        subroot = root[len(path) + 1:].replace('/', '.')
        if root == path:
            # we are at the root level so we create only modules
            for py_file in py_files:
                module = py_file[: - 3]
                create_module_file(root, module)
                toc.append(module)
        elif not subs and "__init__.py" in py_files:
            # we are in a package without sub package
            create_package_file(root, subroot, py_files)
            toc.append(subroot)
        elif "__init__.py" in py_files:
            # we are in package with subpackage(s)
            create_package_file(root, subroot, py_files, subs)
            toc.append(subroot)
            
    # create the module's index
    modules_toc(toc)

def modules_toc(modules, name='modules.txt'):
    """
    Create the module's index.
    """
    print "Creating module's index modules.txt."
    text = write_heading("Miville Modules", '')
    text += title_line('Modules:', '-')
    text += '.. toctree::\n'
    text += '   :maxdepth: 4\n\n'
    
    modules.sort()
    prev_module = ''
    for module in modules:
        # look if the module is a subpackage and, if yes,  ignore it
        if module.startswith(prev_module + '.'):
            continue
        prev_module = module
        text += '   %s\n' % module
        
    # write the file
    fd = open(name, 'w')
    fd.write(text)
    fd.close()

def format_excludes(path, excludes):
    """
    Format the excluded directory list.
    (verify that the path is not from the root of the volume or the root of the
    package)
    """
    f_excludes = []
    for exclude in excludes:
        if exclude[0] != '/' and exclude[:len(path)] != path:
            exclude = '%s/%s' % (path, exclude)
        # remove trailing slash
        f_excludes.append(exclude.rstrip('/'))
    return f_excludes

def check_excludes(root, excludes):
    """
    Check if the directory is in the exclude list.
    """
    for exclude in excludes:
        if root[:len(exclude)] == exclude:
            return True
    return False

def check_py_file(files):
    """
    Return a list with only the python scripts (remove all other files). 
    """
    py_files = [fich for fich in files if fich[ - 3:] == '.py']
    return py_files


if __name__ == '__main__':
    if len(sys.argv) < 2:
        print "You should specified the path to the root of the modules/packages."
    else:
        if os.path.isdir(sys.argv[1]):
            excludes = []
            # if there's some exclude arguments, build the list of excludes
            if len(sys.argv) > 2:
                excludes = sys.argv[2:]
            recurse_tree(sys.argv[1], excludes)
        else:
            print '%s is not a valid directory.' % sys.argv[1]
            
            