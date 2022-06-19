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
//   {"così", true},
     {"cos4", true},    // transformed
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
class PickedWord
{
public:
    std::string         word;
    uint32_t            pos;                    // in answer
    uint32_t            pos_last;               // in answer

    inline PickedWord( std::string word, uint32_t pos, uint32_t pos_last ) : word(word), pos(pos), pos_last(pos_last) {}
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
                    words.push_back( PickedWord( word, word_pos, i-1 ) );
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
                dassert( i < (a_len-1), "incomplete special character in answer: " + a );
                ch = a[++i];
                if ( ch >= '\x80' && ch <= '\x9f' ) {
                    ch = 0xa0 + ch - 0x80;              // make lower-case
                }

                // Map these characters to 0..9 so we use only one byte to represent them,
                // which will make creation of the puzzle much easier.
                // When we go to write out the puzzle, we'll these characters back.
                switch( ch )
                {
                    case '\xa0': ch = '0'; break;
                    case '\xa1': ch = '1'; break;
                    case '\xa8': ch = '2'; break;
                    case '\xa9': ch = '3'; break;
                    case '\xac': ch = '4'; break;
                    case '\xad': ch = '5'; break;
                    case '\xb2': ch = '6'; break;
                    case '\xb3': ch = '7'; break;
                    case '\xb9': ch = '8'; break;
                    case '\xba': ch = '9'; break;
                    default:     die( "bad special character in answer:" + a ); break;
                }
                word += ch;
            }
        }
    }
    if ( word != "" ) {
        words.push_back( PickedWord( word, word_pos, a_len-1 ) );
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
        uint32_t        len;
        uint32_t        pos;
        uint32_t        pos_last;
        std::string     a;
        const Entry *   entry;
    };
    std::vector<Word> words;
    for( uint32_t i = entry_first; i <= entry_last; i++ )
    {
        const Entry& e = entries[i];
        auto aa = split( e.a, ';' ); 
        for( auto _a: aa ) 
        {
            std::string a = replace( _a, ws1, "" );
            std::vector< PickedWord > picked_words;
            pick_words( a, picked_words );
            for( auto pw: picked_words )
            {
                if ( pw.word.length() > 3 && common_words.find( pw.word ) == common_words.end() ) { 
                    Word w;
                    w.word     = pw.word;
                    w.pos      = pw.pos;
                    w.pos_last = pw.pos_last;
                    w.a        = a;
                    w.entry    = &e;
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
    struct Clue
    {
        std::string     word;
        uint32_t        pos;
        uint32_t        pos_last;
        std::string     a;
        const Entry *   entry;
        uint32_t        x;
        uint32_t        y;
        bool            is_across;
        uint32_t        num;
    };
    char ** grid        = new char *[side];
    char ** across_grid = new char *[side];
    char ** down_grid   = new char *[side];
    Clue ***clue_grid   = new Clue **[side];
    for( uint32_t x = 0; x < side; x++ )
    {
        grid[x]        = new char[side];
        across_grid[x] = new char[side];
        down_grid[x]   = new char[side];
        clue_grid[x]   = new Clue*[side];
        for( uint32_t y = 0; y < side; y++ )
        {
            grid[x][y]        = '-';
            across_grid[x][y] = '-';
            down_grid[x][y]   = '-';
            clue_grid[x][y]   = new Clue[2];    // 1=across, 0=down
        }
    }

    std::map<const Entry *, bool> entries_used;
    float large_frac = float(0 + rand_n( 80 )) / 100.0;
    uint32_t attempts_large = float(attempts) * large_frac;
    for( uint32_t i = 0; i < attempts; i++ ) 
    {
        uint32_t wi = rand_n( word_cnt );
        Word& info = words[wi];
        const Entry *entry = info.entry;
        if ( entries_used.find( entry ) != entries_used.end() ) continue;

        std::string  word = info.word;
        uint32_t     word_len = word.length();
        if ( i < attempts_large && word_len < larger_cutoff ) continue;
        const char * word_cs = word.c_str();

        uint32_t     pos      = info.pos;
        uint32_t     pos_last = info.pos_last;
        std::string  a        = info.a;

        Clue best;
        uint32_t best_score = 0;

        for( uint32_t x = 0; x < side; x++ ) 
        {
            for( uint32_t y = 0; y < side; y++ ) 
            {
                if ( (x + word_len) <= side ) {
                    // score across
                    uint32_t score = (y == 0 || y == (side-1)) ? 5 : 1; 
                    if ( x == 0 || (x+word_len-1) == (side-1) ) score++;
                    for( uint32_t ci = 0; ci < word_len; ci++ ) 
                    {
                        if ( across_grid[x+ci][y] != '-' ||
                             (ci == 0 && x > 0 && grid[x-1][y] != '-') || 
                             (ci == (word_len-1) && (x+ci+1) < side && grid[x+ci+1][y] != '-') ) {
                            score = 0;
                            break;
                        }
                        char c  = word_cs[ci];
                        char gc = grid[x+ci][y];
                        if ( c == gc ) {
                            score++;
                        } else if ( gc != '-' ||
                                    (y > 0 and grid[x+ci][y-1] != '-') || 
                                    (y < (side-1) and grid[x+ci][y+1] != '-') ) {
                            score = 0;
                            break;
                        }
                    }
                    if ( score != 0 && score > best_score ) {
                        best.word      = word;
                        best.pos       = pos;
                        best.pos_last  = pos_last;
                        best.a         = a;
                        best.entry     = entry;
                        best.x         = x;
                        best.y         = y;
                        best.is_across = true;
                        best_score     = score;
                    }
                }

                if ( (y + word_len) <= side ) {
                    // score down
                    uint32_t score = (x == 0 || x == (side-1)) ? 5 : 1;
                    if ( y == 0 || (y+word_len-1) == (side-1) ) score++;
                    for( uint32_t ci = 0; ci < word_len; ci++ )
                    {
                        if ( down_grid[x][y+ci] != '-' || 
                             (ci == 0 && y > 0 && grid[x][y-1] != '-') || 
                             (ci == (word_len-1) && (y+ci+1) < side && grid[x][y+ci+1] != '-') ) {
                            score = 0;
                            break;
                        }
                        char c  = word_cs[ci];
                        char gc = grid[x][y+ci];
                        if ( c == gc ) {
                            score++;
                        } else if ( gc != '-' || 
                                    (x > 0 && grid[x-1][y+ci] != '-') || 
                                    (x < (side-1) && grid[x+1][y+ci] != '-') ) {
                            score = 0;
                            break;
                        }
                    }
                    if ( score != 0 && score > best_score ) {
                        best.word      = word;
                        best.pos       = pos;
                        best.pos_last  = pos_last;
                        best.a         = a;
                        best.entry     = entry;
                        best.x         = x;
                        best.y         = y;
                        best.is_across = false;
                        best_score     = score;
                    }
                }
            }
        }

        if ( best_score > 0 ) {
            entries_used[entry] = true;
            uint32_t x = best.x;
            uint32_t y = best.y;
            bool     is_across = best.is_across;
            for( uint32_t ci = 0; ci < word_len; ci++ ) 
            {
                if ( is_across ) {
                    grid[x+ci][y] = word[ci];
                    across_grid[x+ci][y] = word[ci];
                } else {
                    grid[x][y+ci] = word[ci];
                    down_grid[x][y+ci] = word[ci];
                }
            }
            dassert( clue_grid[x][y][is_across].word == "", "already have a clue in place" );
            clue_grid[x][y][is_across] = best;
        }
    }

    //-----------------------------------------------------------------------
    // Generate .html or .puz file.
    //-----------------------------------------------------------------------

    if ( html ) {
        std::cout << "<!DOCTYPE html>\n";
        std::cout << "<html lang=\"en\">\n";
        std::cout << "<head>\n";
        std::cout << "<meta charset=\"utf-8\"/>\n";
        std::cout << "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"/>\n";
        std::cout << "<link rel=\"stylesheet\" type=\"text/css\" href=\"exolve-m.css?v1.35\"/>\n";
        std::cout << "<script src=\"exolve-m.js?v1.35\"></script>\n";
        std::cout << "<script src=\"exolve-from-ipuz.js?v1.35\"></script>\n";
        std::cout << "\n";
        std::cout << "<title>Test-Ipuz-Solved</title>\n";
        std::cout << "\n";
        std::cout << "</head>\n";
        std::cout << "<body>\n";
        std::cout << "<script>\n";
        std::cout << "let ipuz =\n";
    }

    // header
    std::cout << "{\n";
    std::cout << "\"origin\": \"Bob Alfieri\",\n";
    std::cout << "\"version\": \"http://ipuz.org/v1\",\n";
    std::cout << "\"kind\": [\"http://ipuz.org/crossword#1\"],\n";
    //std::cout << "\"copyright\": \"2022 Robert A. Alfieri (this puzzle), Viresh Ratnakar (crossword program)\",\n";
    //std::cout << "\"author\": \"Bob Alfieri\",\n";
    std::cout << "\"publisher\": \"Robert A. Alfieri\",\n";
    std::cout << "\"title\": \"" << title << "\",\n";
    std::cout << "\"intro\": \"\",\n";
    std::cout << "\"difficulty\": \"Moderate\",\n";
    std::cout << "\"empty\": \"0\",\n";
    std::cout << "\"dimensions\": { \"width\": " << side << ", \"height\": " << side << " },\n";
    std::cout << "\n";

    // solution
    std::cout << "\"solution\": [\n";
    for( uint32_t y = 0; y < side; y++ )
    {
        for( uint32_t x = 0; x < side; x++ )
        {
            if ( x == 0 ) {
                std::cout << "    [";
            } else {
                std::cout << ",";
            }
            std::cout << "\"";
            char ch = grid[x][y];
            if ( ch == '-' ) {
                std::cout << "#";
            } else if ( ch >= 'a' && ch <= 'z' ) {
                ch = 'A' + ch - 'a';
                std::cout << ch;
            } else {
                // convert back to special character and make it uppercase
                dassert( ch >= '0' && ch <= '9', "unexpected special char in grid" );
                switch( ch )
                {
                    case '0': std::cout << "À"; break;
                    case '1': std::cout << "Á"; break;
                    case '2': std::cout << "È"; break;
                    case '3': std::cout << "É"; break;
                    case '4': std::cout << "Ì"; break;
                    case '5': std::cout << "Í"; break;
                    case '6': std::cout << "Ò"; break;
                    case '7': std::cout << "Ó"; break;
                    case '8': std::cout << "Ù"; break;
                    case '9': std::cout << "U'"; break;
                    default:  die( "something is wrong" ); break;
                }            
            }
            std::cout << "\"";
        }
        std::cout << "]";
        if ( y != (side-1) ) std::cout << ",";
        std::cout << "\n";
    }
    std::cout << "],\n";

    // labels
    std::cout << "\"puzzle\": [\n";
    uint32_t clue_num = 1;
    for( uint32_t y = 0; y < side; y++ )
    {
        for( uint32_t x = 0; x < side; x++ )
        {
            if ( x == 0 ) {
                std::cout << "    [";
            } else {
                std::cout << ", ";
            }
            if ( clue_grid[x][y][0].word != "" || clue_grid[x][y][1].word != "" ) {
                std::cout << clue_num;
                clue_grid[x][y][0].num = clue_num;
                clue_grid[x][y][1].num = clue_num;
                clue_num++; 
            } else if ( grid[x][y] != '-' ) {
                std::cout << " 0";
            } else {
                std::cout << "\"#\"";
            }
        }
        std::cout << "]";
        if ( y != (side-1) ) std::cout << ",";
        std::cout << "\n";
    }
    std::cout << "]," << "\n";

    // clues
    std::cout << "\"clues\": {\n";
    for( uint32_t i = 0; i < 2; i++ )
    {
        bool        is_across = i == 0;
        std::string which_mc = is_across ? "Across" : "Down";
        std::cout << "    \"" << which_mc << "\": [";
        bool have_one = false;
        for( uint32_t y = 0; y < side; y++ )
        {
            for( uint32_t x = 0; x < side; x++ )
            {
                const Clue& cinfo = clue_grid[x][y][is_across];
                if ( cinfo.word == "" ) continue;
                if ( have_one ) std::cout << ", "; 
                have_one = true;
                std::cout << "\n";
                uint32_t     num    = cinfo.num;
                std::string  word   = cinfo.word;
                uint32_t     first  = cinfo.pos;
                uint32_t     last   = cinfo.pos_last;
                std::string  a      = cinfo.a;
                std::string  q      = cinfo.entry->q;
                std::string  a_     = "";
                for( uint32_t j = 0; j < a.length(); j++ ) 
                {
                    if ( j >= first && j <= last ) {
                        if ( (j-first) < word.length() ) a_ += "_";
                    } else {
                        a_ += a[j];
                    }
                }
                std::cout << "        [" << num << ", \"" << q << " ==> " << a_ << "\"]";
            }
        }
        std::cout << "\n    ]";
        if ( is_across ) std::cout << ",";
        std::cout << "\n";
    }
    std::cout << "},\n";
    std::cout << "}\n";

    if ( html ) {
        std::cout << "text = exolveFromIpuz(ipuz)\n";
        //std::cout << "text += '\\n    exolve-option: allow-chars:ÀÁÈÉÌÍÒÓÙÚ\\n'\n";
        std::cout << "text += '\\n    exolve-language: it Latin\\n'\n";
        std::cout << "text += '\\n    exolve-end\\n'\n";
        std::cout << "createExolve(text)\n";
        std::cout << "</script>\n";
        std::cout << "</body>\n";
        std::cout << "</html>\n";
    }

    return 0;
}
