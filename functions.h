#include <algorithm> 
#include <ilcplex/ilocplex.h>
#include <iomanip>
#include <stdlib.h>
#include "structs.h" // self-authorship.

using namespace std;

#define M 1000 // penalty.

const double PROPAGATION_SPEED = 299792.458; // propagation speed of light in km/s.
const double RADIUS_EARTH = 6371.0; // radius of the earth, in km, is the average of the equatorial radius and the polar radius.

/* Debug:
cout << endl << "0" << endl << endl;
*/

typedef IloArray <IloNumVarArray> NumVar2Matrix;

double haversine_distance (double olat1, double olon1, double olat2, double olon2) { // o = old.
    
    /*        
    The distance calculation that will be used is the Haversine formula.
    */

    long double lat1 = olat1 * (M_PI / 180.0);
    long double lon1 = olon1 * (M_PI / 180.0);

    long double lat2 = olat2 * (M_PI / 180.0);
    long double lon2 = olon2 * (M_PI / 180.0);

    long double dlat = lat2 - lat1;
    long double dlon = lon2 - lon1;

    long double internal_part = cos (lat1) * cos (lat2) * pow (sin (dlon / 2.0), 2.0) + pow (sin (dlat / 2.0), 2.0);
    long double external_part = 2.0 * asin (sqrt (internal_part));

    return external_part * RADIUS_EARTH;
}

double d_to_i_distance (Device d, Server i)   {return haversine_distance(d.latitude, d.longitude, i.latitude, i.longitude);}

vector <Device> coverage (vector <Device> &devices, vector <Server> &servers, Metrics &m) {
    // declaração padrão de variavel:
    double radius       = -1;
    double data_rate_d_e = -1;
    double data_rate_e_c = 100000.0;
    // diferentes tipos de tecnologias móveis:
    if      (m.technology == 1) { // 1G.+
        radius            = 20.0;
        data_rate_d_e      = 0.0024; // até 2.4 Kbps
    }
    else if (m.technology == 2) { // 2G.
        radius            = 10.0;
        data_rate_d_e      = 0.064; // até 64 Kbps
    }
    else if (m.technology == 3) { // 3G.
        radius            = 5.0;
        data_rate_d_e     = 2.0; // até 2 Mbps
    }
    else if (m.technology == 4) { // 4G.
        radius            = 3.0;
        data_rate_d_e     = 100.0; // até 100 Mbps
        //data_rate_e_c      = 0;
        //propagation_speed = 0;
    }
    else if (m.technology == 5) { // 5G.
        radius            = 0.6;
        data_rate_d_e     = 1000.0; // até 1 Gbps
    }
    else if (m.technology == 6) { // 6G.
        radius            = 0.32;
        data_rate_d_e      = 10000.0; // até 100 Gbps
    }
    // verificação se o dispositivo d é coberto pelo servidor i:
    vector <Device> devices_covered; // declaração do vetor dos dispositivos d que estão cobertos.
    set <int> checker_device; // declaração do vetor sem repetição para não duplicar os dispositivos cobertops.
    for (int d = 0; d < devices.size (); d++) { // percorre todos os dispositivos d.
        devices[d].bw = data_rate_d_e; // baseado na tecnologia insere o valor de bw ao dispositivo d.
        for (int i = 0; i < servers.size () - 5; i++) { // percorre todos os servidores i.
            servers[i].bw = data_rate_e_c; // baseado na tecnologia insere o valor de bw ao servidor i.
            double distance = d_to_i_distance (devices[d], servers[i]); // cálcula a distância do dispositivo d ao servidor i.
            if (distance <= radius) { // verifica se a distancia do dispositivo d ao servidor i é menor ou igual ao raio de cobertura da tecnologia escolhida.
                if (checker_device.find (d) == checker_device.end ()) { // verifica se o dispositivo d já foi coberto por pelo menos um servidor i, se não coberto entra.
                    checker_device.insert (d); // dispositivo d ainda não coberto é salvo no vetor sem repetição.
                    devices_covered.push_back (devices[d]); // adiciona o dispositivo d ao vetor dos dispositivos cobertos.
                }
                devices_covered.back ().servers.push_back ({i, -1, distance, 0.0, 0.0, 0.0}); // adiciona o servidor i que cobre o dispositivo d a lista de servidores que o cobrem.
            }    
        }    
        if (checker_device.find (d) == checker_device.end ()) {m.cd_nCovered += devices[d].cost;} // após o loop se o dispositivo d não for coberto, soma o custo de não atendimento dele.
    }

    m.devices_covered = devices_covered.size (); // após o loop guarda a quantidade total de dispositivos d cobertos.

    for (Device &d : devices_covered) { // percorre todos os dispositivos d cobertos.
        Dserver temp = {-1, -1, RADIUS_EARTH, 0.0, 0.0, 0.0}; // strutc temporária.
        for (Dserver &i : d.servers) { // percorre todos os servidores i que cobrem o dispositivo d.
            // Procura a menor distância para roteamento.
            if (i.d_i_Distance <= temp.d_i_Distance) { //verifica se a distancia do servidor i é a menor dentre os servidores que cobrem o dispositivo d.
                temp.d_i_Distance = i.d_i_Distance; // salva a menor distancia do servidor i que cobre o dispositivo d.
                temp.aux = i.main; // salva o servidor i que contem a menor distancia de cobertura do dispositivo d.
            }      
        }
        // completa os dispositivos cobertos com os roteamentos para CC:
        for (int i = servers.size () - 5; i < servers.size (); i++) { // percorre todos os servidores CC que são somente 5 .
            temp.main = i; // salva o servidor i de CC que não cobre o dispositivo d, mas pode ser usado com roteamento.
            d.servers.push_back (temp); // salva a struct no vetor que contem todos os servidores que cobrem o dispositivo d.
            servers[i].bw = data_rate_e_c;
        }
    }

    return devices_covered; // retorna o vetor com todos os dispositivos d que estão cobertos.
}

