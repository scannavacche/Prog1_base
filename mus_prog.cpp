#include "mus_lib.hpp"

/* esempio di dispatcher 

SongsCmd songs_exec_cmd(SongsArgs &args) {
    SongsCmd cmd;
    if (songs_parse_cmd(args.cmdstr, cmd)){
        switch (cmd) {

            case SongsCmd::listTest:
                ...
                break;

            case SongsCmd::filterLen:
                ...
                break;

            case SongsCmd::filterTit:
                ...
                break;

            case SongsCmd::INVALID:
                break;
        }    

    }
}

*/


int main(int argc, char *argv[])
{
    SongsArgs argstr;

    if (songs_parse_args(argc, argv, argstr)) {

        inFile inSongs = song_ropen(argstr.infile);
        
        return 0;

    } else {
        return 1;
    }
    
}