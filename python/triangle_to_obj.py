#!/usr/bin/env python
'''
Glue script to take tesselation output from the triangle program and put it into an
obj (wavefront) file.

Created on Jun 23, 2010

@author: drogers
'''
import os, sys
import subprocess

usage = \
"""Usage:  %s nodefile elefile objfile
    Writes objfile from the nodefile and elefile
""" % os.path.basename(sys.argv[0])


def convert_to_obj(nodefile, elefile, objfile):
    """Converts triangle program output in elefile and nodefile to objfile, writes empty mtl file as well.
    We are in 2d here, so writing obj output for xz axis with normals up (maybe).
    """
    obj = open(objfile, 'w')
    print "objfile: ", objfile
    mtlfile = "%s.mtl" % (objfile.split('.')[0])
    print "mtlfile: ", mtlfile
    # write empty metal file
    mtl = open(mtlfile, 'w')
    print >> mtl, "# MTL file converted from Triangle output: %s" % mtlfile
    print >> mtl, "# Material Count: 0"
    mtl.close()
    
    # start objfile 
    print >> obj, "# OBJ file converted from Triangle output: %s" % objfile
    print >> obj, "mtllib %s" % mtlfile
    
    print "nodefile: ", nodefile
    numverts = numtriangles = 0
    i = 0
    for line in open(nodefile):
        line = line.strip()
        if line.startswith('#'):
            continue
        parts = line.split()
        if not parts or int(parts[0]) != i:
            # pick up number of verts from first line
            if i == 0 and parts:
                numverts = int(parts[0])
                print "numverts:", numverts
            continue
        i += 1
#        print "line: %s" % line
#        if i < 30:
#            print "parts:", parts
        objline = "v %f %f %f" % (float(parts[1]), float(0), float(parts[2]))
        print >> obj, objline
#    print "i =", i
    print >> obj, "usemtl (null)"
    print >> obj, "s off"
    i = 0
    for line in open(elefile):
        line = line.strip()
        if line.startswith('#'):
            continue
        parts = line.split()
        if not parts or int(parts[0]) != i:
            # pick up number of triangles from first line
            if i == 0 and parts:
                numtriangles = int(parts[0])
                print "numtriangles:", numtriangles
            continue
        # parse triangle vertices
        if len(parts) < 4:
            print "error parsing triangles, not enough verts, line: %d" % i
            print line
        # careful: ele file indexes nodefile, which is zero based, obj file is 1 based
        v1 = int(parts[1]) + 1
        v2 = int(parts[2]) + 1
        v3 = int(parts[3]) + 1
        objline = "f %d %d %d" % (v1, v2, v3)
        print >> obj, objline        
        i += 1
    
    obj.close()


def main(argv): 
    num_args = len(argv)
    if num_args == 4:
        convert_to_obj(argv[1], argv[2], argv[3])
    else:
        print usage
    
    
if __name__ == '__main__':
    main(sys.argv)