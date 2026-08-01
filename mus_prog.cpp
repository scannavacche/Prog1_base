#include "mus_lib.hpp"
#include "mus_csr.hpp"

bool app_exec_cmd(const VecS &inColl, SongsArgs &args, VecS &outColl) {

    debug_argv_dump(args, "Nella exec ");
  
    //
    //  non serve piu' il parsing dei comandi qui perche' lo abbiamo gia' fatto in app_parse_arg
    //  ed abbiamo gia' args.cmdcode (da verificare se da tutti i rami della parse_arg)
    //
    // if (songs_parse_cmd(args.cmdstr, args.cmdcode)){  // e' qui che carichiamo il cod dell' ope richiesta

    if (args.cmdcode != SongsCmd::invalid) {

        switch (args.cmdcode) {

            case SongsCmd::listTest:
                outColl = collection_list(inColl); // il comando List non ha argomenti
                break;
            case SongsCmd::filterLen:
                outColl = collection_durata(inColl,validate_minute(args.subarg)); // stringa gia' filtrata convertibile in num  
                break; 
            case SongsCmd::filterAll: // riversa le canzoni che matchano una stringa in titolo o interprete
                outColl = collection_cerca(inColl, args.subarg);
                break;
            case SongsCmd::filterTit: // riversa solo le canzoni che matchano una stringa nel titolo 
                outColl = collection_cercat(inColl, args.subarg);
                break;
            case SongsCmd::filterInt: // riversa solo le canzoni che matchano una stringa nell' interprete 
                outColl = collection_cercai(inColl, args.subarg);
                break;
            case SongsCmd::filterAnP: // riversa solo le canzoni con un certo Anno di Pubblicazione (sub arg)
                outColl = collection_anno(inColl, std::atoi(args.subarg.c_str())); // string agia' filtrata convertibile in num
                break;
            case SongsCmd::sortbyAnP: 
                //
                // ordina la lista per anno di pubb e la riversa integrale
                // per questa operazione uso il bucket sort suggerito (siamo su naturali)
                outColl = collection_ordina(inColl);
                break;
            case SongsCmd::sortbyLen: 
                //
                // ordina la lista per durata crescente e la riversa integrale
                //
                outColl = collection_ordlen(inColl);
                break;
            case SongsCmd::getHelp:
                showHelp();
                exit(0);  // uscita obbligata: comando valido ma nessuna coll da leggere, deve evitare exit(1)
            case SongsCmd::invalid:  
                // solo per togliersi dai piedi un warning senza spegnerlo per ogni switch
                // in realta' invalid produce gia' una lista a console con l'avviso di comando non valido
                break;
            default:
                showError(SongsErr::errNotImplemented, "Fine");
                return false;
        }  // end switch (args.cmdcode) 
        
    } else { // args.cmdcode == invalid
        
        for (song currSong : inColl) {
            debug_song_dump(currSong);
        }
        showError(errInvalidOp, args.cmdstr);
        return false;

    }
    return true;
}


int main(int argc, char *argv[])
{
    SongsArgs argstr;
    VecS inCollection, outCollection;

    if (app_parse_args(argc, argv, argstr)) {
        if (collection_read (inCollection, argstr)) {
            debug_argv_dump(argstr, "Prima del dispatcher ");
            if (app_exec_cmd(inCollection, argstr, outCollection)) { 
                //
                // if (!outCollection.empty()) collection_write(outCollection, argstr);
                //
                // tolto il controllo da qui, lo affidiamo al loop di tipo "for each" 
                // cosi' con output nullo produce comunque file da 0 bytes 
                // ed elimina il contenuto precedente che potrebbe triggerare errore
                //
                collection_write(outCollection, argstr); 
            } else {
                return 1; // non riconosciuta, fallita o non implementata esecuzione dei comando
            }
        } else {
            return 1; // mancata lettura della collection in ingresso, segnalato
        };
        return 0;
    } else {    
        return 1; // parsing degli argomenti fallito (si avvale di check finale su invalid, rivedere)
    }
    
}