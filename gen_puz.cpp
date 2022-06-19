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
const std::map<std::string, bool> common_words = { 
     {"avere", true}, 
     {"averla", true},
     {"averlo", true},
     {"averle", true},
     {"averli", true},
     {"aver", true},
     {"essere", true}, 
     {"esserla", true}, 
     {"esserlo", true}, 
     {"esserle", true}, 
     {"esserli", true}, 
     {"stare", true},
     {"stai", true},
     {"stiamo", true},
     {"state", true},
     {"stanno", true},
     {"fare", true}, 
     {"farla", true},
     {"farlo", true},
     {"farle", true},
     {"farli", true},
     {"farsi", true},
     {"dare", true},
     {"come", true},
     {"così", true},
     {"sono", true}, 
     {"miei", true},
     {"tuoi", true},
     {"suoi", true},
     {"vuoi", true},
     {"dall", true},
     {"dalla", true},
     {"dallo", true},
     {"dagli", true}, 
     {"dalle", true}, 
     {"dell", true},
     {"della", true},
     {"dello", true},
     {"degli", true}, 
     {"delle", true}, 
     {"nell", true},
     {"nella", true},
     {"nello", true},
     {"negli", true}, 
     {"nelle", true}, 
     {"sull", true},
     {"sugli", true},
     {"sulla", true},
     {"sullo", true},
     {"sulle", true},
     {"all", true},
     {"alla", true},
     {"allo", true},
     {"alle", true},
     {"agli", true},
     {"cosa", true},
     {"cose", true},
     {"anno", true},
     {"anni", true},
     {"mese", true},
     {"mesi", true},
     {"idea", true},
     {"idee", true},
     {"area", true},
     {"golf", true},
     {"ieri", true},
     {"ecco", true},
     {"vita", true},
     {"sole", true},
     {"tuba", true},
     {"film", true},

     {"than", true},
     {"each", true},
     {"with", true},
     {"does", true},
     {"doesn", true},
     {"must", true},
     {"here", true},
     {"bass", true},
     {"take", true},
     {"away", true},
     {"club", true},
};

//-----------------------------------------------------------------------
// Read a line from a file w/o newline and return it as a string.
// Return "" if nothing else in the file.
//-----------------------------------------------------------------------
inline std::string readline( std::ifstream& in )
{
    std::string s = "";
    while( !in.eof() ) 
    {
        char c;
        if ( !in.get( c ) ) break;
        s += c;
        if ( c == '\n' ) break;
    }
    return s;
}

//-----------------------------------------------------------------------
// Pull out all interesting answer words and put them into an array, 
// with a reference back to the original question.
//-----------------------------------------------------------------------
struct PickedWord
{
    std::string         word;
    uint32_t            pos;               // in answer
};

void pick_words( std::string a, std::vector<PickedWord>& words )
{
    words.clear();
    std::string word = "";
    uint32_t    word_pos = 0;
    bool        in_parens = false;
    size_t      a_len = a.length();
    for( size_t i = 0; i < a_len; i++ )
    {
        char ch = a[i];
        if ( ch == ' ' || ch == '\t' || ch == '\'' || ch == '\'' || ch == '/' || ch == '(' || ch == ')' || 
             ch == '!' || ch == '?' || ch == '.' || ch == ',' || ch == '-' || ch == ':' || ch == '"' || ch == '[' || ch == ']' || 
             ch == '0' || ch == '1' || ch == '2' || ch == '3' || ch == '4' || ch == '5' || ch == '6' || ch == '7' || ch == '8' || ch == '9' ||
             ch == '\xe2' ) {
            if ( word != "" ) {
                if ( !in_parens ) {
                    PickedWord w;
                    w.word = word;
                    w.pos  = word_pos;
                    words.push_back( w );
                }
                word = "";
            }
            if ( ch == '\xe2' ) {
                ch = a[++i];
                dassert( ch == '\x80', "did not get 0x80 after 0xe2 for quote" );
                ch = a[++i];
                dassert( ch == '\x99', "did not get 0x99 after 0xe2 0x80 for quote" );
            } else if ( ch == '(' ) {
                dassert( !in_parens, "cannot support nested parens" );
                in_parens = true;
            } else if ( ch == ')' ) {
                dassert( in_parens, "no matching left paren" );
                in_parens = false;
            }
        } else if ( !in_parens ) {
            if ( word == "" ) word_pos = i;
            if ( ch != '\xc3' ) {
                // not 16-bit char
                if ( ch >= 'A' && ch <= 'Z' ) {
                    ch = 'a' + ch - 'A';
                }
                if ( ch < 'a' || ch > 'z' ) {
                    std::cout << "ERROR: bad character in answer at pos " << i << "\n";
                    for( size_t ii = 0; ii < a_len; ii++ )
                    {
                        ch = a[ii];
                        std::cout << ii << ": " << std::string( 1, ch ) << " (0x" << std::hex << int(uint8_t(ch)) << std::dec << ")\n";
                    }
                    exit( 1 );
                }
                word += ch;
            } else {
                // 16-bit char
                // allowed: àáèéìíòóùú
                word += ch;
                dassert( i < (a_len-1), "incomplete special character in answer: " + a );
                ch = a[++i];
                if ( ch >= '\x80' && ch <= '\x9f' ) {
                    ch = 0xa0 + ch - 0x80;              // make lower-case
                }
                dassert( ch == '\xa0' || ch == '\xa1' || ch == '\xa8' || ch == '\xa9' || ch == '\xac' || ch == '\xad' || ch == '\xb2' || ch == '\xb3' || ch == '\xb9' || ch == '\xba',
                         "bad special character in answer: " + a );
                word += ch;
            }
        }
    }
    if ( word != "" ) {
        PickedWord w;
        w.word = word;
        w.pos  = word_pos;
        words.push_back( w );
    }
}

