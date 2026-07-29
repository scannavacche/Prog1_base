#ifndef MUS_H
#define MUS_H

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

// codici normalizzati per i comandi richiesti come stringa

enum SongsCmd {
    listTest,   // riversa in output la stessa lista di input
    filterLen,  // riversa solo le canzoni con runtime min eccedente N minuti (sub arg)
    filterTit,  // riversa solo le canzoni che matchano una stringa nel titolo (sub arg)
    filterInt,  // riversa solo le canzoni che matchano una stringa nell' interprete (sub arg)
    filterAnP,  // riversa solo le canzoni con un certo Anno di Pubblicazione (sub arg)
    sortbyAnP,  // ordina la lista per anno di pubb e la riversa integrale 
    invalid     // comando non riconosciuto 
};

// i comandi sono indicabili in argv[2] con una certa tolleranza, mixed case e con alias

struct CmdAlias {
    const char *name;
    SongsCmd cmd;
};

// tavola degli alias (post normalizzazione)

const CmdAlias CmdTable[] = {
    {"LIST",        SongsCmd::listTest},
    {"L",           SongsCmd::listTest},
    {"SHOW",        SongsCmd::listTest},

    {"SORT",        SongsCmd::sortbyAnP},
    {"S",           SongsCmd::sortbyAnP},

    {"FILTERLEN",   SongsCmd::filterLen},
    {"FILTERL",     SongsCmd::filterLen},
    {"FL",          SongsCmd::filterLen},

    {"FILTERTIT",   SongsCmd::filterTit},
    {"FILTERT",     SongsCmd::filterTit},
    {"FT",          SongsCmd::filterTit},

    {"FILTERINT",   SongsCmd::filterAnP},
    {"FILTERI",     SongsCmd::filterAnP},
    {"FI",          SongsCmd::filterAnP},

    {"FILTERANN",   SongsCmd::filterLen},
    {"FILTERA",     SongsCmd::filterLen},
    {"FA",          SongsCmd::filterLen}
};

// codici di errore per selezionare il messaggio 

enum SongsErr {
    OK,
    errNotFoundIn,
    errNotOKOut,
    errNot4Args,

    errCount // contatore di items disponibili in enum
};

// e relativa chiamata al messaggio di errore

void showError(SongsErr err, const std::string& detail);

// record singola canzone

struct song {
    std::string titolo;
    std::string interprete;
    int anno;
    int runtime_min;
    int runtime_sec;
};

//  record di parametri attuali dalla linea di comando
struct SongsArgs {
    std::string infile;
    std::string cmdcode;
    std::string outfile;
};
struct SongsArgs {
    std::string infile;
    std::string cmdstr; // stringa letta
    SongsCmd cmdcode;   // codificato in enum val
    std::string subarg;
    std::string outfile;
};

using VecS = std::vector<song>;
using VecStr = std::vector<std::string>;

using inFile = std::ifstream;
using outFile = std::ofstream;

long song_runtime_total (song s); // rende il runtime totale in secondi 

void songs_split_cmd(std::string inparm, std::string &outcmd, std::string &outval); // split di argv[2] se cmd:val
SongsCmd songs_code_cmd(std::string inparm) ; // normalizza e codifica la string cmd in val di enum univoci
void songs_normalize_cmd(std::string inparm) ;
bool songs_parse_args(int argc, char *argv[], SongsArgs& args); // parse degli argv[], rende true se ok
bool songs_parse_cmd(const std::string& text, SongsCmd& cmd);


std::ifstream song_ropen(std::string song_filename_in); // apre il file di input in modeo r 
std::ofstream song_fopen(std::string song_filename_out); // apre il file di output in modo w

void song_close(std::ifstream& file); // chiude il file anche se non e' necessario con ifstream, ofstream

#endif
