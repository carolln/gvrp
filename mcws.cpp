/**/
#include <bits/stdc++.h>
#include <fstream>
using namespace std;
#define int long long

int n;

struct node {

    int id;
    int x, y;
    int demand; // PARA AFSs, DEMAND EH 
    bool is_afs_recharging_station;

    node(int idd, int xcord, int ycord, int demandd, bool is_afs = false) {
        id = idd;
        x = xcord;
        y = ycord;
        demand = demandd;
        is_afs_recharging_station = is_afs;
    }

};

struct tour {
    double distance;
    double fuel;
    deque<int> lista;
    vector<bool> incluso;

    tour () {
        distance= 0;
        incluso.assign(n, 0);
    }
};

double max_distance;
double full_tank_capacity;
vector<node> nodes;
vector<tour> tours;
deque<tour> infeasible, feasible;
vector<int> afs_estacoes_reabastecimento;


// calculates the euclidean distance between two nodes
double calculateEuclideanDistance(node node1, node node2) {
    return sqrt((double)((node1.x - node2.x)*(node1.x - node2.x) + (node1.y - node2.y)*(node1.y - node2.y)));
}


// inicializa recebendo as instancias e colocando num vetor global
// equivalent to steps 1 and 2
void initialize(int n) {


    // if () {
    //     "couldn't open filepath; please check its integrity.\n";
    //     return;
    // }

    nodes.push_back(node(0,0,0,0)); // depot inicial

    int id, xcoord, ycoord, demand;


    for (int i = 0; i < n; i++) {
        cin >> id >> xcoord >> ycoord;

        nodes.push_back(node(id, xcoord, ycoord, 0));
    }

    for (int i = 0; i < n; i++) {
        cin >> id >> demand;

        nodes.at(id).demand = demand;
    }

    // QUE PORRA EH MUTUALLY EXCLUSIVE GROUP

    // colocar aqui rotas iniciais

    double calc;

    for (int i = 1; i <= n; i++) {

        // ADICIONAR UM CONTINUE PRA CASO FOR UMA AFS!!!!!!!!!!!!!!!!!!!
        
        calc = 2 * calculateEuclideanDistance(nodes.at(0), nodes.at(i+1));
        tour t;
        t.distance = calc;
        t.lista.push_back(0);
        t.lista.push_back(i);
        t.lista.push_back(0);
        t.incluso.at(0) = true;
        t.incluso.at(i) = true;
        t.fuel = calc;

        if (calc <= full_tank_capacity && calc <= max_distance) {
            feasible.push_back(t);
        }

        else {
            infeasible.push_back(t);
        }
    }

    return;
}


// THIS IS EQUIVALENT TO STEP 3!!!!!!!!!!!!!!!!!!!

void initialize_try_to_make_more_feasible() {

    for (int i = 0; i < infeasible.size(); i++) {

        double minimo = min(max_distance, full_tank_capacity);
        int onde_eh_o_minimo = -1;
        for (int j = 0; j < afs_estacoes_reabastecimento.size(); i++) {

            // a distancia sempre eh ida e volta;
            double distancia_node_ate_afs = calculateEuclideanDistance(nodes.at(infeasible.at(i).lista.back()), nodes.at(afs_estacoes_reabastecimento.at(j)));
            double distancia_afs_ate_depot = calculateEuclideanDistance(nodes.at(0), nodes.at(afs_estacoes_reabastecimento.at(j)));

            // consigo chegar na AFS em questao e consigo voltar dela pro depot? se sim, atualizo o minimo

            if (infeasible.at(i).fuel/2 + distancia_node_ate_afs <= max_distance && distancia_afs_ate_depot <= max_distance) {

                double tempdist = infeasible.at(i).distance - infeasible.at(i).fuel/2 + distancia_node_ate_afs + distancia_afs_ate_depot;
                minimo = min(minimo, tempdist);

                if (tempdist < minimo) {
                    minimo = tempdist;
                    onde_eh_o_minimo = j;
                }
                

                // retirar do infeasible; talvez seja bom mudar isso aq pra um deque
            }
            else {
                //continua infeasible
            }
            // checar se da pra adicionar 1 afs e deixar feasible; se nao fodase nao comecamos por eles
        }

        if (onde_eh_o_minimo != -1) {

            tour t = infeasible.at(i);


            double distancia_node_ate_afs = calculateEuclideanDistance(nodes.at(infeasible.at(i).lista.back()), nodes.at(afs_estacoes_reabastecimento.at(onde_eh_o_minimo)));
            double distancia_afs_ate_depot = calculateEuclideanDistance(nodes.at(0), nodes.at(afs_estacoes_reabastecimento.at(onde_eh_o_minimo)));

            t.distance-= infeasible.at(i).fuel/2;
            t.distance+= distancia_node_ate_afs + distancia_afs_ate_depot;
            t.fuel = full_tank_capacity - distancia_afs_ate_depot;

            feasible.push_back(t);
        }


    }
}


// EQUIVALENT TO STEP 4 !!!!!!!!!!!!!!!!!!!!!!!!!!

void compute_savings() {
    
}




signed main (int argc, string argv[]) {


    if (argc <= 1) {
        cout << "missing instance file name\n";
        return 1;
    }

    string filepath;



    return 0;
}