#pragma once

#include <vector>
#include "std/string.h"

class ResourcePackManager;
class SkinRepository;

class I18n {

public:
    static mcpe::string get(mcpe::string const&, std::vector<mcpe::string> const&);
    static void chooseLanguage(mcpe::string const&);
    static void loadLanguages(ResourcePackManager&, SkinRepository*, mcpe::string const&);

};