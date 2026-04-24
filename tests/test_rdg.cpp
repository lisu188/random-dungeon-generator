#include "rdg.h"

#include <algorithm>
#include <cstdlib>
#include <random>
#include <string>

static void require(bool condition) {
    if (!condition) {
        std::abort();
    }
}

static int dungeon_checksum(const rdg::Dungeon &dungeon) {
    int checksum = 0;
    for (const auto &cell: dungeon.getCells()) {
        if (cell.isOpenspace()) {
            checksum += 1;
        }
        if (cell.hasLabel()) {
            checksum += static_cast<unsigned char>(cell.getLabel().front());
        }
    }
    checksum += static_cast<int>(dungeon.getRooms().size()) * 17;
    checksum += static_cast<int>(dungeon.getStairs().size()) * 31;
    return checksum;
}

static void test_cell_type_management() {
    rdg::Cell cell;

    cell.setType(rdg::CellType::ROOM);
    require(cell.hasType(rdg::CellType::ROOM));
    require(!cell.hasType(rdg::CellType::CORRIDOR));
    require(cell.isBlockedRoom());
    require(cell.isOpenspace());

    cell.addType(rdg::CellType::ENTRANCE);
    require(cell.isEspace());

    cell.addType(rdg::CellType::LOCKED);
    require(cell.isDoorspace());
    require(cell.isBlockedDoor());

    cell.clearEspace();
    require(!cell.hasType(rdg::CellType::ENTRANCE));
    require(!cell.hasType(rdg::CellType::LOCKED));

    cell.clearTypes();
    require(!cell.hasType(rdg::CellType::ROOM));
}

static void test_cell_labels_and_stairs() {
    rdg::Cell cell;

    require(!cell.hasLabel());
    cell.setLabel('A');
    require(cell.hasLabel());
    require(cell.getLabel() == "A");

    cell.clearLabel();
    require(!cell.hasLabel());

    cell.addType(rdg::CellType::STAIR_UP);
    require(cell.isStairs());
    cell.removeType(rdg::CellType::STAIR_UP);
    cell.addType(rdg::CellType::STAIR_DN);
    require(cell.isStairs());
}

static void test_generate_default_dungeon() {
    rdg::Options options;
    options.n_rows = 21;
    options.n_cols = 21;
    options.add_stairs = 2;
    options.remove_deadends = 50;

    auto dungeon = rdg::create_dungeon(options);

    const auto &cells = dungeon.getCells();
    require(!cells.empty());
    require(static_cast<int>(cells.size()) == options.n_rows * options.n_cols);
    require(dungeon.rowCount() == options.n_rows);
    require(dungeon.colCount() == options.n_cols);

    bool has_open_space = false;
    bool has_room_label = false;
    for (const auto &cell : cells) {
        if (cell.isOpenspace()) {
            has_open_space = true;
        }
        if (cell.hasLabel()) {
            has_room_label = true;
        }
    }

    require(has_open_space);
    require(has_room_label);
    require(!dungeon.getRooms().empty());
}

static void test_layout_variants() {
    rdg::Options options;
    options.n_rows = 25;
    options.n_cols = 25;
    options.dungeon_layout = rdg::DungeonLayout::Cross;
    options.room_layout = rdg::RoomLayout::Packed;
    options.corridor_layout = rdg::CorridorLayout::STRAIGHT;
    options.add_stairs = 0;
    options.remove_deadends = 100;

    auto dungeon = rdg::create_dungeon(options);

    require(!dungeon.getRooms().empty());

    const auto &cells = dungeon.getCells();
    int open_cells = 0;
    for (const auto &cell : cells) {
        if (cell.isOpenspace() || cell.hasLabel()) {
            open_cells++;
        }
    }

    require(open_cells > 0);
}

static void test_option_helpers() {
    require(rdg::dungeon_layout_from_string("Cross") == rdg::DungeonLayout::Cross);
    require(rdg::room_layout_from_string("Packed") == rdg::RoomLayout::Packed);
    require(rdg::map_style_from_string("Standard") == rdg::MapStyle::Standard);
    require(rdg::to_string(rdg::DungeonLayout::Round) == "Round");

    rdg::Options options;
    options.n_rows = 20;
    require(rdg::validate_options(options).has_value());
}

static void test_compact_metadata_helpers() {
    rdg::Door door;
    door.kind = rdg::DoorKind::Secret;
    require(door.getKey() == "secret");
    require(door.getType() == "Secret Door");

    rdg::Stairs stairs;
    stairs.kind = rdg::StairKind::Up;
    require(stairs.getKey() == "up");
}

static void test_seeded_rng_is_deterministic() {
    rdg::Options options;
    options.n_rows = 21;
    options.n_cols = 21;
    options.add_stairs = 2;
    options.remove_deadends = 50;

    std::mt19937 first_rng{123};
    std::mt19937 second_rng{123};

    auto first = rdg::create_dungeon(options, first_rng);
    auto second = rdg::create_dungeon(options, second_rng);

    require(dungeon_checksum(first) == dungeon_checksum(second));
    require(first.getStairs().size() == 2);
    require(!first.getStairs().front().getKey().empty());
}

int main() {
    test_cell_type_management();
    test_cell_labels_and_stairs();
    test_generate_default_dungeon();
    test_layout_variants();
    test_option_helpers();
    test_compact_metadata_helpers();
    test_seeded_rng_is_deterministic();
    return 0;
}
