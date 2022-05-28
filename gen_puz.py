#!/usr/bin/env python3
#
# gen_puz.py <subjects> [options]
#
# This program generates a random crossword puzzle in .puz format from 
# questions taken from one or more subject files.
#
import sys
import time
import random
import string
import re

def die( msg, prefix='ERROR: ' ):
    print( prefix + msg )
    sys.exit( 1 )

def rand_n( n ):
    return int( random.random() * n )

#-----------------------------------------------------------------------
# process command line args
#-----------------------------------------------------------------------
if len( sys.argv ) < 2: die( 'usage: puz.py <subjects> [options]', '' )
subjects_s = sys.argv[1]
subjects = subjects_s.split( ',' )
side = 15
reverse = False
seed = int( time.time() )
attempts = 10000
larger_cutoff = 7
start_pct = 0
end_pct = 100
html = True
title = ''
i = 2
while i < len( sys.argv ):
    arg = sys.argv[i]
    i += 1
    if   arg == '-side':           
        side = int(sys.argv[i])
        i += 1
    elif arg == '-reverse':          
        reverse = int(sys.argv[i]) == 1
        i += 1
    elif arg == '-seed':
        seed = int(sys.argv[i])
        i += 1
    elif arg == '-attempts':
        attempts = int(sys.argv[i])
        i += 1
    elif arg == '-larger_cutoff':
        larger_cutoff = int(sys.argv[i])
        i += 1
    elif arg == '-start_pct':
        start_pct = int(sys.argv[i])
        if start_pct < 0 or end_pct < 0: die( 'start_pct and end_pct must be >= 0' )
        i += 1
    elif arg == '-end_pct':
        end_pct = int(sys.argv[i])
        if end_pct > 100: die( 'end_pct must be <= 100' )
        i += 1
    elif arg == '-html':          
        html = int(sys.argv[i]) == 1
        i += 1
    elif arg == '-title':          
        title = sys.argv[i]
        i += 1
    else:
        die( f'unknown option: {arg}' )

if start_pct >= end_pct: die( 'start_pct must be < end_pct' )

if title == '': title = '_'.join( subjects ) + f'_{seed}' 

random.seed( seed )

#-----------------------------------------------------------------------
# Read in <subject>.txt files.
#-----------------------------------------------------------------------
entries = []
for subject in subjects:
    filename = subject + '.txt'
    Q = open( filename, 'r' )
    line_num = 0
    while True:
        question = Q.readline()
        if question == '': break
        line_num += 1
        question = re.sub( r'^\s+', '', question )
        question = re.sub( r'\s+$', '', question )
        if len(question) == 0 or question[0] == '#': continue

        answer = Q.readline()
        answer = re.sub( r'^\s+', '', answer )
        answer = re.sub( r'\s+$', '', answer )
        answer = answer.lower()
        if answer == '': die( f'question on line {line_num} is not followed by a non-blank answer on the next line: {question}' )
        line_num += 1

        if reverse:
            tmp = question
            question = answer
            answer = tmp

        entries.append( [question, answer] )
    Q.close()

#-----------------------------------------------------------------------
# Pull out all interesting answer words and put them into an array, 
# with a reference back to the original question.
#-----------------------------------------------------------------------
good_chars = {
    'a': 1, 'b': 1, 'c': 1, 'd': 1, 'e': 1, 'f': 1, 'g': 1, 'h': 1, 'i': 1, 'j': 1, 'k': 1, 'l': 1, 'm': 1,
    'n': 1, 'o': 1, 'p': 1, 'q': 1, 'r': 1, 's': 1, 't': 1, 'u': 1, 'v': 1, 'w': 1, 'x': 1, 'y': 1, 'z': 1,
    'à': 1, 'á': 1, 'è': 1, 'é': 1, 'ì': 1, 'í': 1, 'ò': 1, 'ó': 1, 'ù': 1, 'ú': 1 }

