#pragma once

#include <vector>
#include "string.h"

class ResourcePackManager;
class SkinRepository;

class I18n {

public:

    static mcpe::string (*I18n_get)(mcpe::string const&, std::vector<mcpe::string> const&);
    static void (*I18n_chooseLanguage)(mcpe::string const&);
    static void (*I18n_loadLanguages)(ResourcePackManager&, SkinRepository*, mcpe::string const&);

    static void loadLanguages(ResourcePackManager& m, SkinRepository* r, mcpe::string const& s) {
        I18n_loadLanguages(m, r, s);
    }

};