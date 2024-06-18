#include <fstream>

#include <spdlog/spdlog.h>

#include <uspto/config.h>
#include <uspto/submission.h>

int main() {
    auto tasks = generateQueries(getCompetitionDataDirectory() / "test.csv");

    auto submissionFile = getOutputDirectory() / "submission.csv";
    spdlog::info("Writing best queries to {}", submissionFile.c_str());

    std::ofstream out(submissionFile);

    out << "publication_number,query\n";
    for (const auto& task : tasks) {
        out << task.publicationNumber << ',' << task.bestQuery << '\n';
    }

    out.flush();
    return 0;
}
