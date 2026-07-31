#ifndef MUSCSR_H
#define MUSCSR_H

#include "mus_lib.hpp"


//
// Libreria specifica delle funzioni di servizio ai comandi esterni 
// e richieste esplicitamente [CSR Command Service Routines]
// 

bool collection_read(VecS &inCollection, SongsArgs &argstr);
void collection_write(const VecS &outCollection, SongsArgs &args);


VecS collection_anno(const VecS &inColl, int anno);             // cerca canzoni di anno
VecS collection_cerca(const VecS &inColl, std::string tstr);    // cerca tstr in titolo o interprete
VecS collection_cercat(const VecS &inColl, std::string tstr);   // cerca tstr in titolo
VecS collection_cercai(const VecS &inColl, std::string tstr);   // cerca tstr in interprete
VecS collection_durata(const VecS &inColl, int minuti);         // cerca durata sino a minuti
VecS collection_list(VecS inColl);                              // test: Lista il file di input
VecS collection_ordina(const VecS &inColl);                     // ordina per anno (bucket sort)
VecS collection_ordlen(const VecS &inColl);                     // ordina per durata totale (std::stable_sort)

bool app_parse_args(int argc, char *argv[], SongsArgs& args); // parse degli argv[], rende true se ok

#endif