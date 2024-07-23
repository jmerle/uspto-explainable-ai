#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include <ankerl/unordered_dense.h>

#include <uspto/config.h>
#include <uspto/files.h>
#include <uspto/queries.h>

class PatentReader : public DataReader<std::uint8_t> {
public:
    using DataReader::DataReader;

    PatentReader() : PatentReader(getReformattedPatentDataDirectory()) {}

    std::vector<std::string> readTerms(const std::string& publicationNumber, TermCategory::TermCategory categories) {
        seekToKey(publicationNumber);

        auto cpcSize = readScalar<std::uint32_t>();
        auto titleSize = readScalar<std::uint32_t>();
        auto abstractSize = readScalar<std::uint32_t>();
        auto claimsSize = readScalar<std::uint32_t>();

        auto cpcOffset = getPosition();
        auto titleOffset = cpcOffset + cpcSize;
        auto abstractOffset = titleOffset + titleSize;
        auto claimsOffset = abstractOffset + abstractSize;
        auto descriptionOffset = claimsOffset + claimsSize;

        std::vector<std::string> out;

        if ((categories & TermCategory::Cpc) != 0) {
            readKeywordTokens(out, cpcOffset, TermCategory::Cpc);
        }

        if ((categories & TermCategory::Title) != 0) {
            readTextTokens(out, titleOffset, TermCategory::Title);
        }

        if ((categories & TermCategory::Abstract) != 0) {
            readTextTokens(out, abstractOffset, TermCategory::Abstract);
        }

        if ((categories & TermCategory::Claims) != 0) {
            readTextTokens(out, claimsOffset, TermCategory::Claims);
        }

        if ((categories & TermCategory::Description) != 0) {
            readTextTokens(out, descriptionOffset, TermCategory::Description);
        }

        return out;
    }

    ankerl::unordered_dense::map<std::string, std::uint16_t> readTermsWithCounts(
        const std::string& publicationNumber,
        TermCategory::TermCategory categories) {
        seekToKey(publicationNumber);

        auto cpcSize = readScalar<std::uint32_t>();
        auto titleSize = readScalar<std::uint32_t>();
        auto abstractSize = readScalar<std::uint32_t>();
        auto claimsSize = readScalar<std::uint32_t>();

        auto cpcOffset = getPosition();
        auto titleOffset = cpcOffset + cpcSize;
        auto abstractOffset = titleOffset + titleSize;
        auto claimsOffset = abstractOffset + abstractSize;
        auto descriptionOffset = claimsOffset + claimsSize;

        ankerl::unordered_dense::map<std::string, std::uint16_t> out;

        if ((categories & TermCategory::Cpc) != 0) {
            readKeywordTokensWithCounts(out, cpcOffset, TermCategory::Cpc);
        }

        if ((categories & TermCategory::Title) != 0) {
            readTextTokensWithCounts(out, titleOffset, TermCategory::Title);
        }

        if ((categories & TermCategory::Abstract) != 0) {
            readTextTokensWithCounts(out, abstractOffset, TermCategory::Abstract);
        }

        if ((categories & TermCategory::Claims) != 0) {
            readTextTokensWithCounts(out, claimsOffset, TermCategory::Claims);
        }

        if ((categories & TermCategory::Description) != 0) {
            readTextTokensWithCounts(out, descriptionOffset, TermCategory::Description);
        }

        return out;
    }

private:
    void readKeywordTokens(std::vector<std::string>& out, std::uint64_t offset, TermCategory::TermCategory category) {
        auto prefix = TermCategory::toString(category) + ":";
        seek(offset);

        auto noTokens = readScalar<std::uint16_t>();
        out.reserve(out.size() + noTokens);

        ankerl::unordered_dense::set<std::string> wildcardTokens;

        for (std::uint16_t i = 0; i < noTokens; ++i) {
            auto term = prefix + readString<std::uint8_t>();
            out.emplace_back(term);

            auto slashIndex = term.find('/');
            if (slashIndex != std::string::npos) {
                wildcardTokens.insert(term.substr(0, slashIndex) + "/*");
            }
        }

        out.insert(out.end(), wildcardTokens.begin(), wildcardTokens.end());
    }

    void readTextTokens(std::vector<std::string>& out, std::uint64_t offset, TermCategory::TermCategory category) {
        auto prefix = TermCategory::toString(category) + ":";
        seek(offset);

        auto noTokens = readScalar<std::uint32_t>();
        out.reserve(out.size() + noTokens);

        for (std::uint32_t i = 0; i < noTokens; ++i) {
            out.emplace_back(prefix + readString<std::uint16_t>());
            readScalar<std::uint16_t>();
        }
    }

    void readKeywordTokensWithCounts(
        ankerl::unordered_dense::map<std::string, std::uint16_t>& out,
        std::uint64_t offset,
        TermCategory::TermCategory category) {
        auto prefix = TermCategory::toString(category) + ":";
        seek(offset);

        auto noTokens = readScalar<std::uint16_t>();
        out.reserve(out.size() + noTokens);

        for (std::uint16_t i = 0; i < noTokens; ++i) {
            auto term = prefix + readString<std::uint8_t>();
            out.emplace(term, 1);

            auto slashIndex = term.find('/');
            if (slashIndex != std::string::npos) {
                ++out[term.substr(0, slashIndex) + "/*"];
            }
        }
    }

    void readTextTokensWithCounts(
        ankerl::unordered_dense::map<std::string, std::uint16_t>& out,
        std::uint64_t offset,
        TermCategory::TermCategory category) {
        auto prefix = TermCategory::toString(category) + ":";
        seek(offset);

        auto noTokens = readScalar<std::uint32_t>();
        out.reserve(out.size() + noTokens);

        for (std::uint32_t i = 0; i < noTokens; ++i) {
            auto token = readString<std::uint16_t>();
            auto count = readScalar<std::uint16_t>();

            out.emplace(prefix + token, count);
        }
    }
};

