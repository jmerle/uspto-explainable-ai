#pragma once

#include <string>
#include <vector>

#include <fmt/format.h>

namespace TermCategory {
enum TermCategory {
    Cpc = 1,
    Title = 2,
    Abstract = 4,
    Claims = 8,
    Description = 16,
};

inline TermCategory operator|(TermCategory a, TermCategory b) {
    return static_cast<TermCategory>(static_cast<int>(a) | static_cast<int>(b));
}

inline std::string toString(TermCategory categories) {
    switch (categories) {
        case Cpc:
            return "cpc";
        case Title:
            return "ti";
        case Abstract:
            return "ab";
        case Claims:
            return "clm";
        case Description:
            return "detd";
        default:
            std::vector<std::string> matches;

            if ((categories & Cpc) != 0) matches.emplace_back("cpc");
            if ((categories & Title) != 0) matches.emplace_back("ti");
            if ((categories & Abstract) != 0) matches.emplace_back("ab");
            if ((categories & Claims) != 0) matches.emplace_back("clm");
            if ((categories & Description) != 0) matches.emplace_back("detd");

            return fmt::format("[{}]", fmt::join(matches, ", "));
    }
}
}

struct Term {
    TermCategory::TermCategory category;
    std::string token;

    Term(const char* term) : Term(std::string(term)) {}

    Term(const std::string& term) {
        switch (term[0]) {
            case 'c':
                if (term[1] == 'p') {
                    category = TermCategory::Cpc;
                    token = term.substr(4);
                } else {
                    category = TermCategory::Claims;
                    token = term.substr(4);
                }
                break;
            case 't':
                category = TermCategory::Title;
                token = term.substr(3);
                break;
            case 'a':
                category = TermCategory::Abstract;
                token = term.substr(3);
                break;
            case 'd':
                category = TermCategory::Description;
                token = term.substr(5);
                break;
            default:
                __builtin_unreachable();
        }
    }

    Term(TermCategory::TermCategory category, const std::string& token)
        : category(category),
          token(token) {}
};
