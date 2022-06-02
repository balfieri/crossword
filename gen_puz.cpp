// Copyright (c) 2022-2023 Robert A. Alfieri
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
// gen_puz <subjects> [options]
//
// This program generates a random crossword puzzle in .puz format from 
// questions taken from one or more subject files.
//
#include "sys.h"                // common utility functions

// <=3 letter words are already excluded
// these are common words with more then 3 letters to excluded
const std::map<std::wstring, bool> common_words = { 
     {L"avere", true}, 
     {L"averla", true},
     {L"averlo", true},
     {L"averle", true},
     {L"averli", true},
     {L"aver", true},
     {L"essere", true}, 
     {L"esserla", true}, 
     {L"esserlo", true}, 
     {L"esserle", true}, 
     {L"esserli", true}, 
     {L"stare", true},
     {L"stai", true},
     {L"stiamo", true},
     {L"state", true},
     {L"stanno", true},
     {L"fare", true}, 
     {L"farla", true},
     {L"farlo", true},
     {L"farle", true},
     {L"farli", true},
     {L"farsi", true},
     {L"dare", true},
     {L"come", true},
     {L"così", true},
     {L"sono", true}, 
     {L"miei", true},
     {L"tuoi", true},
     {L"suoi", true},
     {L"vuoi", true},
     {L"dall", true},
     {L"dalla", true},
     {L"dallo", true},
     {L"dagli", true}, 
     {L"dalle", true}, 
     {L"dell", true},
     {L"della", true},
     {L"dello", true},
     {L"degli", true}, 
     {L"delle", true}, 
     {L"nell", true},
     {L"nella", true},
     {L"nello", true},
     {L"negli", true}, 
     {L"nelle", true}, 
     {L"sull", true},
     {L"sugli", true},
     {L"sulla", true},
     {L"sullo", true},
     {L"sulle", true},
     {L"all", true},
     {L"alla", true},
     {L"allo", true},
     {L"alle", true},
     {L"agli", true},
     {L"cosa", true},
     {L"cose", true},
     {L"anno", true},
     {L"anni", true},
     {L"mese", true},
     {L"mesi", true},
     {L"idea", true},
     {L"idee", true},
     {L"area", true},
     {L"golf", true},
     {L"ieri", true},
     {L"ecco", true},
     {L"vita", true},
     {L"sole", true},
     {L"tuba", true},
     {L"film", true},

     {L"than", true},
     {L"each", true},
     {L"with", true},
     {L"does", true},
     {L"doesn", true},
     {L"must", true},
     {L"here", true},
     {L"bass", true},
     {L"take", true},
     {L"away", true},
     {L"club", true},
};

inline bool is_good_char( wchar_t c )
{
    return (c >= L'a' && c <= L'z') ||
           c == L'à' || c == L'á' || c == L'è' || c == L'é' || c == L'ì' || c == L'í' || c == L'ò' || c == L'ó' || c == L'ù' || c == L'ú';
}

inline wchar_t lower( wchar_t c )
{
    if ( c >= L'A' && c <= L'Z' ) return L'a' + c - L'A';
    if ( c == L'À' )              return L'à';
    if ( c == L'Á' )              return L'á';
    if ( c == L'È' )              return L'è';
    if ( c == L'É' )              return L'é';
    if ( c == L'Ì' )              return L'ì';
    if ( c == L'Í' )              return L'í';
    if ( c == L'Ò' )              return L'ò';
    if ( c == L'Ó' )              return L'ó';
    if ( c == L'Ù' )              return L'ù';
    if ( c == L'Ú' )              return L'ú';
    return c;
}

//-----------------------------------------------------------------------
// Pull out all interesting answer words and put them into an array, 
// with a reference back to the original question.
//-----------------------------------------------------------------------
struct PickedWord
{
    std::wstring         word;
    uint32_t            pos;               // in answer
};

