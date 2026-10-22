/*
pelo que eu entendi os feasible tours sao disjuntos
logo, eles sao uma solucao no fim da computacao

essa heuristica sempre assume que o numero k de afs (recharging stations)
eh pequeno o suficiente (eles sempre sao brutados)
*/

#include <bits/stdc++.h>
using namespace std;
// #define int long long

int n;

struct node
{

  int id;
  string id2;
  double x, y;
  int demand; // PARA AFSs, DEMAND EH
  bool is_afs_recharging_station;

  node(int idd, string idd2, double xcord, double ycord, int demandd,
       bool is_afs = false)
  {
    id2 = idd2;
    id = idd;
    x = xcord;
    y = ycord;
    demand = demandd;
    is_afs_recharging_station = is_afs;
  }
};

struct tour
{
  double distance;
  double fuel;
  deque<int> lista;
  bool feasible = true;
  bool has_afs = false;

  double distance_rightmost_afs;
  double distance_leftmost_afs;

  tour()
  {
    distance = 0;
    // incluso.assign(n, 0);
  }

  void concat_lists(tour t2)
  {
    lista.insert(lista.end(), t2.lista.begin(), t2.lista.end());
  }
};

double max_distance;
double full_tank_capacity;
vector<node> nodes;
vector<tour> tours;
deque<tour> infeasible, feasible;
vector<int> afs_estacoes_reabastecimento;
deque<pair<double, pair<int, int>>> savings_pair_list;

node depot = node(0, "depot", 0, 0, 0);


vector<int> pai;
vector<int> rank_dsu;

// calculates the euclidean distance between two nodes
double calculateEuclideanDistance(node node1, node node2)
{
  double R = 4182.44949, g = M_PI / 180;
  double dlat = (node2.y - node1.y) * g, dlon = (node2.x - node1.x) * g;
  double s = sin(dlat / 2) * sin(dlat / 2) + cos(node1.y * g) *
                                                 cos(node2.y * g) *
                                                 sin(dlon / 2) * sin(dlon / 2);
  return 2 * R * asin(sqrt(s));
}

int find(int x)
{
  if (x == pai[x])
  {
    return x;
  }
  else
    return x = find(pai[x]);
}


// peco perdao essa funcao foi projetada da PIOR MANEIRA POSSIVEL mas agora eh tarde demais
/*
essa funcao funciona assim:
ele faz o unite na dsu com compressao de caminhos e afins
adiciona o custo TOTAL do tour y ao tour x
adiciona o custo passado (que deve ser equivalente a distancia de vi a vj - (distancia de vi ate depot + distancia de vj ate depot))
espero ter feito sentido
*/

void unite(int x, int y, double cost)
{
  x = find(pai[x]);
  y = find(pai[y]);

  if (x == y)
    return;

//   tours.at(x).distance_rightmost_afs = tours.at(y).distance_rightmost_afs;

  tours.at(x).distance += cost + tours.at(y).distance;
  // mais a distance do y menos a distance ate o depot
  tours.at(x).concat_lists(tours.at(y));

  pai[y] = pai[x];

  // if (rank_dsu[x] > rank_dsu[y]) {
  //     pai[y] = x;
  // }
  // else if (rank_dsu[x] > rank_dsu[y]) {
  //     pai[x] = y;
  // }
  // else {
  //     pai[y] = x;
  //     rank_dsu[x]++;
  // }
}