void time_response (vector <Device> &devices, vector <Server> &servers) {
    for (Device &d : devices) { // percorre todos os dispositivos d cobertos.
        for (Dserver &i : d.servers) { // percorre o vetor que contem todos os servidores i que cobrem o dispositivo.
            // Cálculo para o tempo, em ms, de processamento do servidor i.
            i.t_i_p = d.sd * servers[i.main].tp; // time processing.
            // Cálculo para tempo, em ms, de comunicação do dispositivo d até o servidor i
            i.t_i_d = (d.sd / d.bw) + (i.d_i_Distance / PROPAGATION_SPEED); // time communication.
            i.t_i_d *= 1000.0; // transform s into ms.
            if (i.main >= servers.size () - 5) { // verifica se o servidor i é CC.
                i.t_i_d += 111.86; // milan (CC) to ohio (CC) in ms, verificado 18/12/24.
            }
            i.tr = i.t_i_d + i.t_i_p; // soma o tempo de comunicação com o tempo de processamento gerando o tempo de resposta.
        }
    }
}

void coverage_and_tr (vector <Device> &devices, vector <Server> &servers, Metrics &m) {
    devices = coverage (devices, servers, m);
    time_response (devices, servers);
}

vector <string> line_split (string line, char delimiter) {
	vector <string> words_split;
	stringstream ss (line);
	string word;
	while (getline (ss, word, delimiter)) {words_split.push_back (word);}
    return words_split;
}

void reading_files_devices (string path, vector <Device> &devices) {
 
    ifstream file (path);
    string line;
    
    if (file.is_open ()) {

        getline (file, line); // elimina a primeira linha dor arquivo

        while (getline (file, line)) {

            vector <string> l = line_split (line, ' ');

            Device device = Device (stoi (l[0]),   // position
                                    stod (l[1]),   // latitude
                                    stod (l[2]),   // longitude
                                    stod (l[3]),   // pcc
                                    stoi (l[4]),   // pcn
                                    stod (l[5]),   // memory
                                    stod (l[6]),   // storage
                                    stod (l[7]),   // bandwidth
                                    stod (l[8]),   // sd
                                    stod (l[9]),   // cost
                                    stoi (l[10]),  // service
                                    stoi (l[11])); // task             
            devices.push_back (device);
        } file.close ();
    } else {cerr << "ERROR: The device file could not be opened." << endl;}
}