int main( int argc, const char * argv[] )
{
    //-----------------------------------------------------------------------
    // process command line args
    //-----------------------------------------------------------------------
    if (argc < 2) die( "usage: puz.py <subjects> [options]" );
    std::string subjects_s = argv[1];
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
    std::string title           = "";

    for( int i = 2; i < argc; i++ )
    {
        std::string arg = argv[i];
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

    //-----------------------------------------------------------------------
    // Read in <subject>.txt files.
    //-----------------------------------------------------------------------
    std::regex ws1( "^\\s+" );
    std::regex ws2( "\\s+$" );
    struct Entry 
    {
        std::string     q;
        std::string     a;
    };
    std::vector< Entry > entries;
    for( auto subject: subjects )
    {
        std::string filename = subject + ".txt";
        std::ifstream Q( filename );
        dassert( Q.is_open(), "could not open file " + filename + " for input" );
        uint32_t line_num = 0;
        for( ;; )
        {
            std::string question = readline( Q );
            if ( question == "" ) break;
            line_num++;
            question = replace( question, ws1, "" );
            question = replace( question, ws2, "" );
            if ( question.length() == 0 or question[0] == '#' ) continue;

            std::string answer = readline( Q );
            answer = replace( answer, ws1, "" );
            answer = replace( answer, ws2, "" );
            dassert( answer.length() != 0, "question on line " + std::to_string(line_num) + " is not followed by a non-blank answer on the next line: " + question );
            line_num++;

            if ( reverse ) {
                std::string tmp = question;
                question = answer;
                answer = tmp;
            }

            Entry entry;
            entry.q = question;
            entry.a = answer;
            entries.push_back( entry );
        }
        Q.close();
    }

    uint32_t entry_cnt   = entries.size();
    uint32_t entry_first = float(start_pct)*float(entry_cnt)/100.0;
    uint32_t entry_last  = std::min( uint32_t( float(end_pct)*float(entry_cnt)/100.0 ), entry_cnt-1 );

    //-----------------------------------------------------------------------
    // Pull out all interesting answer words and put them into an array, 
    // with a reference back to the original question.
    //-----------------------------------------------------------------------
    struct Word
    {
        std::string     word;
        uint32_t        pos;
        const Entry *   entry;
    };
    std::vector<Word> words;
    for( uint32_t i = entry_first; i <= entry_last; i++ )
    {
        const Entry& e = entries[i];
        auto aa = split( e.a, ';' ); 
        for( auto a: aa ) 
        {
            std::vector< PickedWord > picked_words;
            pick_words( a, picked_words );
            for( auto pw: picked_words )
            {
                if ( pw.word.length() > 3 && common_words.find( pw.word ) == common_words.end() ) { 
                    Word w;
                    w.word  = pw.word;
                    w.pos   = pw.pos;
                    w.entry = &e;
                    words.push_back( w );
                }
            }
        }
    }
    uint32_t word_cnt = words.size();

#if 0
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
