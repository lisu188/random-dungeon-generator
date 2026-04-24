#include "rdg.h"

#include <iostream>
#include <string>

int main() {
    auto dungeon = rdg::create_dungeon(rdg::Options());
    std::string output;
    output.reserve(static_cast<std::size_t>(dungeon.rowCount() * ((dungeon.colCount() * 2) + 1)));

    for (int row = 0; row < dungeon.rowCount(); ++row) {
        for (int col = 0; col < dungeon.colCount(); ++col) {
            const auto &cell = dungeon.cellAt(row, col);
            if (cell.hasLabel()) {
                output.append(cell.getLabel());
            } else if (cell.hasType(rdg::CellType::ROOM)) {
                output.push_back('X');
            } else if (cell.hasType(rdg::CellType::CORRIDOR)) {
                output.push_back('x');
            } else if (cell.isDoorspace()) {
                output.push_back('D');
            } else {
                output.push_back(' ');
            }
            output.push_back(' ');
        }
        output.push_back('\n');
    }
    std::cout << output;
    return 0;
}
