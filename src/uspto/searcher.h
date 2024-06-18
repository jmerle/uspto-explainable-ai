#pragma once

#include <algorithm>
#include <any>
#include <cmath>
#include <cstdint>
#include <string>
#include <vector>

#include <ankerl/unordered_dense.h>
#include <antlr4-runtime.h>
#include <roaring/roaring.hh>

#include <uspto/index.h>
#include <uspto/whoosh/WhooshBaseListener.h>
#include <uspto/whoosh/WhooshLexer.h>
#include <uspto/whoosh/WhooshParser.h>
#include <uspto/whoosh/WhooshVisitor.h>

class Searcher {
    const SearchIndex& searchIndex;

    std::vector<std::string> patentIdsReversed;

    struct MatchCollector : whoosh::WhooshVisitor {
        const Searcher& searcher;

        explicit MatchCollector(const Searcher& searcher)
            : searcher(searcher) {}

        std::any visitTerm(whoosh::WhooshParser::TermContext* ctx) override {
            return searcher.searchIndex.termBitsets.at(ctx->CATEGORY()->toString() + ":" + ctx->TOKEN()->toString());
        }

        std::any visitTermExpr(whoosh::WhooshParser::TermExprContext* ctx) override {
            return visit(ctx->term());
        }

        std::any visitWrappedExpr(whoosh::WhooshParser::WrappedExprContext* ctx) override {
            return visit(ctx->expr());
        }

        std::any visitOrExpr(whoosh::WhooshParser::OrExprContext* ctx) override {
            return std::any_cast<roaring::Roaring>(visit(ctx->left))
                   | std::any_cast<roaring::Roaring>(visit(ctx->right));
        }

        std::any visitAndExpr(whoosh::WhooshParser::AndExprContext* ctx) override {
            return std::any_cast<roaring::Roaring>(visit(ctx->left))
                   & std::any_cast<roaring::Roaring>(visit(ctx->right));
        }

        std::any visitXorExpr(whoosh::WhooshParser::XorExprContext* ctx) override {
            return std::any_cast<roaring::Roaring>(visit(ctx->left))
                   ^ std::any_cast<roaring::Roaring>(visit(ctx->right));
        }

        std::any visitNotExpr(whoosh::WhooshParser::NotExprContext* ctx) override {
            auto bits = std::any_cast<roaring::Roaring>(visit(ctx->right));
            bits.flip(0, searcher.searchIndex.ids.size());
            return bits;
        }
    };

    struct TermCollector : whoosh::WhooshBaseListener {
        const Searcher& searcher;

        std::vector<std::pair<std::string, double>> terms;

        explicit TermCollector(const Searcher& searcher)
            : searcher(searcher) {}

        void enterTerm(whoosh::WhooshParser::TermContext* ctx) override {
            auto term = ctx->CATEGORY()->toString() + ":" + ctx->TOKEN()->toString();

            double patentCount = searcher.searchIndex.ids.size();
            double termFrequency = searcher.searchIndex.termBitsets.at(term).cardinality();
            double idf = std::log(patentCount / (termFrequency + 1)) + 1;

            terms.emplace_back(term, idf);
        }
    };

public:
    explicit Searcher(const SearchIndex& searchIndex)
        : searchIndex(searchIndex) {
        patentIdsReversed.resize(searchIndex.ids.size());
        for (const auto& [publicationNumber, id] : searchIndex.ids) {
            patentIdsReversed[id] = publicationNumber;
        }
    }

    ankerl::unordered_dense::set<std::string> search(const std::string& query) const {
        antlr4::ANTLRInputStream input(query);
        whoosh::WhooshLexer lexer(&input);
        antlr4::CommonTokenStream tokens(&lexer);
        whoosh::WhooshParser parser(&tokens);

        antlr4::tree::ParseTree* tree = parser.expr();

        MatchCollector matchCollector(*this);
        auto bits = std::any_cast<roaring::Roaring>(matchCollector.visit(tree));

        std::vector<std::uint32_t> matchingPatentIds;
        matchingPatentIds.reserve(bits.cardinality());
        for (auto id : bits) {
            matchingPatentIds.emplace_back(id);
        }

        if (matchingPatentIds.size() <= 50) {
            return idsToPublicationNumbers(matchingPatentIds);
        }

        TermCollector termCollector(*this);
        antlr4::tree::ParseTreeWalker::DEFAULT.walk(&termCollector, tree);

        ankerl::unordered_dense::map<std::uint32_t, double> tfIdf;
        tfIdf.reserve(matchingPatentIds.size());

        for (const auto& id : matchingPatentIds) {
            double score = 0;

            for (const auto& [term, idf] : termCollector.terms) {
                const auto& counts = searchIndex.termCounts.at(term);

                auto countsIt = counts.find(id);
                if (countsIt != counts.end()) {
                    score += static_cast<double>(countsIt->second) * idf;
                }
            }

            tfIdf.emplace(id, score);
        }

        std::sort(
            matchingPatentIds.begin(),
            matchingPatentIds.end(),
            [&](std::uint32_t a, std::uint32_t b) {
                auto tfIdfA = tfIdf[a];
                auto tfIdfB = tfIdf[b];

                if (tfIdfA == tfIdfB) {
                    return patentIdsReversed[a] < patentIdsReversed[b];
                }

                return tfIdfA > tfIdfB;
            });

        return idsToPublicationNumbers(matchingPatentIds);
    }

private:
    ankerl::unordered_dense::set<std::string> idsToPublicationNumbers(const std::vector<std::uint32_t>& ids) const {
        std::size_t size = std::min(static_cast<std::size_t>(50), ids.size());

        ankerl::unordered_dense::set<std::string> out;
        out.reserve(size);
        for (std::size_t i = 0; i < size; ++i) {
            out.emplace(patentIdsReversed[ids[i]]);
        }

        return out;
    }
};
