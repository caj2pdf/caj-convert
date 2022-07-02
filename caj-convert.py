#!/usr/bin/env python

# Minimal Linux-specific re-write of the C++ caj-convert.c in Python.

# Copyright 2022 (c) Hin-Tak Leung <htl10@users.sourceforge.net>

import sys

if len(sys.argv) < 3:
    print("Usage: %s input.caj output.pdf [.caj]" % sys.argv[0])
    sys.exit()

from ctypes import *

# "./" is needed, as linux otherwise looks only at system locations.
libname = './libreaderex_x64.so'

_lib = CDLL(libname)

CAJFILE_Init          = _lib.CAJFILE_Init
CAJFILE_Init.argtypes = [c_char_p, c_void_p, c_int, c_char_p]
CAJFILE_Init.restype  = c_int

CAJFILE_DistillPageEx1          = _lib.CAJFILE_DistillPageEx1
CAJFILE_DistillPageEx1.argtypes = [c_void_p]
CAJFILE_DistillPageEx1.restype  = c_int

class Parameter(Structure):
    _fields_ = [('cb',      c_long      ),
                ('flag',    c_uint * 2  ),
                ('src',     c_char_p    ),
                ('extname', c_char_p    ),
                ('pfnFILE', c_void_p * 6),
                ('dest',    c_char_p    ),
                ('pfnoss',  c_void_p * 6)]

result = CAJFILE_Init(b".", None, 0, b"./tmp")
if (result != 0):
    print("CAJFILE_Init Failed.")

params         = Parameter()
params.cb      = sizeof(Parameter)
params.flag[1] = 0x26
params.src     = sys.argv[1].encode()
# extname is mostly ignored but needs to be ".caj" or ".teb" in some cases
if (len(sys.argv) == 3):
    params.extname = sys.argv[1][-4:].encode()
    print("Using \"%s\" as extname." % params.extname)
else:
    params.extname = sys.argv[3].encode()
params.dest    = sys.argv[2].encode()

result = CAJFILE_DistillPageEx1(byref(params))
if (result == 0):
    print("CAJFILE_DistillPageEx1 Failed.")