void rearrange(int x, int y, bool b1, bool b2) {

    x = find(x);
    y = find(y);

    if (!b1 && !b2) {
        reverse(tours.at(y).lista.begin(), tours.at(y).lista.end());
        swap(tours.at(y).distance_leftmost_afs, tours.at(y).distance_rightmost_afs);
    }

    // direita esquerda do nothing


    // esquerda direita
    else if (b1 && !b2) {
        reverse(tours.at(x).lista.begin(), tours.at(x).lista.end());
        reverse(tours.at(y).lista.begin(), tours.at(y).lista.end());
        swap(tours.at(y).distance_leftmost_afs, tours.at(y).distance_rightmost_afs);
        swap(tours.at(x).distance_leftmost_afs, tours.at(x).distance_rightmost_afs);
    }

    // esquerda esquerda
    else if (b1 && b2) {
        reverse(tours.at(x).lista.begin(), tours.at(x).lista.end());
        swap(tours.at(x).distance_leftmost_afs, tours.at(x).distance_rightmost_afs);
    }


}

void update_afs_dist(tour & t1, tour & t2, bool b1, bool b2) {

    if (!t1.has_afs && !t2.has_afs) { // sempre junto no da direita
        // double og_t1_leftmost_dist = t1.distance_leftmost_afs;
        t1.distance_leftmost_afs += t2.distance_leftmost_afs - calculateEuclideanDistance(nodes.at(t2.lista.front()), depot) + calculateEuclideanDistance(nodes.at(t2.lista.front()), nodes.at(t1.lista.back()));
        t1.distance_rightmost_afs = t2.distance_rightmost_afs + t1.distance_rightmost_afs - calculateEuclideanDistance(nodes.at(t1.lista.back()), depot) + calculateEuclideanDistance(nodes.at(t2.lista.front()), nodes.at(t1.lista.back()));
    }
    else if (!t1.has_afs && t2.has_afs) {
        t1.distance_leftmost_afs += t2.distance_leftmost_afs - calculateEuclideanDistance(nodes.at(t2.lista.front()), depot) + calculateEuclideanDistance(nodes.at(t2.lista.front()), nodes.at(t1.lista.back()));
        t1.distance_rightmost_afs = t2.distance_rightmost_afs;

    }
    else if (t1.has_afs && !t2.has_afs) {
        t1.distance_leftmost_afs = t1.distance_leftmost_afs;
        t1.distance_rightmost_afs += t2.distance_leftmost_afs - calculateEuclideanDistance(nodes.at(t1.lista.back()), depot) + calculateEuclideanDistance(nodes.at(t2.lista.front()), nodes.at(t1.lista.back()));

    }
    else {
        t1.distance_rightmost_afs = t2.distance_rightmost_afs;
    }
}

bool decCompare(pair<double, pair<int, int>> a, pair<double, pair<int, int>> b)
{
  return a.first > b.first;
}



// inicializa recebendo as instancias e colocando num vetor global
// equivalent to steps 1 and 2
void initialize()
{

  // if () {
  //     "couldn't open filepath; please check its integrity.\n";
  //     return;
  // }
  string line;

  auto read_parameter = [&]()
  {
    while (line.find('/') == string::npos && getline(cin, line))
    {
    }
    double valor = stod(line.substr(line.find('/') + 1));
    line.clear();
    return valor;
  };

  string id2;
  char type;
  int id;
  double lon, lat;
  double num_vehicles;

  while (getline(cin, line))
  {
    stringstream ss(line);
    if (!(ss >> id2 >> type >> lon >> lat))
    {
      if (nodes.empty())
        continue;
      break;
    }
    int id = nodes.size();
    nodes.push_back(node(id, id2, lon, lat, 0, type == 'f'));
    if (type == 'f')
      afs_estacoes_reabastecimento.push_back(id);
  }

  n = nodes.size();

  double Q = read_parameter();
  double r = read_parameter();
  double TL = read_parameter();
  double v = read_parameter();
  num_vehicles = read_parameter();
  full_tank_capacity = Q / r;
  max_distance = TL * v;

  // QUE PORRA EH MUTUALLY EXCLUSIVE GROUP

  // colocar aqui rotas iniciais

  depot = nodes.at(0);

  double calc;
  tour tt;
  tours.push_back(tt); // o 0 pra poder indexar corretamente
  pai.push_back(0);

  for (int i = 1; i < n; i++)
  {
    pai.push_back(i);
    calc = 2 * calculateEuclideanDistance(nodes.at(0), nodes.at(i));
    tour t;
    t.distance = calc;
    t.distance_leftmost_afs = calc/2;
    t.distance_rightmost_afs = t.distance_leftmost_afs;

    if (nodes.at(i).is_afs_recharging_station) {
        tours.push_back(t);
        continue;
    }

    
    // t.lista.push_back(0);
    t.lista.push_back(i);
    // // t.lista.push_back(0);
    // t.incluso.at(0) = true;
    // t.incluso.at(i) = true;
    t.fuel = calc;
    
    if (calc <= full_tank_capacity && calc <= max_distance)
    {
      tours.push_back(t);
      feasible.push_back(t);
    }

    else
    {
        t.feasible = false;
        tours.push_back(t);
      infeasible.push_back(t);
    }
  }

  return;
}

