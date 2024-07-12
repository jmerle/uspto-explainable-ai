#include <string>

#include <ankerl/unordered_dense.h>
#include <spdlog/spdlog.h>

#include <uspto/config.h>
#include <uspto/index.h>
#include <uspto/patents.h>

int main() {
    spdlog::info("Creating patent reader");
    PatentReader patentReader;

    spdlog::info("Copying publication numbers from patent index");
    auto index = patentReader.getIndex();

    ankerl::unordered_dense::set<std::string> publicationNumbers;
    publicationNumbers.reserve(index->size());
    for (const auto& [publicationNumber, _] : *index) {
        publicationNumbers.emplace(publicationNumber);
    }

    createSearchIndex(publicationNumbers, getFullIndexDirectory(), patentReader, false);
    return 0;
}
