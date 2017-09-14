/*
 * Copyright (C) 2017~2017 by CSSlayer
 * wengxt@gmail.com
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1 of the
 * License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; see the file COPYING. If not,
 * see <http://www.gnu.org/licenses/>.
 */
#include "tableime.h"
#include "tablebaseddictionary.h"
#include <boost/utility/string_view.hpp>
#include <map>

namespace libime {

class TableIMEPrivate {
public:
    TableIMEPrivate(LanguageModelResolver *lmProvider)
        : lmProvider_(std::move(lmProvider)) {}

    LanguageModelResolver *lmProvider_;
    std::map<std::string, std::unique_ptr<TableBasedDictionary>, std::less<>>
        dicts_;
    std::map<std::string, std::unique_ptr<UserLanguageModel>, std::less<>>
        languageModels_;
};

TableIME::TableIME(LanguageModelResolver *lmProvider)
    : d_ptr(std::make_unique<TableIMEPrivate>(lmProvider)) {}

TableIME::~TableIME() { destroy(); }

TableBasedDictionary *TableIME::requestDict(boost::string_view dictName) {
    FCITX_D();
    auto iter = d->dicts_.find(dictName);
    if (iter == d->dicts_.end()) {
        std::unique_ptr<TableBasedDictionary> dict(requestDictImpl(dictName));
        if (!dict) {
            return nullptr;
        }
        iter = d->dicts_.emplace(dictName.to_string(), std::move(dict)).first;
    }

    return iter->second.get();
}

void TableIME::saveDict(TableBasedDictionary *dict) {
    FCITX_D();
    saveDictImpl(dict);
}

UserLanguageModel *
TableIME::languageModelForDictionary(TableBasedDictionary *dict) {
    FCITX_D();
    auto language = dict->tableOptions().languageCode();
    auto iter = d->languageModels_.find(language);
    if (iter == d->languageModels_.end()) {
        auto file = d->lmProvider_->languageModelFileForLanguage(
            dict->tableOptions().languageCode());
        if (!file) {
            return nullptr;
        }

        iter = d->languageModels_
                   .emplace(language, std::make_unique<UserLanguageModel>(file))
                   .first;
    }
    return iter->second.get();
}
}