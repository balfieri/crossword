#!/usr/bin/env python3
#
# play_puz.py - generate one puzzle and play it right now
#
import sys
import os
import os.path
import subprocess
import time
import string
import re
import datetime

def die( msg, prefix='ERROR: ' ):
    print( prefix + msg )
    sys.exit( 1 )

cmd_en = True

def cmd( c, echo=True, echo_stdout=False, can_die=True ):  
    if echo: print( c )
    if cmd_en:
        info = subprocess.run( c, shell=True, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT )
        if echo_stdout: print( info.stdout )
        if can_die and info.returncode != 0: die( f'command failed: {c}' )
        return info.stdout
    else:
        return ''

#-----------------------------------------------------------------------
# process command line args
#-----------------------------------------------------------------------
subjects = 'italian_advanced'

extra_args = ''

i = 1
while i < len( sys.argv ):
    arg = sys.argv[i]
    i += 1
    if   arg == '-subjects':
        subjects = sys.argv[i]
        i += 1
    else:
        extra_args += f' {arg} {sys.argv[i]}'
        i += 1

cmd( f'./gen_puz.py {subjects}{extra_args} > www/one.html' )
cmd( f'open -a Safari www/one.html' )
