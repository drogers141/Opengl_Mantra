'''
Place for convenience functions, objects, etc. related to opengl.

Created on Jul 21, 2010

@author: drogers
'''
import os, sys

def normalize_rgb(rgb):
    """Convert 3 tuple or list with values in interval [0,255] to 3 tuple with values over [0,1].
    Prints comma separated output rounded to 3 places for copying.
    @return 3 tuple
    """
    ret = []
    strval = ""
    for i in rgb:
        ret.append(i/255.0)
        strval += "%.3f, " % (i/255.0)
    print "%s" % (strval[:-2], )
    return tuple(ret)