// THIS IS EQUIVALENT TO STEP 3!!!!!!!!!!!!!!!!!!!

void initialize_try_to_make_more_feasible()
{
  vector<int> tirar;

  for (int i = 0; i < infeasible.size(); i++)
  {

    double minimo = min(max_distance, full_tank_capacity);
    int onde_eh_o_minimo = -1;
    for (int j = 0; j < afs_estacoes_reabastecimento.size(); j++)
    {

      // a distancia sempre eh ida e volta;
      double distancia_node_ate_afs = calculateEuclideanDistance(
          nodes.at(infeasible.at(i).lista.back()),
          nodes.at(afs_estacoes_reabastecimento.at(j)));
      double distancia_afs_ate_depot = calculateEuclideanDistance(
          depot, nodes.at(afs_estacoes_reabastecimento.at(j)));

      // consigo chegar na AFS em questao e consigo voltar dela pro depot? se
      // sim, atualizo o minimo

      if (infeasible.at(i).distance / 2 + distancia_node_ate_afs + distancia_afs_ate_depot <= max_distance &&
          distancia_afs_ate_depot <= full_tank_capacity &&
          infeasible.at(i).distance / 2 + distancia_node_ate_afs <= full_tank_capacity)
      {

        double tempdist = infeasible.at(i).distance -
                          infeasible.at(i).fuel / 2 + distancia_node_ate_afs +
                          distancia_afs_ate_depot;
        minimo = min(minimo, tempdist);

        if (tempdist < minimo)
        {
          minimo = tempdist;
          onde_eh_o_minimo = j;
        }

        // retirar do infeasible; talvez seja bom mudar isso aq pra um deque
      }
      else
      {
        // continua infeasible
      }
      // checar se da pra adicionar 1 afs e deixar feasible; se nao fodase nao
      // comecamos por eles
    }

    if (onde_eh_o_minimo != -1)
    {

      tour t = infeasible.at(i);
      tirar.push_back(i);

      double distancia_node_ate_afs = calculateEuclideanDistance(
          nodes.at(infeasible.at(i).lista.back()),
          nodes.at(afs_estacoes_reabastecimento.at(onde_eh_o_minimo)));
      double distancia_afs_ate_depot = calculateEuclideanDistance(
          nodes.at(0),
          nodes.at(afs_estacoes_reabastecimento.at(onde_eh_o_minimo)));

      t.distance -= infeasible.at(i).fuel / 2;
      t.distance += distancia_node_ate_afs + distancia_afs_ate_depot;
      t.fuel = full_tank_capacity - distancia_afs_ate_depot;

      feasible.push_back(t);
    }
  }
  deque<tour> refeito;
  int j = 0;

  for (int i = 0; i < infeasible.size(); i++) {
    if (j < tirar.size() && tirar[j] == i) {
      j++;
    }
    else {
      refeito.push_back(infeasible.at(i));
    }
  }

  infeasible = refeito;
}

