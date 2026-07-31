//
// Libreria di base, implementazione delle funzioni comuni  
// 

#include "mus_lib.hpp"
#include "mus_csr.hpp"

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
    errNullValue,


    errCount // contatore di items disponibili in enum
};

*/
    // dalla versione 2026-07-31 tutti i msg vengonno dirottati su std:cerr (attivi con -DDEBUG)
    // non piu' disponibili su std::cout per non turbare eventuali redirect 

    "Operazione completata. ",
    "Non trovato input file: ",
    "Impossibile creare output file: ",
    "Errore di sintassi - Per aiuto: ", 
    "Lista delle canzoni in ingresso tristemente vuota ",
    "Operazione non riconosciuta: ",
    "Inversione inattesa di Min e Max ",
    "Panic at the disco! La ricerca dell'anno minimo ha fallito ", 
    "Il comando richiesto deve avere argomento non nullo: "

};

const std::string helpMessage = 
    "Sintassi: musica fileinput.csv <operazione> <valore> [fileoutput.csv] \n\n\
      <operazione> (case insensitive) di base  \n\
        durata <m>                                      estrae canzoni con durata sino a <m> minuti \n\
        cerca <stringa>                                 estrae canzoni con <stringa> in titolo o interprete \n\
        anno <anno>                                     estrae canzoni con <anno> indicato \n\
        ordina                                          ordina per Anno \n\n\
      <operazione> (case insensitive) in sviluppo \n\
        List | L                                        lista il contenuto in input 'As is' \n\
        OrdLen | OL                                     ordina per lunghezza totale (std::stable_sort)\n" ;

bool null_string(std::string s) {
    return s.empty();
}

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

void showError(SongsErr err, const std::string &detail)
{
    if (err >= SongsErr::OK && err < SongsErr::errCount)
        std::cerr << std::endl << ErrMsgs[err] << detail << '\n';
}

void showHelp()
{
    std::cout << std::endl << helpMessage << '\n';
}


void songs_split_cmd(std::string inparm, std::string &outcmd, std::string &outval){
    //
    // non piu' necessaria, serviva a dividere comando:valore in cmd, subarg
    //
    std::size_t pos = inparm.find(':');
    DBG("Letto: " << inparm);
    if (pos == std::string::npos) {
        // comando senza sub arg
        DBG("non ha argomento e lo tengo com'era, intero"); 
        outcmd = inparm;
        outval = "";
    } else {
        outcmd = inparm.substr(0, pos);
        outval  = inparm.substr(pos + 1);
        DBG("Split in " << outcmd << " e " << outval);
    }
}

// 
// scorre la string di comando e volge ogni char ad uppercase
// lo fa in place su inparm (byref), quindi non c'e' bisogno di accumulare in una str di ritorno
// 
std::string text_normalize(const std::string &inparm) {
    std::string text = inparm;
    for (char& c : text)
        c = std::toupper(
            static_cast<unsigned char>(c) 
            // il cast esterno serve a tornare da int a char, 
            // toupper non accetta char con valore "negativo", quindi unsigned e' una guardia
        );
    return text;
}    

bool songs_parse_cmd(const std::string &pippo, SongsCmd& cmd)
{
    std::string text = pippo; // aveva ragione Zucchero

    DBG(
            "Cerco " 
        << text 
        << std::endl // un LF extra qui ci vuole 
    );

    text = text_normalize(text); // ad un certo punto dello sviluppo da void e' diventata string :)

    DBG("Upper " << text  << std::endl);   // anche qui uno extra ci vuole 

    for (const CmdAlias& item : CmdTable) {
        if (text == item.name) {
            cmd = item.cmd;
            DBG("Trovato " << cmd );
            return true;
        }
    }

    cmd = SongsCmd::invalid;  // se non trova una delle stringhe previste non sfonda niente
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

void debug_song_dump(song s){
    // 
    // dump di debug della singola canzone
    //
    DBG(
            s.titolo << " | " 
        <<  s.interprete << " | " 
        <<  s.anno << " | " 
        <<  zero_fill(s.runtime_min,2) << ":" 
        <<  zero_fill(s.runtime_sec,2) 
    );
}


void debug_argv_dump(SongsArgs args, std::string msg) {
    //
    // dump di debug degli argomenti da linea di comando gia' interpretati
    //
    DBG(msg + " Argv - in: " 
            << args.infile << " op: " 
            << args.cmdstr << " cod: " 
            << args.cmdcode << " arg: "
            << args.subarg << " out: " 
            << args.outfile 
        << std::endl
    );
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

// aiutino dall' AI


int parse_year(const std::string& text)
{
    //
    // valida un anno 1800-2999 e lo restituisce come int
    // 
    static const std::regex year_pattern(
        R"(^(?:1[89][0-9]{2}|2[0-9]{3})$)"
    );
    if (!std::regex_match(text, year_pattern)) return 0;
    return std::atoi(text.c_str());
}

int parse_minute(const std::string& text)
{
    //
    // valida una stringa tra 00 e 99 e la rende come valore intero per minuti
    //
    static const std::regex minutes_pattern(R"(^[0-9]+$)");
    if (!std::regex_match(text, minutes_pattern)) return 0;
    int minutes = std::atoi(text.c_str());
    if (minutes < 0 || minutes > 99 ) return 0; else return minutes;   
}



