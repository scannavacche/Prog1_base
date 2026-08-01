#ifndef MUSLIB_H
#define MUSLIB_H

//
// Libreria di base, prototipi e tipi comuni 
// 

#ifdef DEBUG
#define DBG(x) do { std::cerr << x << '\n'; } while (false)
#else
#define DBG(x) do {} while (false)
#endif

/*
*  macro che sostituisce le std::cout di debug lasciando libera 
*  la composizione degli argomenti con la sintassi:

    DBG("des0 " << cmd0
        << " des1: " << cmd1
        << " des2: " << cmd2);

*  che con la direttiva DEBUG attiva diventa 

std::cerr << "des0: " << cmd0
          << " des1: " << cmd1
          << " des2: " << cmd2
          << '\n';

*/

#include <cstdio>       // std::printf, std::fopen
#include <string>       // std::string, std::getline, std::stoi, std::stod
#include <vector>       // std::vector
#include <iostream>     // std::cout, std::cin, std::cerr
#include <fstream>      // std::ifstream, std::ofstream	
#include <sstream>      // std::stringstream, std::istringstream	
#include <algorithm>    // std::sort, std::find	
#include <iomanip>      // std::setw, std::setprecision	        
#include <cctype>       // std::tolower, std::isdigit	            
#include <cmath>        // std::abs
#include <regex>        // std::regex (check anno e minuto)

using inFile = std::ifstream;
using outFile = std::ofstream; 
using VecStr = std::vector<std::string>;

// record singola canzone

struct song {
    std::string titolo;
    std::string interprete;

    int anno;           // E' richiesta la riproduzione fedele in fileout dei dati di input
    int runtime_min;    // Solo due record hanno dati numerici con zero-padding sui secondi.
    int runtime_sec;    // un caso o un test di compliance? La soluzione conservativa e'
    // lavorare con gli interi ma preservare le stringhe originali per la scrittura finale.

    std::string anno_raw;
    std::string runtime_min_raw;
    std::string runtime_sec_raw;
};  

using VecS = std::vector<song>;

// codici normalizzati per i comandi richiesti come stringa

enum SongsCmd {
    listTest,   // riversa in output la stessa lista di input
    filterLen,  // riversa solo le canzoni con runtime sino a N minuti 
    filterAll,  // riversa solo le canzoni che matchano una stringa nel titolo o nell' interprete
    filterTit,  // riversa solo le canzoni che matchano una stringa nel titolo
    filterInt,  // riversa solo le canzoni che matchano una stringa nell' interprete
    filterAnP,  // riversa solo le canzoni con un certo Anno di Pubblicazione
    sortbyAnP,  // ordina la lista per anno di pubb e la riversa integrale 
    sortbyLen,  // ordina la lista per durata crescente e la riversa integrale
    getHelp,    // richiede l'help per gli argomenti a linea di comando
    invalid     // comando non riconosciuto o parametro non riconosciuto 
};

//  record di parametri attuali dalla linea di comando

struct SongsArgs {
    std::string infile;
    std::string cmdstr; // stringa letta e ripulita
    SongsCmd cmdcode;   // codificato in enum val
    std::string subarg;
    std::string outfile;
};


// i comandi sono indicabili in argv[2] con una certa tolleranza, mixed case e con alias

struct CmdAlias {
    const char *name;
    SongsCmd cmd;
};

// tavola degli alias (post normalizzazione)

const CmdAlias CmdTable[] = {

    {"DURATA",      SongsCmd::filterLen},   // enum 0

    {"CERCA",       SongsCmd::filterAll},   // enum 1

    {"ANNO",        SongsCmd::filterAnP},   // enum 2 

    {"ORDINA",      SongsCmd::sortbyAnP},   // enum 3

//  Extra Commands

    {"LIST",        SongsCmd::listTest},    // enum 4
    {"L",           SongsCmd::listTest},

    {"ORDLEN",      SongsCmd::sortbyLen},   // enum 5
    {"OL",          SongsCmd::sortbyLen},

    {"CERCAT",      SongsCmd::filterTit},   // enum 6

    {"CERCAI",      SongsCmd::filterInt},   // enum 7

    {"HELP",        SongsCmd::getHelp},     // enum 8

    {"ERROR",       SongsCmd::invalid}      // enum 9  e' qui solo di guardia allo steccato
                                            // qualsiasi stringa matchera' prima o passera' oltre 
    
};

extern const std::string helpMessage;

// codici di errore per selezionare il messaggio 

enum SongsErr {
    OK,
    errNotFoundIn,
    errNotOKOut,
    errNot4Args,
    errEmptyColl,
    errInvalidOp,
    errMinMax,
    errUnderdate, 
    errNullValue,
    errNotImplemented,

    errCount // contatore di items disponibili in enum
};


// funzioni di base (string management)

bool null_string(std::string s);
std::string text_normalize(const std::string &inparm) ;
bool text_match(std::string fullstr, std::string substr);
int validate_minute(const std::string& text);
int validate_seconds(const std::string& text);
std::string zero_fill(int number, int length); 

// file managemnt (close con overloading if/of, read open, rewrite open)

void file_close(std::ifstream& file);
void file_close(std::ofstream& file); // e un caso di overloading non ce lo vogliamo concedere?
inFile file_ropen(std::string song_filename_in); // apre il file di input in modeo r 
std::ostream& file_wopen(const std::string& filename, outFile& fileOut); // tenta di aprire in scrittura

// songs management ed helper di collection

void coll_find_limits (const VecS &s, int& amin, int& amax);
bool songs_parse_cmd(const std::string& text, SongsCmd& cmd);
int songs_read_int_field(std::istringstream& record, std::string &raw_val);
bool songs_read_fields(std::string line, song &current);
bool songs_read_line (inFile &file, std::string &line) ;
int songs_read_int_field(std::istringstream& record);
long song_runtime_total (const song &s); // rende il runtime totale in secondi 
void song_write(std::ostream& outSongs, song currSong);

// debug, autoestinguenti (attive solo con -DDEBUG)

void debug_argv_dump(SongsArgs args, std::string msg);
void debug_song_dump(song s);
void debug_runtime_len(song s);
void showError(SongsErr err, const std::string &detail);
void showHelp();

// regalino dall' AI (regexp varie)

bool parse_int(const std::string& text);
bool parse_year(const std::string& text);

// deprecated

void songs_split_cmd(std::string inparm, std::string &outcmd, std::string &outval); // split di argv[2] se cmd:val

#endif
