#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <future>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <ankerl/unordered_dense.h>
#include <BS_thread_pool.hpp>
#include <duckdb.hpp>
#include <fmt/format.h>
#include <spdlog/spdlog.h>

#include <uspto/config.h>
#include <uspto/patents.h>
#include <uspto/progress.h>

using CpcCodesMap = ankerl::unordered_dense::map<std::string, std::vector<std::string>>;

template<typename... QueryFmtArgs>
std::unique_ptr<duckdb::MaterializedQueryResult> queryDuckDB(
    duckdb::Connection& connection,
    fmt::format_string<QueryFmtArgs...> queryFmtString,
    QueryFmtArgs&&... queryFmtArgs) {
    auto query = fmt::vformat(queryFmtString, fmt::make_format_args(queryFmtArgs...));

    auto result = connection.Query(query);
    if (result->HasError()) {
        spdlog::error("Running '{}' query failed: {}", query, result->GetError());
        std::exit(1);
    }

    return std::move(result);
}

CpcCodesMap getCpcCodesByPatent() {
    spdlog::info("Extracting CPC codes from patent_metadata.parquet");

    duckdb::DuckDB db(nullptr);
    duckdb::Connection connection(db);

    auto metadata = queryDuckDB(
        connection,
        "SELECT publication_number, cpc_codes FROM read_parquet('{}')",
        (getCompetitionDataDirectory() / "patent_metadata.parquet").generic_string());

    auto& collection = metadata->Collection();

    ProgressBar progressBar(metadata->RowCount(), "Processing metadata");

    std::vector<std::future<ankerl::unordered_dense::map<std::string, std::vector<std::string>>>> futures;
    futures.reserve(collection.ChunkCount());

    duckdb::ColumnDataScanState scanState;
    collection.InitializeScan(scanState, duckdb::ColumnDataScanProperties::DISALLOW_ZERO_COPY);

    std::vector<duckdb::DataChunk> chunks(collection.ChunkCount());
    BS::thread_pool threadPool;

    for (auto& chunk : chunks) {
        collection.InitializeScanChunk(scanState, chunk);
        collection.Scan(scanState, chunk);

        futures.emplace_back(
            threadPool.submit_task(
                [&] {
                    CpcCodesMap map;
                    map.reserve(chunk.size());

                    for (std::size_t i = 0; i < chunk.size(); ++i) {
                        auto publicationNumber = chunk.GetValue(0, i).GetValue<std::string>();
                        auto cpcCodes = duckdb::ListValue::GetChildren(chunk.GetValue(1, i));

                        auto& cpcVec = map[publicationNumber];
                        cpcVec.reserve(cpcCodes.size());
                        for (const auto& value : cpcCodes) {
                            cpcVec.emplace_back(value.GetValue<std::string>());
                        }
                    }

                    progressBar.update(chunk.size());
                    return map;
                }));
    }

    CpcCodesMap cpcCodesByPatent;
    cpcCodesByPatent.reserve(metadata->RowCount());

    for (auto& result : futures) {
        const auto& map = result.get();
        cpcCodesByPatent.insert(map.begin(), map.end());
    }

    return cpcCodesByPatent;
}

void processPatentDataFiles(const CpcCodesMap& cpcCodesByPatent) {
    std::vector<std::filesystem::path> files;
    std::copy(
        std::filesystem::directory_iterator(getCompetitionDataDirectory() / "patent_data"),
        std::filesystem::directory_iterator(),
        std::back_inserter(files));
    std::sort(files.begin(), files.end());

    ProgressBar progressBar(cpcCodesByPatent.size(), "Processing patents");

    duckdb::DuckDB db(nullptr);
    duckdb::Connection connection(db);

    PatentWriter writer;

    ankerl::unordered_dense::set<std::string> writtenPublicationNumbers;
    writtenPublicationNumbers.reserve(cpcCodesByPatent.size());

    for (const auto& file : files) {
        progressBar.setDescription(fmt::format("Processing patents: {}", file.filename().generic_string()));

        auto patentData = queryDuckDB(connection, "SELECT * FROM read_parquet('{}')", file.generic_string());
        for (const auto& chunk : patentData->Collection().Chunks()) {
            for (std::size_t i = 0; i < chunk.size(); ++i) {
                auto publicationNumber = chunk.GetValue(0, i).GetValue<std::string>();
                const auto& cpcCodes = cpcCodesByPatent.at(publicationNumber);
                auto title = chunk.GetValue(1, i).GetValue<std::string>();
                auto abstract = chunk.GetValue(2, i).GetValue<std::string>();
                auto claims = chunk.GetValue(3, i).GetValue<std::string>();
                auto description = chunk.GetValue(4, i).GetValue<std::string>();

                writer.writePatent(publicationNumber, cpcCodes, title, abstract, claims, description);
                writtenPublicationNumbers.emplace(publicationNumber);
            }

            progressBar.update(chunk.size());
        }
    }

    progressBar.setDescription("Processing patents without text");

    // For 93 patents there is no row in their corresponding patent data file
    for (const auto& [publicationNumber, cpcCodes] : cpcCodesByPatent) {
        if (!writtenPublicationNumbers.contains(publicationNumber)) {
            writer.writePatent(publicationNumber, cpcCodes, "", "", "", "");
            progressBar.update(1);
        }
    }
}

int main() {
    auto cpcCodesByPatent = getCpcCodesByPatent();
    processPatentDataFiles(cpcCodesByPatent);
    return 0;
}
