#!/usr/bin/env python3
#
# gen_www.py - generates crossword puzzles for all interesting categories for website
#
import sys
import os
import os.path
import subprocess
import time
import string
import re
import datetime

subjects = [ [ 'italian_basic',         '#a99887',      True ],
             [ 'italian_advanced',      '#53af8b',      True ],
             [ 'italian_expressions',   '#587a8f',      False ],
             [ 'italian_vulgar',        '#95b8e3',      False ],
             [ 'all_lists',             '#c095e3',      False ] ]

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
side = 15
count = 100
today = datetime.date.today()
year = today.year - 2000
month = today.month
day = today.day
seed = year*10000 + month*100 + day
seed *= 10000

i = 2
while i < len( sys.argv ):
    arg = sys.argv[i]
    i += 1
    if   arg == '-side':           
        side = int(sys.argv[i])
        i += 1
    elif arg == '-count':
        count = int(sys.argv[i])
        i += 1
    elif arg == '-seed':
        seed = int(sys.argv[i])
        i += 1
    else:
        die( f'unknown option: {arg}' )

cmd( f'rm -f www/*.html' )

s = ''
s += f'<html>\n'
s += f'<head>\n'
s += f'<style type="text/css">\n'
s += f'.rectangle {{\n'
s += f'  height: 30px;\n'
s += f'  width: 30px;\n'
s += f'  color:black;\n'
s += f'  background-color: rgb(0,0,255);\n'
s += f'  border-radius:5px;\n'
s += f'  display: flex;\n'
s += f'  justify-content:center;\n'
s += f'  align-items: center;\n'
s += f'  font-family: Arial;\n'
s += f'  font-size: 18px;\n'
s += f'  font-weight: bold;\n'
s += f'  float: left;\n'
s += f'  margin-right: 5px;\n'
s += f'  margin-bottom: 5px;\n'
s += f'}}\n'
s += f'</style>\n'
s += f'</head>\n'
s += f'<title>Italian-English Crossword Puzzles</title>\n'
s += f'<h1>Italian-English Crossword Puzzles</h1>\n'
s += f'<body>\n'

#-----------------------------------------------------------------------
# Generate the individual puzzles.
#-----------------------------------------------------------------------
all_s = ''
for subject_info in subjects:
    subject   = subject_info[0]
    color     = subject_info[1]
    do_recent = subject_info[2]
    s += f'<section style="clear: left">\n'
    s += f'<br>\n'
    if subject != 'all_lists':
        s += f'<h2><a href="https://github.com/balfieri/study/blob/master/{subject}.txt">{subject}</a></h2>'
        if all_s != '': all_s += ','
        all_s += subject
    else:
        s += f'<h2>{subject}</h2>'
    for reverse in range(2):
        clue_lang = 'Italian' if reverse == 0 else 'English'
        for recent in range(2):
            if recent and not do_recent: continue
            recency = f'most recent entries' if recent else f'all entries'
            start_pct = 85 if recent else 0
            s += f'<section style="clear: left">\n'
            s += f'<b>{clue_lang} ({recency}):</b><br>'
            for i in range(count):
                title = f'{subject}_s{seed}_r{reverse}'
                subjects = all_s if subject == 'all_lists' else subject
                cmd( f'./gen_puz.py {subjects} -side {side} -seed {seed} -reverse {reverse} -start_pct {start_pct} -title {title} > www/{title}.html' )
                seed += 1
                s += f'<a href="{title}.html"><div class="rectangle" style="background-color: {color}">{i}</div></a>\n'

s += f'<section style="clear: left">\n'
s += '<br>\n'
s += '</body>\n'
s += '</html>\n'

file = open( "www/index.html", "w" )
a = file.write( s )
file.close()