def pick_words( a ):
    words = []
    word = ''
    word_pos = 0
    in_parens = False
    for i in range(len(a)):
        ch = a[i]
        if ch == ' ' or ch == '\t' or ch == '\'' or ch == '’' or ch == '/' or ch == '(' or ch == ')' or \
           ch == '!' or ch == '?' or ch == '.' or ch == ',' or ch == '-' or ch == ':' or ch == '"' or ch == '[' or ch == ']' or \
           ch == '0' or ch == '1' or ch == '2' or ch == '3' or ch == '4' or ch == '5' or ch == '6' or ch == '7' or ch == '8' or ch == '9':
            if word != '': 
                if not in_parens: words.append( [word, word_pos] )
                word = ''
            if ch == '(':
                if in_parens: die( 'cannot support nested parens' )
                in_parens = True
            if ch == ')':
                if not in_parens: die( 'no matching right paren' )
                in_parens = False
        elif not in_parens:
            if word == '': word_pos = i
            c = ch.lower()
            if c not in good_chars:
                die( f'bad character \'{ch}\' in answer: {a}' )
            word += c

    if word != '': words.append( [word, word_pos] )
    return words

# <=3 letter words are already excluded
# these are common words with more then 3 letters to excluded
common_words = { 'avere': 1, 
                 'averla': 1,
                 'averlo': 1,
                 'averle': 1,
                 'averli': 1,
                 'aver': 1,
                 'essere': 1, 
                 'esserla': 1, 
                 'esserlo': 1, 
                 'esserle': 1, 
                 'esserli': 1, 
                 'stare': 1,
                 'stai': 1,
                 'stiamo': 1,
                 'state': 1,
                 'stanno': 1,
                 'fare': 1, 
                 'farla': 1,
                 'farlo': 1,
                 'farle': 1,
                 'farli': 1,
                 'farsi': 1,
                 'dare': 1,
                 'come': 1,
                 'così': 1,
                 'sono': 1, 
                 'miei': 1,
                 'tuoi': 1,
                 'suoi': 1,
                 'vuoi': 1,
                 'dall': 1,
                 'dalla': 1,
                 'dallo': 1,
                 'dagli': 1, 
                 'dalle': 1, 
                 'dell': 1,
                 'della': 1,
                 'dello': 1,
                 'degli': 1, 
                 'delle': 1, 
                 'nell': 1,
                 'nella': 1,
                 'nello': 1,
                 'negli': 1, 
                 'nelle': 1, 
                 'sull': 1,
                 'sugli': 1,
                 'sulla': 1,
                 'sullo': 1,
                 'sulle': 1,
                 'all': 1,
                 'alla': 1,
                 'allo': 1,
                 'alle': 1,
                 'agli': 1,
                 'cosa': 1,
                 'cose': 1,
                 'anno': 1,
                 'anni': 1,
                 'mese': 1,
                 'mesi': 1,
                 'idea': 1,
                 'idee': 1,
                 'area': 1,
                 'golf': 1,
                 'ieri': 1,
                 'ecco': 1,
                 'vita': 1,
                 'sole': 1,
                 'tuba': 1,
                 'film': 1,

                 'than': 1,
                 'each': 1,
                 'with': 1,
                 'does': 1,
                 'doesn': 1,
                 'must': 1,
                 'here': 1,
                 'bass': 1,
                 'take': 1,
                 'away': 1,
                 'club': 1,
               }

entry_cnt = len(entries)
entry_first = int(start_pct*entry_cnt/100.0)
entry_last  = min(int(end_pct*entry_cnt/100.0), entry_cnt-1)

words = []
for i in range(entry_first, entry_last+1):
    entry = entries[i]
    question = entry[0]
    answers = entry[1]
    aa = answers.split( '; ' )
    for a in aa:
        ww = pick_words( a )
        for w in ww:
            word = w[0]
            pos  = w[1]
            if len( word ) > 3 and not word in common_words:
                words.append( [word, pos, a, entry] )
word_cnt = len(words)