// EQUIVALENT TO STEP 4 !!!!!!!!!!!!!!!!!!!!!!!!!!
void add_to_spl(int inicial, int final, vector<int> bordas)
{

  vector<int> compare;
  compare.push_back(inicial);

  if (inicial != final)
  {
    compare.push_back(final);
  }

  for (int i = 0; i < bordas.size(); i++)
  {
    for (int j = 0; j < compare.size(); j++)
    {
      // d(v0,vi) + d(v0,vj) - d(vi,vj)
      double cost = calculateEuclideanDistance(depot, nodes.at(bordas.at(i))) + calculateEuclideanDistance(depot, nodes.at(compare.at(j))) - calculateEuclideanDistance(nodes.at(bordas.at(i)), nodes.at(compare.at(j)));
      savings_pair_list.push_back({cost, {bordas.at(i), compare.at(j)}});
    }
    // calcular o quanto ganha
  }
}

void compute_savings()
{

  vector<int> bordas;

  // o inicio e o fim sao sempre obrigatoriamente o depot

  // como os conjuntos sao disjuntos, nao precisamos colocar em um set!

  for (int i = 1; i < tours.size(); i++)
  {
    if (nodes.at(i).is_afs_recharging_station || find(i) != i) { // o pai eh diferente, nao adianta
      continue;
    }
    int inicial = tours.at(i).lista.front();
    int final = tours.at(i).lista.back();
    
    add_to_spl(inicial, final, bordas);

    bordas.push_back(inicial);
    if (inicial != final) {
        bordas.push_back(final);
    }
  }

  sort(savings_pair_list.begin(), savings_pair_list.end(), decCompare);
}

/*
essa logica ta muito convoluta mas foi o que eu consegui pensar

essa funcao vai ocorrer enquanto ainda ha pares a serem avaliados (possiveis unioes)

precisamos avaliar se eu posso fazer a uniao indicada ()

*/

// EQUIVALENT TO STEP 5!!!!!!!!!!!

