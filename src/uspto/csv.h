#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include <csv.h>

template<typename F>
void readNeighbors(const std::filesystem::path& file, F&& consumer, int maxRows = -1) {
    io::CSVReader<51> in(file);

    std::string publicationNumber;
    std::vector<std::string> neighbors(50);

    bool isHeader = true;

    while (in.read_row(
        publicationNumber,
        neighbors[0], neighbors[1], neighbors[2], neighbors[3], neighbors[4], neighbors[5], neighbors[6], neighbors[7],
        neighbors[8], neighbors[9], neighbors[10], neighbors[11], neighbors[12], neighbors[13], neighbors[14],
        neighbors[15], neighbors[16], neighbors[17], neighbors[18], neighbors[19], neighbors[20], neighbors[21],
        neighbors[22], neighbors[23], neighbors[24], neighbors[25], neighbors[26], neighbors[27], neighbors[28],
        neighbors[29], neighbors[30], neighbors[31], neighbors[32], neighbors[33], neighbors[34], neighbors[35],
        neighbors[36], neighbors[37], neighbors[38], neighbors[39], neighbors[40], neighbors[41], neighbors[42],
        neighbors[43], neighbors[44], neighbors[45], neighbors[46], neighbors[47], neighbors[48], neighbors[49])) {
        if (isHeader) {
            isHeader = false;
            continue;
        }

        consumer(publicationNumber, neighbors);

        if (maxRows == -1) {
            continue;
        }

        --maxRows;
        if (maxRows <= 0) {
            return;
        }
    }
}
