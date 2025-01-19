#include <iostream>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <set>

using namespace std;
namespace fs = std::filesystem;

#define P 21

typedef struct {

    int main;  
    int aux;  
    double d_i_Distance; // distance from the device to the server.
    double t_i_d; // communication time from i to d.
    double t_i_p; // processing time .
    double tr; // response time = tid + tip.
}Dserver; // main aux diD iiD

class Device {

    public:
        int    position;            // 00
        double latitude;            // 01
        double longitude;           // 02
        double pcc;                 // 03
        int    pcn;                 // 04
        double mem;                 // 05
        double st;                  // 06
        double bw;                  // 07
        double sd;                  // 08
        double cost;                // 09
        int    service;             // 10
        int    task;                // 11
        Dserver server;             // 12
        vector <Dserver> servers;   // 13            

        Device (int    position_temp,
                double latitude_temp, 
                double longitude_temp,
                double pcc_temp,
                int    pcn_temp,
                double memory_temp,
                double storage_temp,
                double bandwidth_temp,
                double sd_temp,
                double cost_temp,
                int    service_temp,
                int    task_temp) {
            
            position = position_temp;
            latitude = latitude_temp;
            longitude = longitude_temp;
            pcc = pcc_temp;
            pcn = pcn_temp;
            mem = memory_temp;
            st = storage_temp;
            bw = bandwidth_temp;
            sd = sd_temp;
            cost = cost_temp;
            service = service_temp;
            task = task_temp;
            server = {-1, -1, 0.0, 0.0, 0.0, 0.0};
            servers = vector <Dserver> ();
        }

        void show () {
            cout << endl;
            cout << "Position:.......... " << position              << endl; // 00
            cout << "Latitude:.......... " << latitude              << endl; // 01
            cout << "Longitude:......... " << longitude             << endl; // 02
            cout << "PCC:............... " << pcc       << " GHz"   << endl; // 03
            cout << "PCN:............... " << pcn       << " core"  << endl; // 04
            cout << "Memory:............ " << mem       << " GB"    << endl; // 05
            cout << "Storage:........... " << st        << " GB"    << endl; // 06
            cout << "Bandwidth:......... " << bw        << " Mbps"  << endl; // 07
            cout << "Sent data:......... " << sd        << " Mb"    << endl; // 08
            cout << "Cost:.............. " << cost      << " euros" << endl; // 09
            cout << "Service:........... " << service               << endl; // 10
            cout << "Task:.............. " << task                  << endl; // 11
            cout << "Server:............ " << server.main           << endl; // 12
            cout << "Time Response:..... " << server.tr << " ms"    << endl; // 13
            cout << "Servers Covered:... ";                                  // 14
            for (Dserver i : servers) {cout << i.main << " ";}                       
            cout << endl << endl;
        }
};

class Server {
    
    public:

        int    position;
        double latitude;
        double longitude;
        double pcc;
        int    pcn;
        double mem;
        double st;
        double bw;
        double cost;
        double tp;
        char type;

        Server (int position_temp,
                double latitude_temp,
                double longitude_temp,
                double pcc_temp,
                int    pcn_temp,
                double memory_temp,
                double storage_temp,
                double bandwidth_temp,
                double cost_temp,
                double tp_temp,
                char type_temp) {
            
            position  = position_temp;
            latitude  = latitude_temp;
            longitude = longitude_temp;
            pcc       = pcc_temp;
            pcn       = pcn_temp;
            mem       = memory_temp;
            st        = storage_temp;
            bw        = bandwidth_temp;
            cost      = cost_temp;
            tp        = tp_temp;
            type      = type_temp;
        }

        void show () {
            cout << endl;
            cout << "Position:.......... " << position         << endl;
            cout << "Latitude:.......... " << latitude         << endl;
            cout << "Longitude:......... " << longitude        << endl;
            cout << "Server Type:....... " << type << "C"      << endl;
            cout << "PCC:............... " << pcc  << " GHz"   << endl;
            cout << "PCN:............... " << pcn  << " core"  << endl;
            cout << "Memory:............ " << mem  << " GB"    << endl;
            cout << "Storage:........... " << st   << " GB"    << endl;
            cout << "Bandwidth:......... " << bw   << " Mbps"  << endl;
            cout << "Cost:.............. " << cost << " euros" << endl;
            cout << "Time Processing:... " << tp   << " ms"    << endl;
            cout << endl;
        }
};