void merge_tours(bool &houve_mudanca)
{
    houve_mudanca = false;
  // farei uma dsu para facilitar nossa vida hehehe

  // tenho a lista de todos os pares das pontas ORDENADOS
  while (savings_pair_list.size())
  {
    auto par_merge = savings_pair_list.front();
    savings_pair_list.pop_front();

    if (find(par_merge.second.first) == find(par_merge.second.second))
      continue;
    
    if (!tours.at(par_merge.second.first).feasible && !tours.at(par_merge.second.second).feasible) {
      continue;
    }
    
      if (!tours.at(par_merge.second.first).feasible) {
        swap(par_merge.second.first, par_merge.second.second);
      }

    // vejo se posso unir
    // se posso, concateno uma lista na outra
    // se nao, vejo se consigo adicionando algum posto de abastecimento
    // se nao, f, continua igual

    tour tour_equivalente1 = tours.at(find(par_merge.second.first));
    tour tour_equivalente2 = tours.at(find(par_merge.second.second));

    // double d1 = tour_equivalente1.distance;
    // double d2 = tour_equivalente2.distance;

    bool left1, left2;

    left1 = (par_merge.second.first == tour_equivalente1.lista.front());
    left2 = (par_merge.second.second == tour_equivalente2.lista.front());

    rearrange(par_merge.second.first, par_merge.second.second, left1, left2);

    double distance_since_afs1 = tour_equivalente1.distance_rightmost_afs;
    double distance_since_afs2 = tour_equivalente2.distance_leftmost_afs;

    if (distance_since_afs1 + distance_since_afs2 + calculateEuclideanDistance(nodes.at(par_merge.second.first), nodes.at(par_merge.second.second)) <= full_tank_capacity &&
        tour_equivalente1.distance + tour_equivalente2.distance - par_merge.first <= max_distance) {
          update_afs_dist(tours.at(find(par_merge.second.first)), tours.at(find(par_merge.second.second)), left1, left2);
          unite(par_merge.second.first, par_merge.second.second, -1*par_merge.first);
        houve_mudanca = true;

    }

    else if (tour_equivalente1.distance + tour_equivalente2.distance - par_merge.first > max_distance) {
        // do nothing
    }

    else {
        // testar todas as afs
        bool da = false;
        double dist = 1000000000;
        int onde;
        for (int i = 0; i < afs_estacoes_reabastecimento.size(); i++) {
            if (distance_since_afs1 + calculateEuclideanDistance(nodes.at(par_merge.second.first), nodes.at(afs_estacoes_reabastecimento.at(i))) <= full_tank_capacity &&
                distance_since_afs2 + calculateEuclideanDistance(nodes.at(par_merge.second.second), nodes.at(afs_estacoes_reabastecimento.at(i))) <= full_tank_capacity) {
                da = true;
                if (calculateEuclideanDistance(nodes.at(par_merge.second.first), nodes.at(afs_estacoes_reabastecimento.at(i))) + calculateEuclideanDistance(nodes.at(par_merge.second.second), nodes.at(afs_estacoes_reabastecimento.at(i))) < dist) {
                    dist = calculateEuclideanDistance(nodes.at(par_merge.second.first), nodes.at(afs_estacoes_reabastecimento.at(i)));
                    // essa aq ja eh calculada
                    // + calculateEuclideanDistance(nodes.at(par_merge.second.second), nodes.at(afs_estacoes_reabastecimento.at(i)))
                    onde = i;
                }
            }
        }

        if (da) {
            tours.at(find(par_merge.second.first)).lista.emplace_back(afs_estacoes_reabastecimento.at(onde)); // to colocando na direita
            tours.at(find(par_merge.second.first)).distance += dist - calculateEuclideanDistance(depot, nodes.at(par_merge.second.first)) + calculateEuclideanDistance(nodes.at(afs_estacoes_reabastecimento.at(onde)), depot);
            // adapt over here a questao de adicionar ou nao com a ajuda do left1 e left2
            tours.at(find(par_merge.second.first)).distance_rightmost_afs = calculateEuclideanDistance(nodes.at(afs_estacoes_reabastecimento.at(onde)), depot);
            if (!tour_equivalente1.has_afs) {
              tours.at(find(par_merge.second.first)).distance_leftmost_afs += dist;
            }
            tours.at(find(par_merge.second.first)).has_afs = true;
            double cost = calculateEuclideanDistance(nodes.at(par_merge.second.second), nodes.at(afs_estacoes_reabastecimento.at(onde)));
            cost -= calculateEuclideanDistance(nodes.at(afs_estacoes_reabastecimento.at(onde)), depot) + calculateEuclideanDistance(nodes.at(par_merge.second.second), depot);
            update_afs_dist(tours.at(find(par_merge.second.first)), tours.at(find(par_merge.second.second)), left1, left2);
            unite(par_merge.second.first, par_merge.second.second, cost);
            houve_mudanca = true;
        }

    }

    



    // falta so atualizar os valores
    

    // revalue



    // else if (direita esquerda) {

    // }
    // else if (esquerda direita) {

    // }
    // else if (esquerda esquerda) {
        
    // }





//     if (par_merge.second.first == tour_equivalente1.lista.front())
//     {
//       distance_since_afs1 = tour_equivalente1.distance_leftmost_afs;
//       left1 = true;
//     }
//     else
//     {
//       distance_since_afs1 = tour_equivalente1.distance_rightmost_afs;
//       left1 = false;
//     }

//     if (par_merge.second.second == tour_equivalente2.lista.front())
//     {
//       distance_since_afs2 = tour_equivalente2.distance_leftmost_afs;
//       left2 = true;
//     }
//     else
//     {
//       distance_since_afs2 = tour_equivalente2.distance_rightmost_afs;
//       left2 = false;
//     }


//     double distancia_entre_nos_ponta = -1 * (par_merge.first - d1 - d2);

//     // double distance_since_afs1 = (par_merge.second.first == tour_equivalente1.lista.front() ? tour_equivalente1.distance_leftmost_afs : tour_equivalente1.distance_rightmost_afs);
//     // double distance_since_afs2 = (par_merge.second.second == tour_equivalente2.lista.front() ? tour_equivalente2.distance_leftmost_afs : tour_equivalente2.distance_rightmost_afs);

//     if (tour_equivalente1.distance + tour_equivalente2.distance + distancia_entre_nos_ponta - d1 - d2 <= max_distance &&
//         distance_since_afs1 + distance_since_afs2 + distancia_entre_nos_ponta <= tour_equivalente1.fuel ||
//         distance_since_afs1 + distance_since_afs2 + distancia_entre_nos_ponta <= tour_equivalente2.fuel) { // both constraints are met

//         if (distance_since_afs1 + distance_since_afs2 <= tour_equivalente1.fuel) {
//             tours.at(par_merge.second.first).concat_lists(tours.at(par_merge.second.second));
//             uniteleft(par_merge.second.first, par_merge.second.second, par_merge.first);
//         }

//         else {
//             tours.at(par_merge.second.second).concat_lists(tours.at(par_merge.second.first));
//             uniteright(par_merge.second.first, par_merge.second.second, par_merge.first);
//         }

//       houve_mudanca = true;
//     }

//     else if (tours.at(par_merge.second.first).distance + tours.at(par_merge.second.second).distance - d1 - d2 <= max_distance)
//     { // u cant add it cause the cars gonna be traveling too much
//       // do nothing lowkey
//     }

//     else
//     { // u cant add it cause u dont have enough fuel, test everything
//         bool posso = false;
//         int node1 = (left1 == true ? );

//         for (int i = 0; i < afs_estacoes_reabastecimento.size(); i++) {
//             if (distance_since_afs1 + calculateEuclideanDistance(nodes.at(afs_estacoes_reabastecimento.at(i)), nodes.at()) <= tour_equivalente1.fuel && distance_since_afs2 + calculateEuclideanDistance()) {

//             }
//             else if () {

//             }
//         }

//         if (!posso) continue;
//     }
  }
}

