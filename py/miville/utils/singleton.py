#!/usr/bin/env python

class Singleton (object):
    def __new__(klass, *args, **kwargs): 
        if not hasattr(klass, 'self'):
            klass.self = object.__new__(klass)
        return klass.self

if __name__ == '__main__':
    print "starting example:"
    class Some(Singleton):
        def __init__(self):
            pass
            print 'New Some object.'
    a = Some()
    b = Some()
    print a
    print b
    if a is b:
        print "SUCCESS"
    else:
        raise Exception,"a and b are not the same instance."
