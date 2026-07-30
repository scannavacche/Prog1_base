#include "mus_lib.hpp"

const VecStr ErrMsgs = {
/*
*   aggiornato sino a 

enum SongsErr {
    OK,
    errNotFoundIn,
    errNotOKOut,
    errNot4Args,
    errEmptyColl,
    errInvalidOp,
    errMinMax,
    errUnderdate,

    errCount // contatore di items disponibili in enum
};

*/

    "Operazione completata. ",
    "Non trovato input file: ",
    "Impossibile creare output file: ",
    "Sintassi: musica inputfile comando[:valore] outputfile \n\
      commands (case insensitive):  \n\
        List | L                                        lista il contenuto in input 'As is' \n\
        FilterLen:x | FilterL:x | FL:x                  match esatto con x Minuti   \n\
        FilterTit:x | FilterT:x | FT:x                  match parziale con x ovunque in Titolo \n\
        FilterInt:x | FilterI:x | FI:x                  match parziale con x ovunque in Interprete \n\
        FilterAnno:x | FilterAnP:x | FilterA:a | FA:x   match esatto con x Anno \n\
        OrdAnno | OrdAnP | OA                           ordina per Anno (bucket sort discreto) \n\
        OrdLen | OL                                     ordina per lunghezza totale (std::stable_sort)",
    "Lista delle canzoni in ingresso tristemente vuota ",
    "Operazione non riconosciuta: ",
    "Inversione inattesa di Min e Max ",
    "Panic at the disco! La ricerca dell'anno minimo ha fallito "

};


std::string zero_fill(int number, int length)
{
    std::ostringstream output;

    output << std::internal
           << std::setfill('0')
           << std::setw(length)
           << number;

    return output.str();
}


long song_runtime_total (const song &s){
    return (s.runtime_min * 60 + s.runtime_sec);
}

void file_close(std::ifstream& file)
{
    if (file.is_open())
        file.close();
}

void file_close(std::ofstream& file)
{
    if (file.is_open())
        file.close();
}

std::ostream& file_wopen(const std::string& filename, outFile& fileOut)
{
    //
    // Se filename è valido apre il file e rende lo stream relativo.
    // In mancanza del nome, o se il nome non e' valido, usa stdout.
    //

    if (filename.empty())
        return std::cout;  // caso filename non fornito coma arg da linea di comando

    fileOut.open(filename, std::ios::out | std::ios::trunc);

    if (!fileOut) {  // tento l'apertura con un nome di file non consentito
        showError(SongsErr::errNotOKOut, filename);
        return std::cout;  // e rende ancora stdout
    }

    return fileOut;  // se tutto e' andato bene, faccio solo da pipe sul file previsto
}
inFile file_ropen(std::string song_filename_in){
    inFile file(song_filename_in, std::ios::in);
    if (!file)
    { 
        showError(SongsErr::errNotFoundIn, song_filename_in);
        exit(1);
    } else {
        return file;
    }
}

void showError(SongsErr err, const std::string& detail)
{
    if (err >= SongsErr::OK && err < SongsErr::errCount)
        std::cerr << std::endl << ErrMsgs[err] << detail << '\n';
}

bool songs_parse_args(int argc, char *argv[], SongsArgs& args)
{
    // std::cout << "Argomenti: " << argc << " : " << argv[1]<< " " << argv[2] << " " << argv[3] << std::endl;

    switch (argc) {

        case 4:
        case 3:
            args.infile = argv[1];
            // argv_dump(args, " pre split_cmd ");
            songs_split_cmd(argv[2], args.cmdstr, args.subarg); // split su op[:arg]
            args.outfile = (argc == 4) ? argv[3] : "" ;
            // argv_dump(args, "post split_cmd ");
            // std::cout << "Siamo passati con " << argc << std::endl;
            return true;


        default:
            showError(SongsErr::errNot4Args, "");
            return false;
        

    };

}

void songs_split_cmd(std::string inparm, std::string &outcmd, std::string &outval){
    
    std::size_t pos = inparm.find(':');
    // std::cout << "Letto: " << inparm << " "; // il ritorno a capo lo mettono dopo
    if (pos == std::string::npos) {
        // comando senza sub arg
        // std::cout << "non ha argomento e lo tengo com'era, intero\n"; 
        outcmd = inparm;
        outval = "";

    } else {
        outcmd = inparm.substr(0, pos);
        outval  = inparm.substr(pos + 1);
        // std::cout << "Split in " << outcmd << " e " << outval << std::endl;
    }
}

