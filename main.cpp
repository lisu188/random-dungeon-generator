#include "rdg.h"

#include <iostream>

int main() {
    auto dungeon = rdg::create_dungeon(rdg::Options());
    for (int row = 0; row < dungeon.rowCount(); ++row) {
        for (int col = 0; col < dungeon.colCount(); ++col) {
            const auto &cell = dungeon.cellAt(row, col);
            if (cell.hasLabel()) {
                std::cout << cell.getLabel();
            } else if (cell.hasType(rdg::CellType::ROOM)) {
                std::cout << "X";
            } else if (cell.hasType(rdg::CellType::CORRIDOR)) {
                std::cout << "x";
            } else if (cell.isDoorspace()) {
                std::cout << "D";
            } else {
                std::cout << " ";
            }
            std::cout << " ";
        }
        std::cout << '\n';
    }
    return 0;
}
