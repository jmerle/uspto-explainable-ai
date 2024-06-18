#pragma once

#include <algorithm>
#include <cstdlib>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <ios>
#include <memory>
#include <string>
#include <vector>

#include <ankerl/unordered_dense.h>
#include <fmt/format.h>
#include <spdlog/spdlog.h>

class FileReader {
    std::ifstream in;

public:
    explicit FileReader(const std::filesystem::path& file) {
        if (!std::filesystem::exists(file)) {
            spdlog::error("{} does not exist", file.generic_string());
            std::exit(1);
        }

        in = std::ifstream(file);
    }

    std::uint64_t getPosition() {
        return in.tellg();
    }

    void seek(std::uint64_t offset) {
        in.seekg(offset, std::ios::beg);
    }

    bool isEOF() {
        return in.peek() == std::ifstream::traits_type::eof();
    }

    std::vector<char> readRaw(std::uint64_t size) {
        std::vector<char> buffer(size);
        in.read(buffer.data(), size);
        return buffer;
    }

    template<typename T>
    T readScalar() {
        T value;
        in.read(reinterpret_cast<char*>(&value), sizeof(T));
        return value;
    }

    template<typename SizeType>
    std::string readString() {
        auto length = readScalar<SizeType>();

        std::string str(length, '\0');
        in.read(str.data(), length);

        return str;
    }
};

class FileWriter {
    std::ofstream out;

public:
    explicit FileWriter(const std::filesystem::path& path) {
        if (!std::filesystem::exists(path)) {
            std::filesystem::create_directories(path.parent_path());
        }

        out = std::ofstream(path);
    }

    std::uint64_t getPosition() {
        return out.tellp();
    }

    void writeRaw(const char* buffer, std::uint64_t size) {
        out.write(buffer, size);
    }

    template<typename T>
    void writeScalar(T value) {
        out.write(reinterpret_cast<char*>(&value), sizeof(T));
    }

    template<typename SizeType>
    void writeString(const std::string& value) {
        writeScalar<SizeType>(value.length());
        out << value;
    }
};

template<typename KeySizeType>
class DataReader : public FileReader {
    std::filesystem::path directory;
    std::shared_ptr<ankerl::unordered_dense::map<std::string, std::uint64_t>> index;

public:
    explicit DataReader(const std::filesystem::path& directory)
        : FileReader(directory / "data.bin"),
          directory(directory),
          index(std::make_shared<ankerl::unordered_dense::map<std::string, std::uint64_t>>()) {
        FileReader indexReader(directory / "index.bin");

        std::uint64_t offset = 0;
        while (!indexReader.isEOF()) {
            auto key = indexReader.readString<KeySizeType>();
            offset += indexReader.readScalar<std::uint32_t>();

            index->emplace(key, offset);
        }
    }

    DataReader(const DataReader& other)
        : FileReader(other.directory / "data.bin"), directory(other.directory), index(other.index) {}

    void seekToKey(const std::string& key) {
        seek(index->at(key));
    }

    std::shared_ptr<ankerl::unordered_dense::map<std::string, std::uint64_t>> getIndex() const {
        return index;
    }

    void sortToIndex(std::vector<std::string>& keys) const {
        std::sort(
            keys.begin(),
            keys.end(),
            [&](const std::string& a, const std::string& b) {
                return index->at(a) < index->at(b);
            });
    }
};

template<typename KeySizeType>
class DataWriter : public FileWriter {
    FileWriter indexWriter;

    std::uint64_t lastPosition;

public:
    explicit DataWriter(const std::filesystem::path& directory)
        : FileWriter(directory / "data.bin"),
          indexWriter(directory / "index.bin"),
          lastPosition(0) {}

    void addKey(const std::string& key) {
        auto currentPosition = getPosition();
        auto deltaPosition = currentPosition - lastPosition;
        lastPosition = currentPosition;

        indexWriter.writeString<KeySizeType>(key);
        indexWriter.writeScalar<std::uint32_t>(deltaPosition);
    }
};

struct TemporaryDirectory {
    std::filesystem::path path;

    TemporaryDirectory() {
        auto rootDirectory = std::filesystem::temp_directory_path();

        for (int i = 1;; ++i) {
            path = rootDirectory / fmt::format("uspto-{}", i);
            if (!std::filesystem::exists(path)) {
                std::filesystem::create_directories(path);
                break;
            }
        }
    }

    ~TemporaryDirectory() {
        std::filesystem::remove_all(path);
    }
};
