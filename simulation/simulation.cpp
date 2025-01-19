#include <iostream>
#include <cstdlib>
#include <string>

using namespace std;

/*
Compilar:
g++ starter.cpp -o starter
Executar:
./starter
*/ 

int main (int argc, char *argv[]) {
    string devices_str = "900"; // tamanho do arquivo de dispositivos.
    if (argc > 1) {devices_str = argv[1];}
    int devices = stoi (devices_str);
    
    string path = "cd network_resource_allocation && "; 

    string compileCommand = path + "g++ -O -fPIC -fexceptions -DNDEBUG -DIL_STD -I/opt/ibm/ILOG/CPLEX_Studio126/cplex/include -I/opt/ibm/ILOG/CPLEX_Studio126/concert/include *.cpp -o solver_model -L/opt/ibm/ILOG/CPLEX_Studio126/cplex/lib/x86-64_linux/static_pic -lilocplex -lcplex -L/opt/ibm/ILOG/CPLEX_Studio126/concert/lib/x86-64_linux/static_pic -lconcert -lcplex -lm -lpthread -g -w";
    int compileResult = system (compileCommand.c_str ());
    
    while (!compileResult and devices <= 900) {int tech = 2;
        while (tech <= 6) {string executeCommand = path + "./solver_model " + to_string (devices) + " " + to_string (tech); system (executeCommand.c_str ()); tech++;}
        devices += 200;}
    return 0;
}