void compute_and_merge_tours()
{
  // farei uma dsu para facilitar nossa vida hehehe

  bool houve_mudanca = false;
  compute_savings();
  merge_tours(houve_mudanca);

  while (houve_mudanca)
  {
    houve_mudanca = false;
    compute_savings();
    merge_tours(houve_mudanca);
  }
}

signed main(signed argc, char *argv[]) {

  initialize();

  initialize_try_to_make_more_feasible();

  compute_and_merge_tours(); // 4 e 5

  int cont = 0;

  double total_distance = 0;

  bool ja[25] = {0};

  for (int i = 0; i < tours.size(); i++) {
      if (i != 0 && pai[i] == i && !nodes.at(i).is_afs_recharging_station && tours.at(i).feasible) {
        cont++;
        total_distance += tours[i].distance;
        cout << "carro " << cont << ": ";
        for (int j = 0; j < tours[i].lista.size(); j++) {
            ja[tours[i].lista[j]] = true;
            cout << tours[i].lista[j] << " ";
        }

        cout << "(" << tours[i].distance << " miles)" << "\n";
    }

  }


  if (infeasible.size()) {
    cout << "devido as restricoes definidas na instancia passada e a natureza do algoritmo implementado, os seguintes clientes nao foram inclusos na solucao: ";
  }

  for (int i = 0; i < infeasible.size(); i++) {
    /**/
    if (ja[infeasible.at(i).lista.front()]) {
      continue;
    }
    cont++;
    total_distance+= infeasible.at(i).distance;
    cout << infeasible.at(i).lista.front() << ", ";
  }

  cout << "\n\ntotal de " << cont << " carros utilizados e " << total_distance <<" milhas rodadas!\n";



  return 0;
}
