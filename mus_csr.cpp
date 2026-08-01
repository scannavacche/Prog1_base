//
// Libreria specifica delle funzioni di servizio ai comandi esterni 
// e richieste esplicitamente [CSR Command Service Routines]
// 

#include "mus_lib.hpp"

bool collection_read(VecS &inCollection, SongsArgs &argstr) {

    song currSong;
    std::string line;
    inFile inSongs = file_ropen(argstr.infile);

    while (songs_read_line(inSongs, line)) {
        if (songs_read_fields(line, currSong)) {
            inCollection.push_back(currSong);
        }             
    }
    file_close(inSongs);

    if (inCollection.empty()) {
        showError(SongsErr::errEmptyColl, "");
        return false;
    } else {
        return true;
    }

};

void collection_write(const VecS &outCollection, SongsArgs &args){
    //
    // prepara uno stream di output,  double face file/console
    //
    outFile outFileHandle; 
    //
    // comunque vada, sara' su un ostream
    // questo ci impone di maneggiare il file con outFileHandle (open, close)
    // e di scrivere sullo stream con la redirezione << (essendo ostream come std::cout)
    // questo ci permette di scrivere su file se indicato con fileoutput.csv 
    // oppure su std::cout se il file non e' stato indicato tra gli argomenti 
    //
    std::ostream& outSongs = file_wopen(args.outfile, outFileHandle); 
    for (const song &s : outCollection) {
        song_write(outSongs, s); // write su ostream outSongs con redirect <<
    };
    file_close(outFileHandle);      //  e file handling su ofstream outfileHandle
}


VecS collection_anno(const VecS &inColl, int anno){
    VecS outC;
    for (const song &currSong : inColl) {
        if (currSong.anno == anno) {
            outC.push_back(currSong);
            debug_song_dump(currSong);
        }
    }
    return outC;
}

VecS collection_cerca(const VecS &inColl, std::string tstr){
    VecS outC;
    for (const song &currSong : inColl) {
        if (text_match(currSong.titolo, tstr) ||
            text_match(currSong.interprete, tstr))   {
            outC.push_back(currSong);
            debug_song_dump(currSong);

        }
    }
    return outC;
}

VecS collection_cercat(const VecS &inColl, std::string tstr) {
    VecS outC;
    for (const song &currSong : inColl) {
        if (text_match(currSong.titolo, tstr))   {
            outC.push_back(currSong);
            debug_song_dump(currSong);
        }
    }
    return outC;
}

VecS collection_cercai(const VecS &inColl, std::string tstr) {
    VecS outC;
    for (const song &currSong : inColl) {
        if (text_match(currSong.interprete, tstr))   {
            outC.push_back(currSong);
            debug_song_dump(currSong);
        }
    }
    return outC;
}

VecS collection_durata(const VecS &inColl, int minuti){
    VecS outC;
    long tlen = minuti * 60L;  // ci portiamo a longint, non costa nulla
    for (const song &currSong : inColl) {

        // DBG("Test " << song_runtime_total(currSong) << " <= " << tlen);

        if (song_runtime_total(currSong) <= tlen) {
            outC.push_back(currSong);
            debug_song_dump(currSong);
        }
    }
    return outC;
}

VecS collection_list(VecS inColl){
    VecS outC;
    for (const song &currSong : inColl) {
        outC.push_back(currSong);
        debug_song_dump(currSong);
    }
    return outC;
};

VecS collection_ordina(const VecS &inColl){
    VecS outColl;
    int amin, amax;
    coll_find_limits(inColl, amin, amax);
    if (amax >= amin) {
        std::vector<VecS> aIndex(1 + amax - amin);
        //
        // prima classifica le canzoni per anno di pubblicazione
        //
        for (const song &s : inColl) {
            int icurr = s.anno - amin; 
            if (icurr < 0) {
                showError(SongsErr::errUnderdate, "Anno " + std::to_string(s.anno)+ " precede min: " + std::to_string(amin));
                exit(1);
            } else {
                aIndex[icurr].push_back(s);
            }
        }
        // 
        // e poi lo visita  come se ogni nodo annuale fosse una nmuova lista
        //
        // for (int i = 0; i <= (amax-amin); i++) {
        for (std::size_t i = 0; i < aIndex.size(); ++i) {  // meglio cosi', unica fonte
            for (const song &s : aIndex[i]) outColl.push_back(s);
        }

    } else {
        showError(SongsErr::errMinMax, "Anni da " + std::to_string(amin)+ " a " + std::to_string(amax));
        exit(1);
    }
    return outColl;
}

VecS collection_ordlen(const VecS &inColl) {
    // consideriamo l' indice pseudocontinuo, quindi niente bucket sort
    // per mantenere tra uguali l'ordine originale, proviamo std::stable_sort
    // esperimento degli esperimenti, una funzione lambda per il confronto usando funzione esterna

    VecS outColl = inColl;

    std::stable_sort(
        outColl.begin(),
        outColl.end(),
        [](const song& a, const song& b) {
            return song_runtime_total(a) < song_runtime_total(b);
        }
    );
    return outColl;
}


