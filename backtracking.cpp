#include <bits/stdc++.h>
using namespace std;

/*
backtracking ,testa TODAS as solucoes possiveis (cortando as que
nao tem como ser melhores) e no fim tem a otima teoricamente

compilar: g++ -O2 backtracking.cpp -o backtracking
rodar:    ./backtracking [tempo limite] [semPoda] < instances/X.txt
*/

int n;

// igualzinho ao da mcws
struct node {

  int id;
  string id2;
  double x, y;
  int demand; // PARA AFSs, DEMAND EH
  bool is_afs_recharging_station;

  node(int idd, string idd2, double xcord, double ycord, int demandd,
       bool is_afs = false) {
    id2 = idd2;
    id = idd;
    x = xcord;
    y = ycord;
    demand = demandd;
    is_afs_recharging_station = is_afs;
  }
};

double max_distance;       // TL * v: milhas que uma rota pode rodar
double full_tank_capacity; // autonomia em milhas (Q / r) who the hell uses
                           // milhas/hr
vector<node> nodes;        // nodes[0] e o deposito
vector<int> afs_estacoes_reabastecimento;

node depot = node(0, "depot", 0, 0, 0);

double velocidade_mph;
double duracao_maxima_h;
int max_veiculos;

bool eh_cliente(int v) {
  return v != 0 && !nodes.at(v).is_afs_recharging_station;
}

// folga pra comparar double
const double EPS = 1e-9;

double calculateEuclideanDistance(node node1, node node2) {
  double R = 4182.44949, g = M_PI / 180;
  double dlat = (node2.y - node1.y) * g, dlon = (node2.x - node1.x) * g;
  double s = sin(dlat / 2) * sin(dlat / 2) + cos(node1.y * g) *
                                                 cos(node2.y * g) *
                                                 sin(dlon / 2) * sin(dlon / 2);
  return 2 * R * asin(sqrt(s));
}

