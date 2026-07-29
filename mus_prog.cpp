#include "mus_lib.hpp"



int main(int argc, char *argv[])
{
    SongsArgs argstr;
    song currSong;
    VecS inCollection, outCollection;

    if (songs_parse_args(argc, argv, argstr)) {
        std::string line;
        inFile inSongs = song_ropen(argstr.infile);
        while (songs_read_line(inSongs, line)) {
            if (songs_read_fields(line, currSong)) {
                inCollection.push_back(currSong);
            }             
        }
        if (inCollection.empty()) {
            showError(SongsErr::errEmptyColl, "");
            exit(1);
        } else {
            //
            // std::cout << "e qui comincia il bello (il dispatcher dei comandi)" << std::endl;
            //
            coll_exec_cmd(inCollection, argstr, outCollection);
        };

        return 0;

    } else {
        showError(SongsErr::errFailToParse, "Fine");
        return 1;
    }
    
}