bool app_parse_args(int argc, char *argv[], SongsArgs& args)
{
    //
    // nuova versione del parser di linea di comando, 
    // lavora progressivo argomento per argomento invece di confidare in argc
    // ad ogni argomento caricato, se la posizione lo permette, verifica se e' un comando
    // nel caso, valuta l'arieta' e procede con l'eventuale operando previsto
    // esaurite le posizioni comandate, verifica la presenza del fileoutput.csv opzionale
    // se il comando e' in pos 1 (help) non processa altro
    // se il comando e' in pos 2 (operazione) forka:
    // - operazione 0-aria (ordina, list, ordlen) cerca fileout in pos 3
    // - operazione 1-aria (anno, durata cerca, cercat, cercai) 
    //              separa operando stringa/numerico e cerca fileout in pos 4
    // 

    // adozione di macro controllata da CXXFLAG -DDEBUG   

    DBG("argc: " << argc);

    if (argc > 0) DBG("argv[0]: " << argv[0]);
    if (argc > 1) DBG("argv[1]: " << argv[1]);
    if (argc > 2) DBG("argv[2]: " << argv[2]);
    if (argc > 3) DBG("argv[3]: " << argv[3]);
    if (argc > 4) DBG("argv[4]: " << argv[4]);        


    //
    // inizializza pronto per andarsene se non trova di meglio (se non varia, stampa err ed help in uscita)
    //
    SongsCmd probe_cmd = SongsCmd::invalid;  

    if (argc > 1) {
        //
        // c'e' qualcosa oltre al nome dell' eseguibile
        //
        if (songs_parse_cmd(argv[1], probe_cmd)) {  // e' un comando?
            //
            // e' un comando valido in prima posizione, puo' essere solo <help>
            //
            if (probe_cmd == SongsCmd::getHelp)  { 
                showHelp(); 
                exit(0);  // scelta obbligata, comando valido ma non per leggere coll  
            } else { // ha trovato un comando valido in posizione [1] sbagliata
                DBG("E' un comando in 1 ma non e' HELP: " << argv[1] << " = " << probe_cmd);
                probe_cmd = SongsCmd::invalid;
            };
        } else {   // bene, non e' un comando, assumiamo che sia fileinput.csv

            args.infile = argv[1];   // se non e' help lo consideriamo nome del file
            //
            // ora argv[2] puo' essere un comando senza arg o con un arg, filtriamo con switch, estendibile
            //
            DBG("Assumiamo che sia il file: " << argv[1]);

            if ((argc > 2) && songs_parse_cmd(argv[2], probe_cmd)) {  // e' un comando?
                //
                // ha due argonmenti ed in seconda pos 
                // c'e' un comando valido, consideriamolo con la sua arieta'
                //
                switch (probe_cmd) {
                    //
                    // comandi in argv[2] senza parametro
                    //
                    case SongsCmd::listTest:
                    case SongsCmd::sortbyAnP:
                    case SongsCmd::sortbyLen:
                        args.cmdstr = argv[2];
                        args.cmdcode = probe_cmd;
                        if (argc == 4) args.outfile = argv[3]; // se non c'e' resta nullstr da init
                        break;
                    //
                    // comandi in argv[2] con parametro in argv[3]
                    //
                    case SongsCmd::filterAll:
                    case SongsCmd::filterTit:
                    case SongsCmd::filterInt:
                        //
                        // comandi con argomento stringa
                        //
                        if (argc <= 3) { // se non c'e', de che stamo a parla' ?
                            probe_cmd = SongsCmd::invalid;
                            break; // va ad err + help
                        }
                        // il terzo argomento dev'esserci 

                        args.cmdstr = std::string(argv[2]) + " " + std::string(argv[3]);
                        args.cmdcode = probe_cmd;

                        if (null_string(argv[3])) {
                            showError(errNullValue, args.cmdstr);
                            return false; // non ho dato nulla, non stavo cercando nulla
                        };
                        args.subarg = argv[3];  // buona la stringa per la ricerca
                        
                        if (argc >= 5) args.outfile = argv[4]; // se non c'e' resta nullstr da init

                        break;

                    case SongsCmd::filterAnP:  // argomento e' un anno/data
                    case SongsCmd::filterLen:  // argomento e' un numero di minuti
                        //
                        // comandi con argomento numerico
                        //
                        if (argc <= 3) { // se non c'e', de che stamo a parla' ?
                            probe_cmd = SongsCmd::invalid;
                            break; // va ad err + help
                        }
                        // il terzo argomento dev'esserci 

                        args.cmdstr = std::string(argv[2]) + " " + std::string(argv[3]);
                        // concatenato con il suo arogmento serve solo per tracing

                        args.cmdcode = probe_cmd;

                        if (null_string(argv[3])) {
                            args.subarg="0";    // valore stringa nulla accettabile come 0
                        } else {
                            if (!parse_int(argv[3])) {
                                probe_cmd = SongsCmd::invalid; // non convertibile in naturale
                                break;
                            };
                            args.subarg = argv[3]; 
                        };
                        
                        if (argc >= 5) args.outfile = argv[4]; // se non c'e' resta nullstr da init
                        break;
                    
                    default:
                        //
                        // non tocchiamo, vale sempre invalid perche' getHelp e`' tornato a main()
                        //
                        break;
                };

            } else { // sono due ed il seocndo non e' un comando

                //
                // essere sicuri che sia rimasto invalid
                //
                DBG("Ho un file in 1 ma non un comando in 2: " << argv[1] << " " << argv[2] << std::endl);

            }
          
        } // fine else  



    } else {
       // c'e' solo il nome dell' eseguibile, lo lasciamo cosi' a centrare err+help finale 
    }
    
    debug_argv_dump(args, "post split_cmd ");

    if (probe_cmd == SongsCmd::invalid) {
        // 
        // se cosi' e' rimasto, mostra l' help e saluta
        //
            showError(SongsErr::errNot4Args, " " + std::string(argv[0]) + " help\n\n");
            showHelp();
            return false;

    }

    return true;

}

