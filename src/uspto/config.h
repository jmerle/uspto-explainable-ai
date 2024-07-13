#pragma once

#include <cstdlib>
#include <filesystem>
#include <string>

#include <spdlog/spdlog.h>

inline bool IS_KAGGLE = std::filesystem::exists("/kaggle");

inline std::string getEnv(const std::string& variable) {
    const char* value = std::getenv(variable.c_str());
    if (value == nullptr) {
        spdlog::error("{} environment variable is not set", variable);
        std::exit(1);
    }

    return value;
}

inline std::filesystem::path getPathFromEnv(const std::string& variable) {
    return std::filesystem::current_path() / getEnv(variable);
}

inline std::filesystem::path getCompetitionDataDirectory() {
    if (IS_KAGGLE) {
        return "/kaggle/input/uspto-explainable-ai";
    }

    return getPathFromEnv("COMPETITION_DATA_DIRECTORY");
}

inline std::filesystem::path getValidationDataDirectory() {
    if (IS_KAGGLE) {
        spdlog::error("getValidationDataDirectory() is not supported when running on Kaggle");
        std::exit(1);
    }

    return getPathFromEnv("VALIDATION_DATA_DIRECTORY");
}

inline std::filesystem::path getProjectDirectory() {
    if (IS_KAGGLE) {
        spdlog::error("getProjectDirectory() is not supported when running on Kaggle");
        std::exit(1);
    }

    return getPathFromEnv("PROJECT_DIRECTORY");
}

inline std::filesystem::path getOutputDirectory() {
    if (IS_KAGGLE) {
        return "/kaggle/working";
    }

    return getPathFromEnv("OUTPUT_DIRECTORY");
}

inline std::filesystem::path getReformattedPatentDataDirectory() {
    if (IS_KAGGLE) {
        return "/kaggle/input/uspto-explainable-ai-reformatted-patent-data";
    }

    return getOutputDirectory() / "patents";
}

inline std::filesystem::path getValidationIndexDirectory() {
    if (IS_KAGGLE) {
        spdlog::error("getValidationIndexDirectory() is not supported when running on Kaggle");
        std::exit(1);
    }

    return getOutputDirectory() / "validation-index";
}

inline std::filesystem::path getFullIndexDirectory() {
    if (IS_KAGGLE) {
        return "/kaggle/input/uspto-explainable-ai-full-search-index";
    }

    return getOutputDirectory() / "full-index";
}

inline bool isGrafanaEnabled() {
    if (IS_KAGGLE) {
        return false;
    }

    return getEnv("GRAFANA_ENABLED") == "true";
}