#-----------------------------------------------------------------------
# Generate the puzzle from the data structure using this simple algorithm:
#
#     for some number attempts:
#         pick a random word from the list (pick only longer words during first half)
#         if the word is already in the grid: continue
#         for each across/down location of the word:
#             score the placement of the word in that location
#         if score > 0:
#             add the word to one of the locations with the best score found
#-----------------------------------------------------------------------
grid = []
across_grid = []
down_grid = []
clue_grid = []
for x in range(side):
    grid.append( [] )
    across_grid.append( [] )
    down_grid.append( [] )
    clue_grid.append( [] )
    for y in range(side):
        grid[x].append( '-' )
        across_grid[x].append( '-' )
        down_grid[x].append( '-' )
        clue_grid[x].append( {} )

words_used = {}
large_frac = (0 + rand_n( 80 )) / 100.0
attempts_large = int( attempts * large_frac )
for i in range(attempts):
    wi = rand_n( word_cnt )
    info = words[wi]
    word = info[0]
    if word in words_used: continue
    if i < attempts_large and len(word) < larger_cutoff: continue
    pos = info[1]
    ans = info[2]
    entry = info[3]

    best_words = []
    best_score = 0
    word_len = len(word)
    for x in range(side):
        for y in range(side):
            if (x + word_len) <= side:
                # score across
                score = 5 if y == 0 or y == (side-1) else 1 
                if x == 0 or (x+word_len-1) == (side-1): score += 1
                for ci in range(word_len):
                    if across_grid[x+ci][y] != '-' or \
                       (ci == 0 and x > 0 and grid[x-1][y] != '-') or \
                       (ci == (word_len-1) and (x+ci+1) < side and grid[x+ci+1][y] != '-'): 
                        score = 0
                        break
                    c  = word[ci]
                    gc = grid[x+ci][y]
                    if c == gc:
                        score += 1
                    elif gc != '-' or \
                         (y > 0 and grid[x+ci][y-1] != '-') or \
                         (y < (side-1) and grid[x+ci][y+1] != '-'):
                        score = 0
                        break
                if score != 0 and score >= best_score:
                    if score > best_score:
                        best_score = score
                        best_words = []
                    best_words.append( [word, pos, ans, entry, x, y, True ] )

            if (y + word_len) <= side:
                # score down
                score = 5 if x == 0 or x == (side-1) else 1 
                if y == 0 or (y+word_len-1) == (side-1): score += 1
                for ci in range(word_len):
                    if down_grid[x][y+ci] != '-' or \
                       (ci == 0 and y > 0 and grid[x][y-1] != '-') or \
                       (ci == (word_len-1) and (y+ci+1) < side and grid[x][y+ci+1] != '-'):
                        score = 0
                        break
                    c  = word[ci]
                    gc = grid[x][y+ci]
                    if c == gc:
                        score += 1
                    elif gc != '-' or \
                         (x > 0 and grid[x-1][y+ci] != '-') or \
                         (x < (side-1) and grid[x+1][y+ci] != '-'):
                        score = 0
                        break
                if score != 0 and score >= best_score:
                    if score > best_score:
                        best_score = score
                        best_words = []
                    best_words.append( [word, pos, ans, entry, x, y, False] )
    if best_score > 0: 
        bi = rand_n( len(best_words) )
        best   = best_words[bi]
        word   = best[0]
        pos    = best[1]
        ans    = best[2]
        entry  = best[3]
        x      = best[4]
        y      = best[5]
        across = best[6]
        which  = 'across' if across else 'down'
        words_used[word] = best
        for ci in range(word_len):
            if across:
                grid[x+ci][y] = word[ci]
                across_grid[x+ci][y] = word[ci]
            else:
                grid[x][y+ci] = word[ci]
                down_grid[x][y+ci] = word[ci]
        if which in clue_grid[x][y]: die( f'{word}: {which} clue already at [{x},{y}]' )
        clue_grid[x][y][which] = best;
        #print( f'{word}: {which} clue added at [{x},{y}]' )

