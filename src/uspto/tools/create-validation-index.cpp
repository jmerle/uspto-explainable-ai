#include <string>

#include <ankerl/unordered_dense.h>
#include <spdlog/spdlog.h>

#include <uspto/config.h>
#include <uspto/csv.h>
#include <uspto/index.h>
#include <uspto/patents.h>

int main() {
    ankerl::unordered_dense::set<std::string> publicationNumbers;

    spdlog::info("Reading neighbors_small.csv");
    readNeighbors(
        getValidationDataDirectory() / "neighbors_small.csv",
        [&](const std::string& publicationNumber, const std::vector<std::string>& neighbors) {
            publicationNumbers.insert(neighbors.begin(), neighbors.end());
        });

    spdlog::info("Creating patent reader");
    PatentReader patentReader;

    createSearchIndex(publicationNumbers, getValidationIndexDirectory(), patentReader, true);
    return 0;
}
