#include "rdg.h"

int main() {
    auto dungeon = rdg<>::create_dungeon(rdg<>::Options());
    for (const auto &row:dungeon.getCells()) {
        for (auto cell:row) {
            if (cell.hasLabel()) {
                std::cout << cell.getLabel();
            } else if (cell.hasType(rdg<>::ROOM)) {
                std::cout << "X";
            } else if (cell.hasType(rdg<>::CORRIDOR)) {
                std::cout << "x";
            } else if (cell.isDoorspace()) {
                std::cout << "D";
            } else {
                std::cout << " ";
            }
            std::cout << " ";
        }
        std::cout << std::endl;
    }
    return 0;
}