#-----------------------------------------------------------------------
# Genrerate .html or .puz file.
#-----------------------------------------------------------------------

if html:
    print( f'<!DOCTYPE html>' )
    print( f'<html lang="en">' )
    print( f'<head>' )
    print( f'<meta charset="utf-8"/>' )
    print( f'<meta name="viewport" content="width=device-width, initial-scale=1"/>' )
    print( f'<link rel="stylesheet" type="text/css" href="exolve-m.css?v1.35"/>' )
    print( f'<script src="exolve-m.js?v1.35"></script>' )
    print( f'<script src="exolve-from-ipuz.js?v1.35"></script>' )
    print( f'' )
    print( f'<title>Test-Ipuz-Solved</title>' )
    print( f'' )
    print( f'</head>' )
    print( f'<body>' )
    print( f'<script>' )
    print( f'let ipuz =' )

# header
print( f'{{' )
print( f'"origin": "Bob Alfieri",' )
print( f'"version": "http://ipuz.org/v1",' )
print( f'"kind": ["http://ipuz.org/crossword#1"],' )
#print( f'"copyright": "2022 Robert A. Alfieri (this puzzle), Viresh Ratnakar (crossword program)",' )
#print( f'"author": "Bob Alfieri",' )
print( f'"publisher": "Robert A. Alfieri",' )
print( f'"title": "{title}",' )
print( f'"intro": "",' )
print( f'"difficulty": "Moderate",' )
print( f'"empty": "0",' )
print( f'"dimensions": {{ "width": {side}, "height": {side} }},' )
print()

# solution
print( f'"solution": [' )
for y in range(side):
    for x in range(side):
        print( f'    [' if x == 0 else ', ', end='' )
        ch = '#' if grid[x][y] == '-' else grid[x][y].upper()
        print( f'"{ch}"', end='' )
    comma = ',' if y != (side-1) else ''
    print( f']{comma}' )
print( f'],' )

# labels
print( f'"puzzle": [' )
clue_num = 1
for y in range(side): 
    for x in range(side):
        print( '    [' if x == 0 else ', ', end='' )
        info = clue_grid[x][y]
        if 'across' in info or 'down' in info:
             print( f'{clue_num:3}', end='' )
             info['num'] = clue_num
             clue_num += 1
        elif grid[x][y] != '-':
             print( '  0', end='' )
        else: 
             print( '"#"', end='' )
    comma = ',' if y != (side-1) else ''
    print( f']{comma}' )            
print( f'],' )

# clues
print( f'"clues": {{' )
for which_mc in ['Across', 'Down']:
    print( f'    "{which_mc}": [', end='' )
    which = which_mc.lower()
    have_one = False
    for y in range(side):
        for x in range(side):
            cinfo = clue_grid[x][y]
            if which in cinfo: 
                if have_one: print( ', ', end='' )
                have_one = True
                print()
                winfo = cinfo[which]
                num   = cinfo['num']
                word  = winfo[0]
                first = winfo[1]
                last  = first + len(word) - 1 
                ans   = winfo[2]
                entry = winfo[3]
                ques  = entry[0]
                ans_  = ''
                for i in range(len(ans)):
                    ans_ += '_' if (i >= first and i <= last) else ans[i]
                clue  = f'"{ques} ==> {ans_}"' 
                print( f'        [{num}, {clue}]', end='' )
    comma = ',' if which == 'across' else ''
    print( f'\n    ]{comma}' )
print( f'}},' )
print( f'}}' )

if html:
    print( f'text = exolveFromIpuz(ipuz)' )
    #print( f'text += \'\\n    exolve-option: allow-chars:ÀÁÈÉÌÍÒÓÙÚ\\n\'' )
    print( f'text += \'\\n    exolve-language: it Latin\\n\'' )
    print( f'text += \'\\n    exolve-end\\n\'' )
    print( f'createExolve(text)' )
    print( f'</script>' )
    print( f'</body>' )
    print( f'</html>' )