void reading_files_servers (string path, vector <Server> &servers, char type) {

    ifstream file (path);
    string line;
    
    if (file.is_open ()) {
    
        getline (file, line); // linha de informações descartada.

        while (getline (file, line)) {
            
            vector <string> l = line_split (line, ' ');

            Server server = Server (stoi (l[0]),   // position
                                    stod (l[1]),   // latitude
                                    stod (l[2]),   // longitude
                                    stod (l[3]),   // pcc
                                    stoi (l[4]),   // pcn
                                    stod (l[5]),   // memory
                                    stod (l[6]),   // sotrage
                                    stod (l[7]),   // bandwidth
                                    stod (l[8]),   // cost
                                    stod (l[9]),   // tp
                                    type);
                                   
            servers.push_back (server);
        } file.close ();
    } else {cerr << "ERROR: The file could not be opened." << endl;}
}

bool round_cplex (double value) {
    if (abs (value) < 1e-6) {
        return 0;
    } else if (abs (value - 1.0) < 1e-6) {
        return 1;
    }
}

void solver_model (vector <Device> &devices, vector <Server> &servers, Metrics &m) {

    try {
        IloEnv env; // declara o ambiente
        IloModel model (env); // adiciona ambiente ao modelo
 
        /* #region Variables: */ 
        IloNumVarArray w_d (env, devices.size ());
        for (int d = 0; d < devices.size (); d++) { w_d[d] = IloBoolVar (env);
            char varname[10]; sprintf (varname, "w_%d", d); w_d[d].setName (varname);}  

        NumVar2Matrix x_i_d (env, servers.size ());
        for (int i = 0; i < servers.size (); i++) { x_i_d[i] = IloNumVarArray (env, devices.size ());
            for (int d = 0; d < devices.size (); d++) {x_i_d[i][d] = IloBoolVar (env);
                char varname[15]; sprintf (varname, "x_%d_%d", i, d); x_i_d[i][d].setName (varname);}}

        IloNumVarArray z_i (env, servers.size ());
        for (int i = 0; i < servers.size (); i++) {z_i[i] = IloBoolVar (env);
            char varname[10]; sprintf (varname, "z_%i", i); z_i[i].setName (varname);}
        /* #endregion */

        /* #region Objective Function: */
        IloExpr o_f (env); // cria função objetivo classica.

        if        (m.solver == 1) { // modelo 1.
            for (int d = 0; d < devices.size (); d++) {o_f += (w_d[d] * M);} // se w_d = 1, dispositivo d não atendido, porntanto soma a penalidade M.
            o_f += ((m.devices - m.devices_covered) * M);
        } else if (m.solver == 2) { // modelo 2.
            for (int i = 0; i < servers.size (); i++) {o_f += (servers[i].cost * z_i[i]);} // se z_i = 1, servidor i ativo, portanto soma custo o da ativação.
            for (int d = 0; d < devices.size (); d++) {o_f += (devices[d].cost * w_d[d]);} // se w_d = 1, dispositivo d não atendido, portanto soma o custo do não atendimento.
            o_f += m.cd_nCovered;
        } else                    { // modelo 3.

            /* #region Extra Variable: */ 
            IloNumVar TR_max (env, 0, IloInfinity, ILOFLOAT);
            char varname[10]; sprintf (varname, "TR_max"); TR_max.setName (varname);
            /* #endregion */

            /* #region Extra C: */
            for (int d = 0; d < devices.size (); d++) { // restrição 7.
                for (Dserver &i : devices[d].servers) {model.add ((i.tr * x_i_d[i.main][d]) <= TR_max);}} 
           
            IloExpr C_8 (env); // restrição 8, função objetivo do modelo 2.
            for (int i = 0; i < servers.size (); i++) {C_8 += (servers[i].cost * z_i[i]);} // se z_i = 1, servidor i ativo, portanto soma custo o da ativação.
            for (int d = 0; d < devices.size (); d++) {C_8 += (devices[d].cost * w_d[d]);} // se w_d = 1, dispositivo d não atendido, portanto soma o custo do não atendimento.
            model.add (C_8 <= ((m.total_costs * m.alpha) - m.cd_nCovered)); C_8.end ();
            /* #endregion */

            o_f = TR_max; // minizar o tempo de resposta médio.
        }

        model.add (IloMinimize (env, o_f)); o_f.end (); // indica ao solucionador que o modelo deve ser minimizado.
        /* #endregion */
  
        /* #region Constraints: */
        for (int d = 0; d < devices.size (); d++) { // restrição 1.
            IloExpr C_1 (env);
            for (Dserver &i : devices[d].servers) {C_1 += x_i_d[i.main][d];}
            model.add (C_1 == (1 - w_d[d]));
            C_1.end ();
        } 
   
        for (int i = 0; i < servers.size (); i++) { // restrição 2.
            IloExpr C_2 (env);
            for (int d  = 0; d < devices.size (); d++) {C_2 += (devices[d].bw * x_i_d[i][d]);}
            model.add (C_2 <= (servers[i].bw * z_i[i]));
            C_2.end ();
        }

        for (int i = 0; i < servers.size (); i++) { // restrição 3.
            IloExpr C_3 (env);
            for (int d  = 0; d < devices.size (); d++) {C_3 += (devices[d].mem * x_i_d[i][d]);}
            model.add (C_3 <= (servers[i].mem * z_i[i])); C_3.end ();
        }

        for (int i = 0; i < servers.size (); i++) { // restrição 4.
            IloExpr C_4 (env);
            for (int d  = 0; d < devices.size (); d++) {C_4 += (devices[d].pcn * x_i_d[i][d]);}
            model.add (C_4 <= (servers[i].pcn * z_i[i])); C_4.end ();
        }

        for (int i = 0; i < servers.size (); i++) { // restrição 5.
            IloExpr C_5 (env);
            for (int d  = 0; d < devices.size (); d++) {C_5 += (devices[d].pcc * x_i_d[i][d]);}
            model.add (C_5 <= (servers[i].pcc * z_i[i])); C_5.end ();
        }

        for (int i = 0; i < servers.size (); i++) { // restrição 6.
            IloExpr C_6 (env);
            for (int d  = 0; d < devices.size (); d++) {C_6 += (devices[d].st * x_i_d[i][d]);}
            model.add (C_6 <= (servers[i].st * z_i[i])); C_6.end ();
        }
        /* #endregion */
   
        /* #region CPLEX: */
        IloCplex cplex (env);
        cplex.extract (model); // extrai o modelo.

        // Exporta o modelo dependo de qual for:
                
        try {
            cplex.setParam (IloCplex::Param::Threads, 1); // para uma úinica thread.
            //cplex.setParam (IloCplex::EpGap, 1e-5); // alta precisão para o Gap .
            cplex.setParam (IloCplex::TiLim, 1800); // limite de tempo em segundos.
            cplex.setParam (IloCplex::Param::MIP::Tolerances::Integrality, 1e-10); // BILP.

            if (!fs::exists ("Results/")) {fs::create_directories ("Results/");}
            if (!fs::exists ("Results/Logs")) {fs::create_directories ("Results/Logs");}
            ofstream logFile ("Results/Logs/D"+ to_string(m.devices) +"_"+ to_string(m.technology) +"G_M"+ to_string(m.solver) +"_"+ to_string(m.alpha) +".txt", ios::app);
            ofstream warningFile ("Results/cplex_warnings.txt", ios::app);
            cplex.setOut (logFile); cplex.setWarning (warningFile);
            
            if (!fs::exists ("Results/ExportModel/")) {fs::create_directories ("Results/ExportModel/");}
            string fileName = "D" + to_string(m.devices) +"_"+ to_string(m.technology) +"G_M"+ to_string(m.solver) +"_"+ to_string(m.alpha) +".lp";
            cplex.exportModel (fileName.c_str ());
            if (fs::exists (fileName)) {fs::rename(fileName, "Results/ExportModel/" + fileName);}

            double startTime = cplex.getCplexTime (); // marca o inicio do solucionador.
            if (!cplex.solve ()) { // resolve o modelo de fato.
                ostringstream status; status << cplex.getStatus (); m.solver_status = status.str ();
                m.total_costs = 0.0;
                for (int d = 0; d < devices.size (); d++) {w_d[d].end ();}
                for (int i = 0; i < servers.size (); i++) {z_i[i].end (); for (int d = 0; d < devices.size (); d++) {x_i_d[i][d].end ();}}
                cplex.end (); model.end (); env.end ();
                logFile.close(); warningFile.close();}
            double endTime = cplex.getCplexTime (); // marca o fim do solucionador.
            
            m.solver_time += (endTime - startTime); // guarda o tempo real de funcionamento do solucionador.
            m.objective_function = cplex.getObjValue (); // guarda o resultado da função objetivo do solucionador.
            ostringstream status; status << cplex.getStatus (); m.solver_status = status.str (); // guarda o tipo de resultado do solucionador.
            
            logFile.close(); warningFile.close();
        } catch (IloCplex::Exception& e) {ofstream errorFile("Results/cplex_errors.txt", ios::app); errorFile << "ERROR CPLEX: " << e.getMessage() << endl; errorFile.close(); return;
        } catch (IloException& e) {ofstream errorFile("Results/cplex_errors.txt", ios::app); errorFile << "ERROR CPLEX: Exceção do ILOG: " << e.getMessage() << endl; return;
        } catch (exception& e) {ofstream errorFile("Results/cplex_errors.txt", ios::app); errorFile << "ERROR CPLEX: Exceção genérica: " << e.what() << endl; return;
        } catch (int e) {ofstream errorFile("Results/cplex_errors.txt", ios::app); errorFile << "ERROR CPLEX: Exceção desconhecida: " << e << endl; return;}
        /* #endregion */

        /* #region Metrics: */
        // Calcula o custo dos dispositivos d não atendidos e incrimenta informações importantes aos atendidos:
        vector <Device> devices_served; // declaração de um vetor de dispositivos, para guardar os dispositivos d atendidos.
        set <int> servers_used; // declaração de um vetor, sem repetição, de servidores, para guardar os servidores i ativados.
        for (int d = 0; d < devices.size (); d++) { // percorre todos os dispositivos d cobertos.
            if (!round_cplex (cplex.getValue (w_d[d]))) { // verifica se o dispositivo d foi atendido, 0 se sim.
                for (Dserver i : devices[d].servers) { // percorre todos os servidores que cobrem o dispositivo d.
                    if (round_cplex (cplex.getValue (x_i_d[i.main][d]))) { // verifica em qual servidor i o dispositivo d foi atendido, 1 se sim.
                        devices[d].server = i; // guarda o servidor i em um dos atributos, "server", do dispositivo d, sendo este o servidor onde ele foi atendido.
                        servers_used.insert (i.main); // vetor sem repetição que contem todos os servidores ativos/usados. 
                        devices_served.push_back (devices[d]); // adiciona o dispositivo atendido a um vetor.
                        if (i.main < servers.size () - 5) {m.devices_served_ec++; // contador dos dispositivos d atendidos em servidores EC.
                        } else {m.devices_served_cc++;} // contador dos dispositivos d atendidos em servidores CC.
                    }
                }
            } else {m.cd_nServerd += devices[d].cost;} // soma os custos dos dispositivos d não atendidos aos dispositivos não cobertos.
        }

        // Calcula o custo dos servidores ativos:
        for (int i : servers_used) {
            m.server_costs += servers[i].cost; // soma os custos dos servidores i ativados.
            if (i < servers.size () - 5) {m.servers_used_ec++;} else {m.servers_used_cc++;} // conta cada tipo de servidor usado.
        }

        // Calcula o tempo de resposta máximo para usar como comparativo:
        if (!devices_served.empty ()) {for (Device &d : devices_served) {if (m.TR_max < d.server.tr) {m.TR_max = d.server.tr;}}}

        m.devices_covered_served = m.devices_served = devices_served.size ();
        m.servers_used           = servers_used.size ();
        m.total_costs            = m.server_costs + m.cd_nCovered + m.cd_nServerd;
        /* #endregion */     
        for (int d = 0; d < devices.size (); d++) {w_d[d].end ();}
        for (int i = 0; i < servers.size (); i++) {z_i[i].end (); for (int d = 0; d < devices.size (); d++) {x_i_d[i][d].end ();}}
        cplex.end (); model.end (); env.end ();
    } catch (int e) {cerr << "ERROR CPLEX: Exceção desconhecida: " << e << endl; return;}
}

void models (vector <Device> &devices, vector <Server> &servers, Metrics &m) {
    
    for (Device &d : devices) {d.server = {-1, -1, 0.0, 0.0, 0.0, 0.0};}
    if (m.devices_covered != 0) {solver_model (devices, servers, m);} // chama novamente a função que soluciona os modelos, neste caso somente para o modelo 3.
    m.show (); // mostra o resultado do solucionador do modelo 3.
    m.save (); // salva em um arquivo os resultados do solucionador.
}

void model_3 (vector <Device> &devices, vector <Server> &servers, Metrics &m2, vector <double> factors) {
    vector <Metrics> m3 (factors.size (), Metrics (m2, 1.0));
    for (int k = 0; k < factors.size (); k++) { 
        m3[k].alpha = factors[k];
        models (devices, servers, m3[k]);
    }
}