#include "rdg.h"

#include <array>
#include <cstdint>
#include <iostream>
#include <limits>
#include <random>

namespace {
void printDoor(const rdg::Door &door) {
    std::cout << door.row << ' ' << door.col << ' ' << static_cast<unsigned>(door.kind) << ' ' << door.out_id << ' '
              << door.getKey() << ' ' << door.getType() << '\n';
}

void printDungeon(const rdg::Dungeon &dungeon) {
    std::cout << "dimensions " << dungeon.rowCount() << ' ' << dungeon.colCount() << '\n';
    std::cout << "cells " << dungeon.getCells().size() << '\n';
    for (const auto &cell : dungeon.getCells()) {
        unsigned mask = 0;
        for (unsigned index = 0; index <= static_cast<unsigned>(rdg::CellType::STAIR_UP); ++index) {
            if (cell.hasType(static_cast<rdg::CellType>(index))) {
                mask |= 1u << index;
            }
        }
        const auto label = cell.getLabel();
        std::cout << mask << ' ' << cell.getRoomId() << ' '
                  << (label.empty() ? 0u : static_cast<unsigned>(static_cast<unsigned char>(label.front()))) << '\n';
    }
    std::cout << "rooms " << dungeon.getRooms().size() << '\n';
    for (const auto &room : dungeon.getRooms()) {
        std::cout << room.id << ' ' << room.row << ' ' << room.col << ' ' << room.north << ' ' << room.south << ' '
                  << room.west << ' ' << room.east << ' ' << room.height << ' ' << room.width << ' ' << room.area
                  << '\n';
        for (const auto &direction : room.door) {
            std::cout << "room-doors " << direction.size() << '\n';
            for (const auto &door : direction) {
                printDoor(door);
            }
        }
    }
    std::cout << "door-groups " << dungeon.getDoors().size() << '\n';
    for (const auto &group : dungeon.getDoors()) {
        std::cout << "doors " << group.size() << '\n';
        for (const auto &door : group) {
            printDoor(door);
        }
    }
    std::cout << "stairs " << dungeon.getStairs().size() << '\n';
    for (const auto &stairs : dungeon.getStairs()) {
        std::cout << stairs.row << ' ' << stairs.col << ' ' << stairs.next_row << ' ' << stairs.next_col << ' '
                  << static_cast<unsigned>(stairs.kind) << ' ' << stairs.getKey() << '\n';
    }
}
} // namespace

int main() {
    struct Scenario {
        rdg::DungeonLayout layout;
        rdg::RoomLayout rooms;
        rdg::CorridorLayout corridors;
        int deadends;
        int stairs;
    };
    constexpr std::array scenarios{
        Scenario{rdg::DungeonLayout::None, rdg::RoomLayout::Scattered, rdg::CorridorLayout::LABYRINTH, 0, 0},
        Scenario{rdg::DungeonLayout::Box, rdg::RoomLayout::Scattered, rdg::CorridorLayout::BENT, 50, 1},
        Scenario{rdg::DungeonLayout::Cross, rdg::RoomLayout::Packed, rdg::CorridorLayout::STRAIGHT, 100, 2},
        Scenario{rdg::DungeonLayout::Round, rdg::RoomLayout::Packed, rdg::CorridorLayout::LABYRINTH, 100, 4},
    };
    constexpr std::array seeds{0u, 1u, 11u, 12u, 13u, 14u, 123u, std::numeric_limits<unsigned>::max()};
    for (const auto &scenario : scenarios) {
        for (const auto seed : seeds) {
            rdg::Options options;
            options.n_rows = 33;
            options.n_cols = 33;
            options.dungeon_layout = scenario.layout;
            options.room_layout = scenario.rooms;
            options.corridor_layout = scenario.corridors;
            options.remove_deadends = scenario.deadends;
            options.add_stairs = scenario.stairs;
            std::mt19937 rng{seed};
            std::cout << "scenario " << static_cast<unsigned>(scenario.layout) << ' '
                      << static_cast<unsigned>(scenario.rooms) << ' ' << static_cast<int>(scenario.corridors) << ' '
                      << scenario.deadends << ' ' << scenario.stairs << ' ' << seed << '\n';
            printDungeon(rdg::create_dungeon(options, rng));
            std::cout << "rng-state " << rng << '\n';
            std::cout << "subsequent-rng";
            for (int draw = 0; draw < 16; ++draw) {
                std::cout << ' ' << rng();
            }
            std::cout << '\n';
        }
    }
}
