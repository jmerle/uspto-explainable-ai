#pragma once

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

#include <spdlog/spdlog.h>

#include <uspto/config.h>
#include <uspto/generators.h>

#ifdef GRAFANA_ENABLED

#include <pqxx/pqxx>

class GrafanaReporter {
    pqxx::connection connection;

public:
    explicit GrafanaReporter(bool initDatabase = false)
        : connection("postgres://postgres:postgres@localhost:5432/postgres") {
        if (initDatabase) {
            pqxx::work work(connection);

            work.exec0(
                R"(
CREATE TABLE IF NOT EXISTS properties (
    key TEXT PRIMARY KEY,
    value INTEGER NOT NULL
)
)");
            work.exec0(
                R"(
CREATE TABLE IF NOT EXISTS generators (
    task INTEGER NOT NULL,
    generator TEXT NOT NULL,
    score DOUBLE PRECISION,
    seconds DOUBLE PRECISION,
    PRIMARY KEY (task, generator)
)
)");
            work.exec0(
                R"(
CREATE TABLE IF NOT EXISTS tasks (
    id INTEGER PRIMARY KEY,
    generator TEXT,
    score DOUBLE PRECISION,
    seconds DOUBLE PRECISION
)
)");

            work.exec0("TRUNCATE properties");
            work.exec0("TRUNCATE generators");
            work.exec0("TRUNCATE tasks");

            work.commit();
        }

        connection.prepare("insert_property", "INSERT INTO properties VALUES ($1, $2)");
        connection.prepare("insert_generator", "INSERT INTO generators VALUES ($1, $2, $3, $4)");
        connection.prepare("insert_task", "INSERT INTO tasks VALUES ($1, $2, $3, $4)");

        connection.prepare(
            "update_generator",
            "UPDATE generators SET score = $3, seconds = $4 WHERE task = $1 AND generator = $2");
        connection.prepare("update_task", "UPDATE tasks SET generator = $2, score = $3, seconds = $4 WHERE id = $1");
    }

    void init(
        std::size_t threadCount,
        std::size_t taskCount,
        const std::vector<std::unique_ptr<QueryGenerator>>& generators) {
        try {
            pqxx::work work(connection);

            work.exec_prepared("insert_property", "threadCount", threadCount);

            for (std::size_t i = 0; i < taskCount; ++i) {
                work.exec_prepared("insert_task", i, nullptr, nullptr, nullptr);

                for (const auto& generator : generators) {
                    work.exec_prepared("insert_generator", i, generator->getName(), nullptr, nullptr);
                }
            }

            work.commit();
        } catch (const pqxx::sql_error& exception) {
            spdlog::warn(R"(init({}, #{} generators): {})", taskCount, generators.size(), exception.what());
        }
    }

    void reportGenerator(std::size_t task, const std::string& generator, double score, double seconds) {
        try {
            pqxx::work work(connection);
            work.exec_prepared("update_generator", task, generator, score, seconds);
            work.commit();
        } catch (const pqxx::sql_error& exception) {
            spdlog::warn(R"(reportGenerator({}, "{}", {}, {}): {})", task, generator, score, seconds, exception.what());
        }
    }

    void reportTask(std::size_t task, const std::string& generator, double score, double seconds) {
        try {
            pqxx::work work(connection);
            work.exec_prepared("update_task", task, generator, score, seconds);
            work.commit();
        } catch (const pqxx::sql_error& exception) {
            spdlog::warn(R"(reportTask({}, "{}", {}, {}): {})", task, generator, score, seconds, exception.what());
        }
    }
};

#else

class GrafanaReporter {
public:
    explicit GrafanaReporter(bool initDatabase = false) {}

    void init(
        std::size_t threadCount,
        std::size_t taskCount,
        const std::vector<std::unique_ptr<QueryGenerator>>& generators) {}

    void reportGenerator(std::size_t task, const std::string& generator, double score, double seconds) {}

    void reportTask(std::size_t task, const std::string& generator, double score, double seconds) {}
};

#endif