class Metrics {
    
    public:

        int    devices;                // 01
        int    servers;                // 02
        int    technology;             // 03
        double alpha;                  // 04
        int    solver;                 // 05 
        double solver_time;            // 06
        string solver_status;          // 07
        double objective_function;     // 08
        double devices_covered;        // 09    
        double devices_served;         // 10
        double devices_covered_served; // 11
        double devices_served_ec;      // 12
        double devices_served_cc;      // 13
        double servers_used;           // 14
        double servers_used_ec;        // 15
        double servers_used_cc;        // 16
        double TR_max;                 // 17
        double total_costs;            // 18
        double server_costs;           // 19
        double cd_nCovered;            // 20
        double cd_nServerd;            // 21

        Metrics (int devices_temp, int servers_temp, int tech_temp, int solver_temp) {
            
            devices                = devices_temp; // 01
            servers                = servers_temp; // 02
            technology             = tech_temp;    // 03
            alpha                  = 1.0;          // 04
            solver                 = solver_temp;  // 05
            solver_time            = 0.0;          // 06
            solver_status          = "";           // 07
            objective_function     = 0.0;          // 08
            devices_covered        = 0.0;          // 09
            devices_served         = 0.0;          // 10
            devices_covered_served = 0.0;          // 11
            devices_served_ec      = 0.0;          // 12
            devices_served_cc      = 0.0;          // 13
            servers_used           = 0.0;          // 14
            servers_used_ec        = 0.0;          // 15
            servers_used_cc        = 0.0;          // 16
            TR_max                 = 0.0;          // 17
            total_costs            = 0.0;          // 18
            server_costs           = 0.0;          // 19
            cd_nCovered            = 0.0;          // 20
            cd_nServerd            = 0.0;          // 21
        }

        Metrics (Metrics &m, double alpha_temp) {

            devices                = m.devices;            // 01
            servers                = m.servers;            // 02
            technology             = m.technology;         // 03
            if (alpha_temp) {                              // 04 and 06
                alpha              = alpha_temp;                
                solver_time        = m.solver_time;                  
            } else {                                       // 04 and 06
                alpha              = 1.0;     
                solver_time        = 0.0;  
            }                                    
            solver                 = m.solver + 1;         // 05
            solver_status          = "";                   // 07
            objective_function     = 0.0;                  // 08
            devices_covered        = m.devices_covered;    // 09
            devices_served         = 0.0;                  // 10
            devices_covered_served = 0.0;                  // 11
            devices_served_ec      = 0.0;                  // 12
            devices_served_cc      = 0.0;                  // 13
            servers_used           = 0.0;                  // 14
            servers_used_ec        = 0.0;                  // 15
            servers_used_cc        = 0.0;                  // 16
            TR_max                 = 0.0;                  // 17
            total_costs            = m.objective_function; // 18
            server_costs           = 0.0;                  // 19
            cd_nCovered            = m.cd_nCovered;        // 20
            cd_nServerd            = 0.0;                  // 21
        }

        void m_to_string (vector <string> &str) {
            int i = 0;

            str[i] = to_string (devices);                                              ++i; // 01
            str[i] = to_string (servers);                                              ++i; // 02 
            str[i] = to_string (technology);                                           ++i; // 03
            str[i] = to_string (alpha * 100.0);                                        ++i; // 04
            str[i] = to_string (solver);                                               ++i; // 05
            str[i] = to_string (solver_time);                                          ++i; // 06
            str[i] = solver_status;                                                    ++i; // 07
            str[i] = to_string (objective_function);                                   ++i; // 08
            str[i] = to_string (devices_covered / devices * 100.0);                    ++i; // 09  
            str[i] = to_string (devices_served / devices  * 100.0);                    ++i; // 10

            if (devices_served) {    
                str[i] = to_string (devices_covered_served / devices_covered * 100.0); ++i; // 11
                str[i] = to_string (devices_served_ec / devices_served       * 100.0); ++i; // 12
                str[i] = to_string (devices_served_cc / devices_served       * 100.0); ++i; // 13
                str[i] = to_string (servers_used / servers                   * 100.0); ++i; // 14
                str[i] = to_string (servers_used_ec / servers_used           * 100.0); ++i; // 15
                str[i] = to_string (servers_used_cc / servers_used           * 100.0); ++i; // 16
            }

            i = 16;

            str[i] = to_string (TR_max);                                               ++i; // 17
            str[i] = to_string (total_costs);                                          ++i; // 18
            str[i] = to_string (server_costs);                                         ++i; // 19
            str[i] = to_string (cd_nCovered);                                          ++i; // 20
            str[i] = to_string (cd_nServerd);                                          ++i; // 21
        }

