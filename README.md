# Prog1_base
Mini programma bonus, minimale, aderente alla proposta 

Versione 2026-07-31

* Sintassi al lancio con elenco funzioni ammesse

**   Sintassi: musica fileinput.csv <operazione> <valore> [fileoutput.csv]
***      <operazione> (case insensitive) di base
-        durata <m>                                      estrae canzoni con durata sino a <m> minuti
-        cerca <stringa>                                 estrae canzoni con <stringa> in titolo o interprete
-        anno <anno>                                     estrae canzoni con <anno> indicato
-        ordina                                          ordina per Anno
***      <operazione> (case insensitive) in sviluppo
-        cercai <stringa>                                estrae canzoni con <stringa> in interprete
-        cercat <stringa>                                estrae canzoni con <stringa> in titolo
-        List | L                                        lista il contenuto in input 'As is'
-        OrdLen | OL                                     ordina per lunghezza totale (std::stable_sort)


* Test automatici di test_musica.sh

** Esito ultimo:

PASS: 135
FAIL: 0
ACCETTATI per POLICY: 11
OSSERVAZIONI: 0

** 11 Test fallibili ma forzati come accettabili per policy

*** Condizioni anomale accettate ignorando l'anomalia (passa comunque un comando valido e completo)

- expect_accepted "help con argomento eccedente" help extra
- expect_accepted "durata con troppi argomenti" "$INPUT" durata 4 "$OUTDIR/test_durata_4_extra.csv" extra
- expect_accepted "cerca con troppi argomenti" "$INPUT" cerca queen "$OUTDIR/test_cerca_extra.csv" extra
- expect_accepted "anno con troppi argomenti" "$INPUT" anno 1979 "$OUTDIR/test_anno_extra.csv" extra
- expect_accepted "troppi argomenti dopo ordina" "$INPUT" ordina "$OUTDIR/test_ordina_extra.csv" extra

*** Condizioni anomale accettate con valore del parametro nullo

- expect_accepted "cerca soli spazi quotati" "$INPUT" cerca "   "

*** Condizioni anomale accettate con valore del parametro nullo forzato a "0"

- expect_accepted "anno stringa vuota" "$INPUT" anno ""
- expect_accepted "durata operando vuoto" "$INPUT" durata ""
- expect_accepted "durata zero" "$INPUT" durata 0

*** Controlli non attivati, l'anno e' ancora un naturale libero

- expect_accepted "anno sotto range" "$INPUT" anno 1799
- expect_accepted "anno sopra range" "$INPUT" anno 3000


---

\newpage

Versione precedente, non ancora aderente

* Sintassi al lancio con elenco funzioni ammesse

** Sintassi: musica inputfile comando[:valore] outputfile
      commands (case insensitive):
-        List | L                                        lista il contenuto in input 'As is'
-        FilterLen:x | FilterL:x | FL:x                  match esatto con x Minuti
-        FilterTit:x | FilterT:x | FT:x                  match parziale con x ovunque in Titolo
-        FilterInt:x | FilterI:x | FI:x                  match parziale con x ovunque in Interprete
-        FilterAnno:x | FilterAnP:x | FilterA:a | FA:x   match esatto con x Anno
-        OrdAnno | OrdAnP | OA                           ordina per Anno (bucket sort discreto)
-        OrdLen | OL                                     ordina per lunghezza totale (std::stable_sort)

---

Indice minimale delle librerie:

Oggetto/funzione	                    Header
std::string	                            <string>
std::vector	                            <vector>
std::cout, std::cin, std::cerr	        <iostream>
std::ifstream, std::ofstream	        <fstream>
std::stringstream, std::istringstream	<sstream>
std::sort, std::find	                <algorithm>
std::setw, std::setprecision	        <iomanip>
std::getline	                        <string>
std::stoi, std::stod	                <string>
std::printf, std::fopen	                <cstdio>
std::tolower, std::isdigit	            <cctype>
std::abs	                            <cmath> 
                                oppure  <cstdlib> a seconda del tipo

filemodes

fopen	C++
"r"	    std::ios::in
"w"	    std::ios::out | std::ios::trunc
"a"	    std::ios::out | std::ios::app
"r+"	std::ios::in | std::ios::out
"w+"	std::ios::in | std::ios::out | std::ios::trunc
"a+"	std::ios::in | std::ios::out | std::ios::app

"r"   lettura, il file deve esistere
"w"   scrittura, crea o tronca il file esistente
"a"   append, scrive in fondo
"r+"  lettura + scrittura, deve esistere
"w+"  lettura + scrittura, crea/tronca
"a+"  lettura + append

