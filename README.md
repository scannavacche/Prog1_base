# Prog1_base
Mini programma bonus, minimale, aderente alla proposta 

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