class PatentWriter : public DataWriter<std::uint8_t> {
    std::array<char, 65536> tokenBlock{};

public:
    PatentWriter() : PatentWriter(getReformattedPatentDataDirectory()) {}

    explicit PatentWriter(const std::filesystem::path& directory)
        : DataWriter(directory) {}

    void writePatent(
        const std::string& publicationNumber,
        const std::vector<std::string>& cpcCodes,
        const std::string& title,
        const std::string& abstract,
        const std::string& claims,
        const std::string& description) {
        auto titleTokens = compressTextTokens(extractTextTokens(title));
        auto abstractTokens = compressTextTokens(extractTextTokens(abstract));
        auto claimsTokens = compressTextTokens(extractTextTokens(claims));
        auto descriptionTokens = compressTextTokens(extractTextTokens(description));

        auto cpcSize = getKeywordChunkSize(cpcCodes);
        auto titleSize = getTextChunkSize(titleTokens);
        auto abstractSize = getTextChunkSize(abstractTokens);
        auto claimsSize = getTextChunkSize(claimsTokens);

        addKey(publicationNumber);

        writeScalar<std::uint32_t>(cpcSize);
        writeScalar<std::uint32_t>(titleSize);
        writeScalar<std::uint32_t>(abstractSize);
        writeScalar<std::uint32_t>(claimsSize);

        writeKeywordTokens(cpcCodes);
        writeTextTokens(titleTokens);
        writeTextTokens(abstractTokens);
        writeTextTokens(claimsTokens);
        writeTextTokens(descriptionTokens);
    }

    // Only used internally, but publicly exposed for unit tests
    std::vector<std::string> extractTextTokens(const std::string& text) {
        std::vector<std::string> tokens;

        std::size_t tokenLength = 0;

        std::size_t textLength = text.length();
        auto* textChars = text.data();

        for (std::size_t i = 0; i < textLength; ++i) {
            char ch = textChars[i];

            if (!isWordCharacter(ch)
                && !(tokenLength > 0 && ch == '.' && i < textLength - 1 && isWordCharacter(textChars[i + 1]))) {
                if (tokenLength > 1) {
                    std::string token(tokenBlock.data(), tokenBlock.data() + tokenLength);

                    if (!isForbiddenToken(token)) {
                        tokens.emplace_back(std::move(token));
                    }
                }

                tokenLength = 0;
            } else {
                tokenBlock[tokenLength++] = toLower(ch);
            }
        }

        if (tokenLength > 1) {
            std::string token(tokenBlock.data(), tokenBlock.data() + tokenLength);

            if (!isForbiddenToken(token)) {
                tokens.emplace_back(std::move(token));
            }
        }

        return tokens;
    }

private:
    bool isWordCharacter(char ch) const {
        return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9') || ch == '_';
    }

    char toLower(char ch) const {
        if (ch >= 'A' && ch <= 'Z') {
            return static_cast<char>(ch + 32);
        }

        return ch;
    }

    bool isForbiddenToken(const std::string& token) const {
        bool hasNonDecimal = false;
        int periodCount = 0;

        for (char ch : token) {
            if (ch == '.') {
                ++periodCount;
            } else if (ch < '0' || ch > '9') {
                hasNonDecimal = true;
                break;
            }
        }

        if (!hasNonDecimal && periodCount < 2) {
            return true;
        }

        auto tokenLength = token.length();
        return (tokenLength == 2 && (token == "an" || token == "by" || token == "if"
                                     || token == "is" || token == "no" || token == "of"
                                     || token == "on" || token == "to"))
               || (tokenLength == 3 && (token == "are" || token == "for" || token == "not"
                                        || token == "the" || token == "was"))
               || (tokenLength == 4 && (token == "into" || token == "such" || token == "that"
                                        || token == "then" || token == "they" || token == "this"
                                        || token == "will"))
               || (tokenLength == 5 && (token == "their" || token == "there" || token == "these"));
    }

    ankerl::unordered_dense::map<std::string, std::uint16_t> compressTextTokens(
        const std::vector<std::string>& tokens) const {
        ankerl::unordered_dense::map<std::string, std::uint16_t> count;

        for (const auto& token : tokens) {
            ++count[token];
        }

        return count;
    }

    std::uint32_t getKeywordChunkSize(const std::vector<std::string>& tokens) const {
        // Number of tokens (uint16)
        std::uint32_t size = 2;

        for (const auto& token : tokens) {
            // Length of token (uint8) + token (byte per character)
            size += 1 + token.size();
        }

        return size;
    }

    std::uint32_t getTextChunkSize(const ankerl::unordered_dense::map<std::string, std::uint16_t>& tokens) const {
        // Number of tokens (uint32)
        std::uint32_t size = 4;

        for (const auto& pair : tokens) {
            // Length of token (uint16) + token (byte per character) + token count (uint16)
            size += 4 + pair.first.size();
        }

        return size;
    }

    void writeKeywordTokens(const std::vector<std::string>& tokens) {
        writeScalar<std::uint16_t>(tokens.size());

        for (const auto& token : tokens) {
            writeString<std::uint8_t>(token);
        }
    }

    void writeTextTokens(const ankerl::unordered_dense::map<std::string, std::uint16_t>& tokens) {
        writeScalar<std::uint32_t>(tokens.size());

        for (const auto& [token, count] : tokens) {
            writeString<std::uint16_t>(token);
            writeScalar<std::uint16_t>(count);
        }
    }
};
