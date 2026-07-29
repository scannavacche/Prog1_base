#include "mus_lib.hpp"

const VecStr ErrMsgs = {
    "Operazione completata. ",
    "Non trovato input file: ",
    "Impossibile creare output file: ",
    "Syntax: musica inputfile command outputfile"
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


long song_runtime_total (song s){
    return (s.runtime_min * 60 + s.runtime_sec);
}

void song_close(std::ifstream& file) {
    file.close();
}

outFile song_fopen(std::string song_filename_out){
    //
    // apre il file in scrittura e rende l'handle al file
    //
    outFile file(song_filename_out, std::ios::out | std::ios::trunc);
    if (!file)
    { 
        showError(SongsErr::errNotOKOut, song_filename_out);
        exit(1);
    } else {
        return file;
    }

    return file;
}

inFile song_ropen(std::string song_filename_in){
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
        std::cerr << ErrMsgs[err] << detail << '\n';
}

bool songs_parse_args(int argc, char *argv[], SongsArgs& args)
{
    // std::cout << "Argomenti: " << argc << " : " << argv[1]<< " " << argv[2] << " " << argv[3] << std::endl;
    switch (argc) {

        case 4:
        case 3:
            args.infile = argv[1];
            songs_split_cmd(argv[2], args.cmdstr, args.subarg);
            args.outfile = (argc == 4) ? argv[3] : "" ;
            // std::cout << "Siamo passati con " << argc << std::endl;
            return true;


        default:
            showError(SongsErr::errNot4Args, "");
            return false;
        

    };

}

void songs_split_cmd(std::string inparm, std::string &outcmd, std::string &outval){
    
    std::size_t pos = inparm.find(':');

    if (pos == std::string::npos) {
        // comando senza sub arg
        outcmd = inparm;
        outval = "";
    } else {
        std::string outcmd = inparm.substr(0, pos);
        std::string outval  = inparm.substr(pos + 1);
    }
}

// 
// scorre la string di comando e volge ogni char ad uppercase
// lo fa in place su inparm (byref), quindi non c'e' bisogno di accumulare in una str di ritorno
// 
void songs_normalize_cmd(std::string &inparm) {
    for (char& c : inparm)
        c = std::toupper(
            static_cast<unsigned char>(c) 
            // il cast esterno serve a tornare da int a char, 
            // toupper non accetta char con valore "negativo", quindi unsigned e' una guardia
        );
}    

bool songs_parse_cmd(std::string& text, SongsCmd& cmd)
{
    songs_normalize_cmd(text);

    // std::cout << "Cerco " << text << std::endl;

    for (const CmdAlias& item : CmdTable) {
        if (text == item.name) {
            cmd = item.cmd;
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

/* esempio di dispatcher */

void coll_exec_cmd(const VecS inColl, SongsArgs &args, VecS &outColl) {
    SongsCmd cmd;
    if (songs_parse_cmd(args.cmdstr, cmd)){
        switch (cmd) {

            case SongsCmd::listTest:
                // std::cout << "Ci sono " << inColl.size() << " Canzoni\n" << std::endl ;
                for (song currSong : inColl) {
                    outColl.push_back(currSong);
                    song_dump(currSong);
                }
                break;

            case SongsCmd::filterLen:
                break;

            case SongsCmd::filterTit:
                break;

            case SongsCmd::invalid:
                break;
        }    

    } else {
        // std::cout << "Fallisce il parsing dei cmd\n";
    }
}