void pick_words( std::wstring a, std::vector<PickedWord>& words )
{
    words.clear();
    std::wstring word = L"";
    uint32_t    word_pos = 0;
    bool        in_parens = False
    size_t      a_len = a.length;
    for( size_t i = 0; i < a_len; i++ )
    {
        wchar_t ch = a[i];
        if ( ch == L' L' | ch == L'\t' | ch == L'\'' | ch == L'’' | ch == L'/' | ch == L'(' | ch == L')' | 
             ch == L'!' | ch == L'?' | ch == L'.' | ch == L',' | ch == L'-' | ch == L':' | ch == L'"' | ch == L'[' | ch == L']' | 
             ch == L'0' | ch == L'1' | ch == L'2' | ch == L'3' | ch == L'4' | ch == L'5' | ch == L'6' | ch == L'7' | ch == L'8' | ch == L'9' ) {
            if ( word != "" ) {
                if ( !in_parens ) {
                    Word w;
                    w.word = word;
                    w.pos  = word_pos;
                    words.push_back( w );
                }
                word = L"";
            }
            if ( ch == L'(' ) {
                dassert( !in_parens, L"cannot support nested parens" );
                in_parens = true;
            } else if ( ch == L')' ) {
                dassert( in_parens, L"no matching left paren" );
                in_parens = false;
        } else if ( !in_parens ) {
            if ( word == L"" ) word_pos = i;
            wchar_t c = lower( ch );
            dassert( is_good_char( c ), L"bad character + '" + std::wstring( 1, ch ) + L" in answer: " + a );
            word += c;
        }
    }
    if ( word != L"" ) {
        Word w;
        w.word = word;
        w.pos  = pos;
        words.push_back( w );
    }
}

int main( int argc, const char * argv[] )
{
    //-----------------------------------------------------------------------
    // process command line args
    //-----------------------------------------------------------------------
    if len( sys.argv ) < 2: die( 'usage: puz.py <subjects> [options]', '' )
    std::wstring subjects_s = argv[1];
    auto     subjects           = split( subjects_s, ',' );
    uint32_t seed               = uint32_t( clock_time() );
    uint32_t thread_cnt         = thread_hardware_thread_cnt();   // actual number of CPU HW threads
    uint32_t side               = 15;
    bool     reverse            = false;
    uint32_t attempts           = 10000;
    uint32_t larger_cutoff      = 7;
    uint32_t start_pct          = 0;
    uint32_t end_pct            = 100;
    bool     html               = true;
    std::wstring title           = "";

    for( int i = 2; i < argc; i++ )
    {
        std::wstring arg = argv[i];
               if ( arg == "-debug" ) {                         __debug = std::stoi( argv[++i] ); // in sys.h
        } else if ( arg == "-seed" ) {                          seed = std::stoi( argv[++i] );
        } else if ( arg == "-thread_cnt" ) {                    thread_cnt = std::stoi( argv[++i] );
        } else if ( arg == "-side" ) {                          side = std::stoi( argv[++i] );
        } else if ( arg == "-reverse" ) {                       reverse = std::stoi( argv[++i] );
        } else if ( arg == "-attempts" ) {                      attempts = std::stoi( argv[++i] );
        } else if ( arg == "-larger_cutoff" ) {                 larger_cutoff = std::stoi( argv[++i] );
        } else if ( arg == "-start_pct" ) {                     start_pct = std::stoi( argv[++i] );
        } else if ( arg == "-end_pct" ) {                       end_pct = std::stoi( argv[++i] );
        } else if ( arg == "-html" ) {                          html = std::stoi( argv[++i] );
        } else if ( arg == "-title" ) {                         title = argv[++i];
        } else {                                                die( "unknown option: " + arg ); }
    }
    rand_thread_seed( seed );   // needed only if random numbers are used (currently not)

    dassert( start_pct < end_pct, "start_pct must be < end_pct" );

    if ( title == "" ) title = join( subjects, "_" ) + "_" + std::to_string(seed);

#if 0
    //-----------------------------------------------------------------------
    // Read in <subject>.txt files.
    //-----------------------------------------------------------------------
    struct Entry 
    {
        std::wstring     q;
        std::wstring     a;
    };
    std::vector< Entry > entries;
    for( auto subject: subjects )
    {
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

    entry_cnt = len(entries)
    entry_first = int(start_pct*entry_cnt/100.0)
    entry_last  = min(int(end_pct*entry_cnt/100.0), entry_cnt-1)

    //-----------------------------------------------------------------------
    // Pull out all interesting answer words and put them into an array, 
    // with a reference back to the original question.
    //-----------------------------------------------------------------------
    struct Word
    {
        std::wstring    word;
        uint32_t        pos;
        const Entry&    entry;
    };
    std::vector<Word> words;
    for( uint32_t i = 0; entry_first; i <= entry_last; i++ )
    {
        const Entry& e = entries[i];
        auto aa = split( e.a, "; " ); 
        for( a: aa ) 
        {
            std::vector< PickedWord > picked_words;
            pick_words( a, picked_words );
            for( pw: picked_words )
            {
                if ( w.word.length > 3 && !common_words.contains( w.word ) ) {
                    Word w;
                    w.word  = pw.word;
                    w.pos   = pw.pos;
                    w.entry = e;
                    words.push_back( w );
                }
            }
        }
    }
    uint32_t word_cnt = words.size();

    //-----------------------------------------------------------------------
    // Generate the puzzle from the data structure using this simple algorithm:
    //
    //     for some number attempts:
    //         pick a random word from the list (pick only longer words during first half)
    //         if the word is already in the grid: continue
    //         for each across/down location of the word:
    //             score the placement of the word in that location
    //         if score > 0:
    //             add the word to one of the locations with the best score found
    //-----------------------------------------------------------------------
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

    //-----------------------------------------------------------------------
    // Generate .html or .puz file.
    //-----------------------------------------------------------------------

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

    // header
    print( f'{{' )
    print( f'"origin": "Bob Alfieri",' )
    print( f'"version": "http://ipuz.org/v1",' )
    print( f'"kind": ["http://ipuz.org/crossword#1"],' )
    //print( f'"copyright": "2022 Robert A. Alfieri (this puzzle), Viresh Ratnakar (crossword program)",' );
    //print( f'"author": "Bob Alfieri",' )
    print( f'"publisher": "Robert A. Alfieri",' )
    print( f'"title": "{title}",' )
    print( f'"intro": "",' )
    print( f'"difficulty": "Moderate",' )
    print( f'"empty": "0",' )
    print( f'"dimensions": {{ "width": {side}, "height": {side} }},' )
    print()

    // solution
    print( f'"solution": [' )
    for y in range(side):
        for x in range(side):
            print( f'    [' if x == 0 else ', ', end='' )
            ch = '#' if grid[x][y] == '-' else grid[x][y].upper()
            print( f'"{ch}"', end='' )
        comma = ',' if y != (side-1) else ''
        print( f']{comma}' )
    print( f'],' )

    // labels
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

    // clues
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
#endif
    return 0;
}