// 
// scorre la string di comando e volge ogni char ad uppercase
// lo fa in place su inparm (byref), quindi non c'e' bisogno di accumulare in una str di ritorno
// 
std::string text_normalize(std::string &inparm) {
    std::string text = inparm;
    for (char& c : text)
        c = std::toupper(
            static_cast<unsigned char>(c) 
            // il cast esterno serve a tornare da int a char, 
            // toupper non accetta char con valore "negativo", quindi unsigned e' una guardia
        );
    return text;
}    

bool songs_parse_cmd(std::string &text, SongsCmd& cmd)
{
    // std::cout << "Cerco " << text << std::endl << std::endl;

    text = text_normalize(text); // ad un certo punto da void e' diventata string :)

    // std::cout << "Upper " << text << std::endl << std::endl;

    for (const CmdAlias& item : CmdTable) {
        if (text == item.name) {
            cmd = item.cmd;
            // std::cout << "Trovato " << cmd << std::endl;

            return true;
        }
    }

    cmd = SongsCmd::invalid;
    return false;
}

bool songs_read_line (inFile &file, std::string &line) {
    return  static_cast<bool>(std::getline(file, line));    // qui line contiene un record CSV completo
}

int songs_read_int_field(std::istringstream& record)
{
    std::string field;
    int value;

    // Legge e consuma tutto il campo, compreso il separatore ';'
    if (!std::getline(record, field, ';'))
        return 9999;

    std::istringstream conv(field);

    // Salta gli spazi iniziali e legge il prefisso numerico (se non sbaglia conversione)
    if (!(conv >> value))
        return 9998;

    return value;
}

bool songs_read_fields(std::string line, song &current){
    std::istringstream record(line);

    std::string field;  // valore temporaneo del campo letto, da validare

    // leggo Titolo

    if (!std::getline(record, field, ';')) {
        // campo non leggibile , lo scarto
        return false;
    } else {
        std::istringstream conv(field);
        current.titolo = field.empty() ? "[TITOLO MANCANTE]" : field; // e' una stringa, al massimo e' nulla
    };

    // Leggo Interprete

    if (!std::getline(record, field, ';')) {
        // campo non leggibile, lo scarto perche' e' un record tronco
        return false;
    } else {
        std::istringstream conv(field);
        current.interprete = field.empty() ? "[INTERPRETE MANCANTE]" : field; // e' una stringa, al massimo e' nulla
    };

    // con i campi numeri cambiamo strategie e rendo il valore intero filtrando da qui se era illeggibile
    // Leggo Anno

    current.anno = songs_read_int_field(record); // rende 9999 se non valido o illeggibile
    if (current.anno == 9999) return false;

    // Leggo Minuti (se non sono validi i minuti, non provo neanche con i secondi e lascio runtime = 0)

    current.runtime_min = songs_read_int_field(record);
    if (current.runtime_min == 9999) return false;

    current.runtime_sec = songs_read_int_field(record);
    if (current.runtime_sec == 9999) return false;

    if (current.runtime_min > 9000) {  // condizione generica di errore (9998 e sviluppi futuri)
        current.runtime_min = 0;
        current.runtime_sec = 0; // se una parte di un timestamp e' fallata, lo e' tutto
    };

        // Bohemian Rhapsody;Queen;1977;5;55

    return true; 
}

void song_dump(song s){
    // 
    // dump di debug della singola canzone
    //
    std::cout << s.titolo << " | " << s.interprete << " | " << s.anno << " | " << zero_fill(s.runtime_min,2) << ":" << zero_fill(s.runtime_sec,2) << std::endl;
}

void argv_dump(SongsArgs args, std::string msg) {
    // return; // spegne ed accende il debug :) 
    std::cout   << msg + "Argv " 
                << args.infile << " " 
                << args.cmdstr << " [:" 
                << args.subarg << "] " 
                << args.outfile 
            << std::endl;
}

bool text_match(std::string fullstr, std::string substr) {
    return (text_normalize(fullstr).find(text_normalize(substr)) != std::string::npos);
}

void coll_find_limits (const VecS &col, int& amin, int& amax) {
    if (col.empty()) {
        amin = 0;
        amax = 0;
    } else {
        //  c'e' almeno un elemento, procediamo
        amin = col[0].anno; amax = amin;
        for (const song &s : col) {
            if (s.anno < amin) amin = s.anno;
            if (s.anno > amax) amax = s.anno; 
        }
    }
}

void  song_write(std::ostream& outSongs, song currSong) {
    outSongs    << currSong.titolo << ";"
                << currSong.interprete << ";" 
                << currSong.anno << ";"
                << currSong.runtime_min << ";"
                << currSong.runtime_sec << std::endl;
}

/* esempio di dispatcher */

