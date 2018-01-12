#include "server_properties.h"

void ServerProperties::load(std::istream& stream) {
    std::string line;
    while (std::getline(stream, line)) {
        if (line.length() > 0 && line[0] == '#')
            continue;
        size_t i = line.find('=');
        if (i == std::string::npos)
            continue;
        properties[line.substr(0, i)] = line.substr(i + 1);
    }
}