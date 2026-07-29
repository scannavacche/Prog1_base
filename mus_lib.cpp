#include "mus_lib.hpp"

const VecStr ErrMsgs = {
    "Operazione completata. ",
    "Non trovato input file: ",
    "Impossibile creare output file: ",
    "Syntax: musica inputfile command outputfile"
};

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
    int TotArgs = argc; // tanto per non restare aggrappati ad un parametro 
    switch (TotArgs) {

        case 4:
        case 3:
            args.infile = argv[1];
            songs_split_cmd(argv[2], args.cmdstr, args.subarg);
            args.outfile = (TotArgs == 4) ? argv[3] : "" ;
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
void songs_normalize_cmd(std::string inparm) {
    for (char& c : inparm)
        c = std::toupper(
            static_cast<unsigned char>(c) 
            // il cast esterno serve a tornare da int a char, 
            // toupper non accetta char con valore "negativo", quindi unsigned e' una guardia
        );
}    

bool songs_parse_cmd(const std::string& text, SongsCmd& cmd)
{
    std::string key = text;   // lo duplico, non si sa mai volessi controllare la normalize
    songs_normalize_cmd(key);

    for (const CmdAlias& item : CmdTable) {
        if (key == item.name) {
            cmd = item.cmd;
            return true;
        }
    }

    cmd = SongsCmd::invalid;
    return false;
}