        void show () {

            vector <string> str (P, "0.0"); m_to_string (str);
            int i = 0; 

            cout << endl;
            cout << "******************** Metrics: ********************" << endl;
            cout << "Devices:.................. " << str[i]              << endl; ++i; // 01
            cout << "Servers:.................. " << str[i]              << endl; ++i; // 02
            cout << "Technology:............... " << str[i] << " G"      << endl; ++i; // 03
            cout << "Alpha:.................... " << str[i] << " %"      << endl; ++i; // 04          
            cout << "Solver Model:............. " << str[i]              << endl; ++i; // 05
            cout << "Solver Time:.............. " << str[i] << " s"      << endl; ++i; // 06
            cout << "Solver Status:............ " << str[i]              << endl; ++i; // 07
            cout << "Objective Function:....... " << str[i]              << endl; ++i; // 08
            cout << "Devices Covered:.......... " << str[i] << " %"      << endl; ++i; // 09
            cout << "Devices Served:........... " << str[i] << " %"      << endl; ++i; // 10
            cout << "Devices Covered Served:... " << str[i] << " %"      << endl; ++i; // 11
            cout << "Devices Served in EC:..... " << str[i] << " %"      << endl; ++i; // 12
            cout << "Devices Served in CC:..... " << str[i] << " %"      << endl; ++i; // 13
            cout << "Servers Used:............. " << str[i] << " %"      << endl; ++i; // 14
            cout << "Servers EC Used:.......... " << str[i] << " %"      << endl; ++i; // 15
            cout << "Servers CC Used:.......... " << str[i] << " %"      << endl; ++i; // 16                    
            cout << "TR_max (makespan):........ " << str[i] << " ms"     << endl; ++i; // 17
            cout << "Total Costs:.............. " << str[i] << " euros"  << endl; ++i; // 18
            cout << "Server Costs:............. " << str[i] << " euros"  << endl; ++i; // 19
            cout << "cd not Covered:........... " << str[i] << " euros"  << endl; ++i; // 20
            cout << "cd not Served:............ " << str[i] << " euros"  << endl;      // 21
            cout << endl;
        }

        string toCSV () {
            vector <string> str (P, "0"); m_to_string (str);
            string csv = str[0];
            for (int i = 1; i < str.size (); i++) {csv += "," + str[i];}
            return csv;
        }           
        
        void save () {
            for (int x = 1; x <= 3; x++) {save (x);}
        }

        void save (int x) {
            string path, directory_0 = "Results/", directory_1 = directory_0 + "Devices/", directory_2 = directory_0 + "Solvers/", directory_3 = directory_0 + "Technologies/";

            if (!fs::exists (directory_0)) {fs::create_directories (directory_0);}
            if (!fs::exists (directory_1)) {fs::create_directories (directory_1);}
            if (!fs::exists (directory_2)) {fs::create_directories (directory_2);}
            if (!fs::exists (directory_3)) {fs::create_directories (directory_3);}

            if        (x == 1) {path = directory_1 + to_string (devices) + ".txt";
            } else if (x == 2) {path = directory_2 + to_string (solver) + ".txt";
            } else if (x == 3) {path = directory_3 + to_string (technology) + "G.txt";}

            ofstream file (path, ios::app);
            file.seekp (0,ios::end); if (!file.tellp ()) {file << "Devices,Servers,Tech,Alpha,Solver,Solver_Time,Solver_Status,Objective_Function,Devices_Covered,Devices_Served,Devices_Covered_Served,Devices_Served_EC,Devices_Served_CC,Servers_Used,Servers_Used_EC,Servers_Used_CC,TR_med,Total_Costs,Server_Costs,Devices_not_Covered_Costs,Devices_not_Served_Costs\n";}
            file << toCSV () << "\n"; file.close ();                
        }
};