void initialize() {

  string line;

  auto read_parameter = [&]() {
    while (line.find('/') == string::npos && getline(cin, line)) {
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

  while (getline(cin, line)) {
    stringstream ss(line);
    if (!(ss >> id2 >> type >> lon >> lat)) {
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

  depot = nodes.at(0);

  duracao_maxima_h = TL;
  velocidade_mph = v;
  max_veiculos = (int)num_vehicles;
}

/*
calcula uma vez a matriz n x n com a distancia entre todos os pares
 de nos.
 a busca consulta distancias ler da
 matriz e muito mais rapido que recalcular toda vez.
*/
vector<vector<double>> dist;
void montar_matriz() {
  dist.assign(n, vector<double>(n));
  for (int i = 0; i < n; i++)
    for (int j = 0; j < n; j++)
      dist[i][j] = calculateEuclideanDistance(nodes[i], nodes[j]);
}

// de u, com comb de combustivel e rodado milhas ja feitas na rota, eu
// consigo voltar pro depot? primeiro direto, se nao der testo passando
// por cada afs (mesma ideia do initialize_try_to_make_more_feasible da mcws)
bool volta(int u, double comb, double rodado) {
  auto ok = [&](double c, double total, int v) {
    return dist[u][v] <= c + EPS && total <= max_distance + EPS;
  };
  if (ok(comb, rodado + dist[u][0], 0))
    return true;
  for (int f : afs_estacoes_reabastecimento)
    if (ok(comb, rodado + dist[u][f] + dist[f][0], f) &&
        dist[f][0] <= full_tank_capacity + EPS)
      return true;
  return false;
}

// estado de uma solucao parcial
// cada chamada da busca recebe a sua propria copia
struct estado {
  int no_atual = 0;       // onde o veiculo esta
  double combustivel = 0; // milhas que ainda da para rodar com o tanque atual
  double distancia_rota = 0; // milhas rodadas na rota atual
  double custo = 0;          // distancia acumulada de todas as rotas
  int rotas_abertas = 1;     // veiculos usados ate agora
  int clientes_restantes = 0;
  vector<bool> visitado;         // por no
  vector<int> afs_desde_cliente; // AFS visitadas desde o ultimo cliente
  vector<vector<int>> rotas;     // rotas montadas ate agora
};

// posso ir de onde eu to pra destino?
// cliente: nao foi visitado e de la ainda da pra voltar
// afs: o tanque nao ta cheio e nao passei nela desde o ultimo cliente big
// problem deposito: a rota tem cliente, cabe no limite e nao estoura o m
bool movimento_viavel(const estado &e, int destino) {
  int u = e.no_atual, v = destino;
  double d = dist[u][v], rodado = e.distancia_rota + d;
  if (v == u || d > e.combustivel + EPS)
    return false;

  if (v == 0) {
    auto &r = e.rotas.back();
    bool tem_cliente =
        any_of(r.begin(), r.end(), [](int x) { return eh_cliente(x); });
    // chegar no deposito com cliente sobrando abre outra rota
    if (e.clientes_restantes > 0 && e.rotas_abertas == max_veiculos)
      return false;
    return tem_cliente && rodado <= max_distance + EPS;
  }
  if (eh_cliente(v))
    return !e.visitado[v] && volta(v, e.combustivel - d, rodado);

  // AFS
  if (e.combustivel >= full_tank_capacity - EPS)
    return false;
  for (int f : e.afs_desde_cliente)
    if (f == v)
      return false;
  return volta(v, full_tank_capacity, rodado);
}

// faz a viagem: copia o estado, soma a distancia, gasta combustivel e anota na
// rota. depois depende:
// deposito: enche o tanque, fecha a rota e abre outra se ainda falta cliente
// cliente: marca como visitado
// afs: enche o tanque
// loona: infelizmente morreu
estado aplicar_movimento(const estado &e, int destino) {
  estado s = e;
  int v = destino;
  double d = dist[e.no_atual][v];
  s.custo += d, s.combustivel -= d, s.distancia_rota += d;
  s.no_atual = v;
  s.rotas.back().push_back(v);

  if (v == 0) {
    s.combustivel = full_tank_capacity, s.distancia_rota = 0;
    s.afs_desde_cliente.clear();
    if (s.clientes_restantes)
      s.rotas_abertas++, s.rotas.push_back({0});
  } else if (eh_cliente(v)) {
    s.visitado[v] = true, s.clientes_restantes--;
    s.afs_desde_cliente.clear();
  } else {
    s.combustivel = full_tank_capacity;
    s.afs_desde_cliente.push_back(v);
  }
  return s;
}

double melhor_custo = 1e18;
vector<vector<int>> melhor_solucao;
long long nos_explorados = 0, nos_podados = 0;
bool estourou_tempo = false;
double limite_tempo_s = 3000;
bool PODA = true;
chrono::steady_clock::time_point inicio;

/*
ela se chama de novo pra cada movimento possivel, entao
no fim ela olhou todas as solucoes (menos as que a poda cortou, que nao tinham
como ganhar da melhor)
*/
void busca(const estado &e) {
  // 1. conto o no
  nos_explorados++;
  if (estourou_tempo)
    return;
  if ((nos_explorados % 1000) == 0 &&
      chrono::duration<double>(chrono::steady_clock::now() - inicio).count() >
          limite_tempo_s) {
    estourou_tempo = true;
    return;
  }
  // poda: se ja gastei mais que a melhor, desistit.exe
  // pode porque distancia nunca eh negativa, o custo so aumenta daqui pra
  // frente
  if (PODA && e.custo >= melhor_custo - EPS) {
    nos_podados++;
    return;
  }
  // todo mundo atendido e to no deposito: achei uma solucao
  if (e.clientes_restantes == 0 && e.no_atual == 0) {
    if (e.custo < melhor_custo)
      melhor_custo = e.custo, melhor_solucao = e.rotas;
    return;
  }
  // pego os destinos que da pra ir, o mais perto primeiro
  //  (a ordem nao muda a resposta, so acha solucao boa mais cedo e poda mais
  //  teoricamente)
  vector<int> dest;
  for (int v = 0; v < n; v++)
    if (movimento_viavel(e, v))
      dest.push_back(v);
  sort(dest.begin(), dest.end(),
       [&](int a, int b) { return dist[e.no_atual][a] < dist[e.no_atual][b]; });
  // 6. desce em cada um
  for (int v : dest) {
    busca(aplicar_movimento(e, v));
    if (estourou_tempo)
      return;
  }
}

void backtracking() {
  estado e;
  e.combustivel = full_tank_capacity;
  e.visitado.assign(n, false);
  e.rotas = {{0}};
  for (int i = 0; i < n; i++)
    e.clientes_restantes += eh_cliente(i);

  inicio = chrono::steady_clock::now();
  busca(e);
  double seg =
      chrono::duration<double>(chrono::steady_clock::now() - inicio).count();

  if (melhor_solucao.empty())
    cout << "nenhuma solucao viavel com " << max_veiculos << " rotas"
         << (estourou_tempo ? " (ou o tempo acabou antes de achar uma)" : "")
         << endl;
  for (int i = 0; i < (int)melhor_solucao.size(); i++) {
    double d = 0;
    for (int j = 1; j < (int)melhor_solucao[i].size(); j++)
      d += dist[melhor_solucao[i][j - 1]][melhor_solucao[i][j]];
    cout << "rota " << i + 1 << ":";
    for (int v : melhor_solucao[i])
      cout << " " << nodes[v].id2;
    cout << fixed << setprecision(2) << " (" << d << " mi)\n";
  }
  if (!melhor_solucao.empty())
    cout << fixed << setprecision(2) << "distancia total: " << melhor_custo
         << " mi\n";
  cout << fixed << setprecision(3) << "tempo: " << seg << " s\n"
       << "nos explorados: " << nos_explorados << ", podados: " << nos_podados
       << "\n"
       << (estourou_tempo ? "parado por tempo" : "otimo provado") << "\n";
}

signed main(int argc, char **argv) {
  for (int i = 1; i < argc; i++) {
    string a = argv[i];
    if (a == "semPoda")
      PODA = false;
    else
      limite_tempo_s = stod(a);
  }
  initialize();
  cout << n << " nos, " << afs_estacoes_reabastecimento.size()
       << " AFS, autonomia " << full_tank_capacity << " mi, limite "
       << duracao_maxima_h << " h x " << velocidade_mph
       << " mph = " << max_distance << " mi por rota, m = " << max_veiculos
       << endl;
  montar_matriz();
  backtracking();
  return 0;
}
