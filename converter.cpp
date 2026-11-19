/*
    Esse programa recebe um arquivo de instância gerado com distancia euclidiana em QUILOMETROS (Km)
    e solta um arquivo com longitude e latitude
    o arquivo de distancia euclidiana precisa fornecer primeiro o depot que vai ser considerado a posição
    (0,0). o depot PRECISA ter uma coordenada auxiliar em longitude e latitude para o programa funcionar.
    um exemplo está em instances/example_km.txt
*/

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cmath>
#include <iomanip>

constexpr double EARTH_RADIUS = 6371.0;
constexpr double PI = 3.14159265358979323846;

struct Location {
    std::string id;
    char type;
    double x, y;
    double longitude, latitude;
};

double toRadians(double degrees) {
    return degrees * PI / 180.0;
}

double toDegrees(double radians) {
    return radians * 180.0 / PI;
}

void convertToGeo(
    Location& loc,
    double depotLon,
    double depotLat
) {
    double lat0 = toRadians(depotLat);
    double lon0 = toRadians(depotLon);

    double lat = lat0 + loc.y / EARTH_RADIUS;
    double lon = lon0 + loc.x /
        (EARTH_RADIUS * std::cos(lat0));

    loc.latitude = toDegrees(lat);
    loc.longitude = toDegrees(lon);
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0]
                  << " input.txt output.txt\n";
        return 1;
    }

    // Original depot geographic coordinates
    constexpr double DEPOT_LON = -77.49439265;
    constexpr double DEPOT_LAT = 37.60851245;

    std::ifstream input(argv[1]);

    if (!input) {
        std::cerr << "Could not open input file\n";
        return 1;
    }

    std::ofstream output(argv[2]);

    if (!output) {
        std::cerr << "Could not create output file\n";
        return 1;
    }

    std::vector<Location> locations;
    std::string line;

    while (std::getline(input, line)) {
        // Skip empty lines and the header
        if (line.empty() || line.find("ID") == 0)
            continue;

        std::istringstream iss(line);

        Location loc;

        if (!(iss >> loc.id >> loc.type >> loc.x >> loc.y))
            continue;

        convertToGeo(loc, DEPOT_LON, DEPOT_LAT);

        locations.push_back(loc);
    }

    // Write output
    output << "ID\tType\tLongitude\tLatitude\n";

    output << std::fixed << std::setprecision(8);

    for (const auto& loc : locations) {
        output << loc.id << '\t'
               << loc.type << '\t'
               << loc.longitude << '\t'
               << loc.latitude << '\n';
    }

    // Preserve VRP parameters
    output << "\n\n"
           << "Q Vehicle fuel tank capacity /60/\n"
           << "r fuel consumption rate /0.2/\n"
           << "TL TourLength /11/\n"
           << "v average Velocity /40/\n"
           << "m numVeh /3/\n";

    std::cout << "Successfully converted "
              << locations.size() << " locations.\n";
}