void coll_exec_cmd(const VecS &inColl, SongsArgs &args, VecS &outColl) {

    // argv_dump(args, "Nella exec ");

    outFile outFileHandle; // prepara uno stream di out double face file/console

    std::ostream& outSongs = file_wopen(args.outfile, outFileHandle); // comunque vada sara' su uno stream

    if (songs_parse_cmd(args.cmdstr, args.cmdcode)){  // e' qui che carichiamo il cod dell' ope richiesta

        // std::cout << "Analizziamo " << args.cmdstr << " e troviamo " << args.cmdcode << " e " << args.subarg << std::endl;

        switch (args.cmdcode) {

            case SongsCmd::listTest:
                // std::cout << "Ci sono " << inColl.size() << " Canzoni\n" << std::endl ;
                for (song currSong : inColl) {
                    outColl.push_back(currSong);
                    song_dump(currSong);
                }
                break;

            case SongsCmd::filterLen:
                for (song currSong : inColl) {
                    if (currSong.runtime_min == std::stoi(args.subarg)) {
                        // argv_dump(args, " Anno " + std::to_string(currSong.anno) + " uguale in "); 
                        // outColl.push_back(currSong);
                        // song_dump(currSong);
                        song_write(outSongs, currSong); // meccanismo rappezzato con i/o su ostream
                    }
                }

                break;

            case SongsCmd::filterTit: // riversa solo le canzoni che matchano una stringa nel titolo (sub arg)
                for (song currSong : inColl) {
                    if (text_match(currSong.titolo, args.subarg))   {
                        // argv_dump(args, " Anno " + std::to_string(currSong.anno) + " uguale in "); 
                        // outColl.push_back(currSong);
                        // song_dump(currSong);
                        song_write(outSongs, currSong); // meccanismo rappezzato con  i/o su ostream

                    }
                }
                break;

            case SongsCmd::filterInt: // riversa solo le canzoni che matchano una stringa nell' interprete (sub arg)
                for (song currSong : inColl) {
                    if (text_match(currSong.interprete, args.subarg))  {
                        // argv_dump(args, " Anno " + std::to_string(currSong.anno) + " uguale in "); 
                        // outColl.push_back(currSong);
                        // song_dump(currSong);
                        song_write(outSongs, currSong); // meccanismo rappezzato con  i/o su ostream
                    }
                }
                break;

            case SongsCmd::filterAnP: // riversa solo le canzoni con un certo Anno di Pubblicazione (sub arg)
                for (const song &currSong : inColl) {
                    if (currSong.anno == std::stoi(args.subarg)) {
                        // argv_dump(args, " Anno " + std::to_string(currSong.anno) + " uguale in "); 
                        // outColl.push_back(currSong);
                        // song_dump(currSong);
                        song_write(outSongs, currSong); // meccanismo rappezzato con  i/o su ostream
                    }
                }
                break;

            case SongsCmd::sortbyAnP: 
                //
                // ordina la lista per anno di pubb e la riversa integrale
                // per questa operazione uso il bucket sort suggerito (siamo su naturali)
                // e lo includiamo in un blocco per circoscrivere lo scope delle dichiarazioni locali
                //
                {
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
                            for (const song &s : aIndex[i]) {
                                outColl.push_back(s); // la salvo che non si sa mai nel main() ....
                                song_write(outSongs, s); 
                            }
                        }

                    } else {
                        showError(SongsErr::errMinMax, "Anni da " + std::to_string(amin)+ " a " + std::to_string(amax));
                        exit(1);
                    }
                    break;
                    
                }

            case SongsCmd::sortbyLen: 
            
                // ordina la lista per durata crescente e la riversa integrale
                // consideriamo l' indice pseudocontinuo, quindi niente bucket sort
                // possiamo provare qualche modalita' di std::sort o una bubblesort mia
                //
                // esplorazione per espolorazione, mi fido ad occhi chiusi e ci metto una lambda 
                //
                // prima la duplico per confrontabilita' e poi ordino la copia
                //
                outColl = inColl;

                std::stable_sort(
                    outColl.begin(),
                    outColl.end(),
                    [](const song& a, const song& b) {
                        return song_runtime_total(a) < song_runtime_total(b);
                    }
                );
                // infine la visita
                for (const song &s : outColl) song_write(outSongs, s);
                break;

            case SongsCmd::invalid:  
                // solo per togliersi dai piedi un warning senza spegnarlo per ogni switch
                // in realta' invalid produce gia' una lista a console con l'avviso di comando non valido
                break;
        }    
        file_close(outFileHandle);      //  e file handling su ofstream

    } else {
        // std::cout << "Fallisce il parsing dei cmd\n";
        for (song currSong : inColl) {
            song_dump(currSong);
        }
        showError(errInvalidOp, args.cmdstr);
        file_close(outFileHandle);      //  e file handling su ofstream
        exit(1);

    }
}


