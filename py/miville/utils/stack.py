import os
import traceback 

def print_stack():
    """
    Prints the Python stack
    """    
    s = traceback.extract_stack()
    dirname = ''
    try:
        dirname = os.path.dirname(__file__)
        dirname = '/'.join(dirname.split('/')[:-2]) + '/' # parent dir
    except IndexError, e:
        print 'error in print_stack:', e.message
    print '\n----------'
    print 'TRACEBACK:'
    for quad in s:
        file, line, function, code = quad
        file = file.replace('/usr/lib/python2.5/site-packages/', '')
        file = file.replace(dirname, '')
        print " * %s:%s in %s : %s" % (file, line, function, code)
    print '----------'


