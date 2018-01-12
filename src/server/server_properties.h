#pragma once

#include <string>
#include <istream>
#include <map>

class ServerProperties {

private:
    std::map<std::string, std::string> properties;

public:
    void load(std::istream& fstream);

    std::string getString(std::string const& name, std::string const& def = "") {
        return properties.count(name) > 0 ? properties.at(name) : def;
    }

    int getInt(std::string const& name, int def = 0) {
        return properties.count(name) > 0 ? std::stoi(properties.at(name)) : def;
    }

    bool getBool(std::string const& name, bool def = false) {
        if (properties.count(name) > 0) {
            std::string const& val = properties.at(name);
            return (val == "true" || val == "yes" || val == "1");
        }
        return def;
    }

};