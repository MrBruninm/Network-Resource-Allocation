#include "functions.h" // self-authorship

/*
Compilar:
g++ -O -fPIC -fexceptions -DNDEBUG -DIL_STD -I/opt/ibm/ILOG/CPLEX_Studio126/cplex/include -I/opt/ibm/ILOG/CPLEX_Studio126/concert/include *.cpp -o solver_model -L/opt/ibm/ILOG/CPLEX_Studio126/cplex/lib/x86-64_linux/static_pic -lilocplex -lcplex -L/opt/ibm/ILOG/CPLEX_Studio126/concert/lib/x86-64_linux/static_pic -lconcert -lcplex -lm -lpthread -g -w

Executar:
./solver_model
*/

using namespace std;

int main (int argc, char *argv[]) {

    string devices_str = "100"; // tamanho do arquivo de dispositivos.
    string tech_str    = "6";   // tipo de tecnologia móvel. 
    string servers_str = "100"; // tamanho do arquivo de servidores.
    string solver_str  = "1";   // modelo de solucionador.

    if (argc > 1) {
        devices_str = argv[1];
        if (argc > 2) {
            tech_str = argv[2];
            if (argc > 3) {
                servers_str = argv[3];
            }
        }
    }

    // para não ter erros...
    int solver = stoi (solver_str);
    if (!(1 <= solver and solver <= 3)) {
        cout << "ERROR: Error: solver out of range." << endl;
        return 0;
    }    
    int tech = stoi (tech_str);
    if (!(1 <= tech and tech <= 6)) {
        cout << "ERROR: unknown technology or out of scope..." << endl;
        return 0;
    } 
    cout << fixed; // fixa em 1e-6

    try {

        // Lendo o arquivo de dispostivo e o colocando em um vetor.
        vector <Device> devices;
        string path_devices = "read_and_write_archive/devices/" + devices_str + "_devices.txt";
        reading_files_devices (path_devices, devices);

        // Lendo os arquivos de servidores e os colocando em vetores.
        vector <Server> cc, ec, servers;
        string path_servers = "read_and_write_archive/servers/";
        reading_files_servers (path_servers + "CC_5_servers.txt", cc, 'C');
        reading_files_servers (path_servers + "EC_" + servers_str + "_servers.txt", ec, 'E');
        // Contatenando os servidores ec e cc.
        servers.insert (servers.end (), ec.begin (), ec.end ());
        servers.insert (servers.end (), cc.begin (), cc.end ());

        // Declarando o objeto métricas, onde ficará todas as informações.
        Metrics m1 = Metrics (devices.size (), servers.size (), tech, solver);
        
        // Função que verifica se o servidor i cobre o dispositivo d, baseado em sua tecnologia, 1 - 6G.
        coverage_and_tr (devices, servers, m1);

        models (devices, servers, m1);

        Metrics m2 = Metrics (m1, 0.0);
        models (devices, servers, m2);

        vector <double> factors (1, 1.1);
        while (factors.back () <= 1.3) {factors.push_back (factors.back () + 0.1);}
        model_3 (devices, servers, m2, factors);         

    } catch (int e) {cout << "ERROR: " << e << endl;}
    return 0;
}