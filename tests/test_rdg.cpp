#include "rdg.h"

#include <algorithm>
#include <array>
#include <cstdlib>
#include <random>
#include <stdexcept>
#include <string>
#include <string_view>

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

static rdg::Dungeon create_seeded(rdg::Options options, unsigned seed) {
    std::mt19937 rng{seed};
    return rdg::create_dungeon(options, rng);
}

static int count_cells_with_type(const rdg::Dungeon &dungeon, rdg::CellType type) {
    int count = 0;
    for (const auto &cell: dungeon.getCells()) {
        if (cell.hasType(type)) {
            count++;
        }
    }
    return count;
}

static bool has_label(const rdg::Dungeon &dungeon, std::string_view label) {
    return std::any_of(dungeon.getCells().begin(), dungeon.getCells().end(),
                       [&](const rdg::Cell &cell) { return cell.getLabel() == label; });
}

static void require_valid_dungeon(const rdg::Dungeon &dungeon, const rdg::Options &options) {
    require(static_cast<int>(dungeon.getCells().size()) == options.n_rows * options.n_cols);
    require(dungeon.rowCount() == options.n_rows);
    require(dungeon.colCount() == options.n_cols);
    require(std::all_of(dungeon.getRooms().begin(), dungeon.getRooms().end(), [&](const rdg::Room &room) {
        return room.id > 0 && room.north <= room.south && room.west <= room.east &&
               room.area == room.height * room.width;
    }));
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

    cell.setType(rdg::CellType::BLOCKED);
    require(cell.isBlockedRoom());
    require(cell.isBlockedCorridor());
    require(cell.isBlockedDoor());

    cell.setType(rdg::CellType::PERIMETER);
    require(cell.isBlockedCorridor());

    for (auto type: {rdg::CellType::ARCH, rdg::CellType::DOOR, rdg::CellType::TRAPPED, rdg::CellType::SECRET,
                     rdg::CellType::PORTC}) {
        cell.clearTypes();
        cell.addType(type);
        require(cell.isDoorspace());
        require(cell.isEspace());
    }
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
    for (const auto &cell: cells) {
        if (cell.isOpenspace()) {
            has_open_space = true;
        }
        if (cell.hasLabel()) {
            has_room_label = true;
        }
    }

    require(has_open_space);
    require(has_room_label);
    require(has_label(dungeon, "1"));
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
    for (const auto &cell: cells) {
        if (cell.isOpenspace() || cell.hasLabel()) {
            open_cells++;
        }
    }

    require(open_cells > 0);
}

static void test_generation_option_matrix() {
    struct Scenario {
        rdg::DungeonLayout dungeon_layout;
        rdg::RoomLayout room_layout;
        rdg::CorridorLayout corridor_layout;
        int remove_deadends;
        int add_stairs;
        unsigned seed;
    };

    const std::array scenarios{
            Scenario{rdg::DungeonLayout::None, rdg::RoomLayout::Scattered, rdg::CorridorLayout::LABYRINTH, 0, 0, 11},
            Scenario{rdg::DungeonLayout::Box, rdg::RoomLayout::Scattered, rdg::CorridorLayout::BENT, 50, 1, 12},
            Scenario{rdg::DungeonLayout::Cross, rdg::RoomLayout::Packed, rdg::CorridorLayout::STRAIGHT, 100, 2, 13},
            Scenario{rdg::DungeonLayout::Round, rdg::RoomLayout::Packed, rdg::CorridorLayout::LABYRINTH, 100, 4, 14},
    };

    for (const auto &scenario: scenarios) {
        rdg::Options options;
        options.n_rows = 33;
        options.n_cols = 33;
        options.dungeon_layout = scenario.dungeon_layout;
        options.room_layout = scenario.room_layout;
        options.corridor_layout = scenario.corridor_layout;
        options.remove_deadends = scenario.remove_deadends;
        options.add_stairs = scenario.add_stairs;

        auto dungeon = create_seeded(options, scenario.seed);
        require_valid_dungeon(dungeon, options);
        require(!dungeon.getRooms().empty());
        require(count_cells_with_type(dungeon, rdg::CellType::ROOM) > 0);
        require(static_cast<int>(dungeon.getStairs().size()) <= options.add_stairs);
        if (options.add_stairs > 0) {
            require(!dungeon.getStairs().empty());
        }
    }
}

static void test_generated_door_and_stair_metadata() {
    rdg::Options options;
    options.n_rows = 33;
    options.n_cols = 33;
    options.add_stairs = 2;
    options.remove_deadends = 50;

    for (unsigned seed = 1; seed < 100; ++seed) {
        auto dungeon = create_seeded(options, seed);
        if (dungeon.getDoors().empty() || dungeon.getStairs().size() != 2) {
            continue;
        }

        bool saw_door = false;
        for (const auto &door_group: dungeon.getDoors()) {
            require(!door_group.empty());
            for (const auto &door: door_group) {
                saw_door = true;
                require(door.kind != rdg::DoorKind::None);
                require(!door.getKey().empty());
                require(!door.getType().empty());
                require(dungeon.cellAt(door.row, door.col).isDoorspace());
                require(dungeon.cellAt(door.row, door.col).hasLabel());
            }
        }
        require(saw_door);

        bool saw_down = false;
        bool saw_up = false;
        for (const auto &stairs: dungeon.getStairs()) {
            require(stairs.kind == rdg::StairKind::Down || stairs.kind == rdg::StairKind::Up);
            require(!stairs.getKey().empty());
            require(dungeon.cellAt(stairs.row, stairs.col).isStairs());
            saw_down = saw_down || stairs.kind == rdg::StairKind::Down;
            saw_up = saw_up || stairs.kind == rdg::StairKind::Up;
        }
        require(saw_down);
        require(saw_up);
        return;
    }

    require(false);
}

static void test_option_helpers() {
    require(rdg::dungeon_layout_from_string("Cross") == rdg::DungeonLayout::Cross);
    require(rdg::dungeon_layout_from_string("None") == rdg::DungeonLayout::None);
    require(rdg::dungeon_layout_from_string("Box") == rdg::DungeonLayout::Box);
    require(rdg::dungeon_layout_from_string("Round") == rdg::DungeonLayout::Round);
    require(!rdg::dungeon_layout_from_string("bad").has_value());
    require(rdg::room_layout_from_string("Packed") == rdg::RoomLayout::Packed);
    require(rdg::room_layout_from_string("Scattered") == rdg::RoomLayout::Scattered);
    require(!rdg::room_layout_from_string("bad").has_value());
    require(rdg::map_style_from_string("Standard") == rdg::MapStyle::Standard);
    require(!rdg::map_style_from_string("bad").has_value());
    require(rdg::to_string(rdg::DungeonLayout::None) == "None");
    require(rdg::to_string(rdg::DungeonLayout::Box) == "Box");
    require(rdg::to_string(rdg::DungeonLayout::Cross) == "Cross");
    require(rdg::to_string(rdg::DungeonLayout::Round) == "Round");
    require(rdg::to_string(rdg::RoomLayout::Scattered) == "Scattered");
    require(rdg::to_string(rdg::RoomLayout::Packed) == "Packed");
    require(rdg::to_string(rdg::MapStyle::Standard) == "Standard");

    rdg::Options options;
    options.n_rows = 20;
    require(rdg::validate_options(options).has_value());
}

static void require_validation_error(rdg::Options options, std::string_view expected_message) {
    const auto validation = rdg::validate_options(options);
    require(validation.has_value());
    require(validation->message == expected_message);

    std::mt19937 rng{7};
    bool threw = false;
    try {
        (void)rdg::create_dungeon(options, rng);
    } catch (const std::invalid_argument &error) {
        threw = true;
        require(error.what() == expected_message);
    }
    require(threw);
}

static void test_option_validation_errors() {
    rdg::Options options;

    auto invalid = options;
    invalid.n_rows = 1;
    require_validation_error(invalid, "n_rows and n_cols must both be at least 3");

    invalid = options;
    invalid.n_cols = 10;
    require_validation_error(invalid, "n_rows and n_cols must be odd");

    invalid = options;
    invalid.room_min = 0;
    require_validation_error(invalid, "room_min and room_max must be positive");

    invalid = options;
    invalid.room_min = 9;
    invalid.room_max = 3;
    require_validation_error(invalid, "room_min must not exceed room_max");

    invalid = options;
    invalid.remove_deadends = -1;
    require_validation_error(invalid, "remove_deadends must be between 0 and 100");

    invalid = options;
    invalid.remove_deadends = 101;
    require_validation_error(invalid, "remove_deadends must be between 0 and 100");

    invalid = options;
    invalid.add_stairs = -1;
    require_validation_error(invalid, "add_stairs must not be negative");

    invalid = options;
    invalid.cell_size = 0;
    require_validation_error(invalid, "cell_size must be positive");
}

static void test_compact_metadata_helpers() {
    const std::array door_cases{
            std::pair{rdg::DoorKind::None, std::pair{"", ""}},
            std::pair{rdg::DoorKind::Arch, std::pair{"arch", "Archway"}},
            std::pair{rdg::DoorKind::Open, std::pair{"open", "Unlocked Door"}},
            std::pair{rdg::DoorKind::Locked, std::pair{"lock", "Locked Door"}},
            std::pair{rdg::DoorKind::Trapped, std::pair{"trap", "Trapped Door"}},
            std::pair{rdg::DoorKind::Secret, std::pair{"secret", "Secret Door"}},
            std::pair{rdg::DoorKind::Portcullis, std::pair{"portc", "Portcullis"}},
    };
    for (const auto &[kind, expected]: door_cases) {
        rdg::Door door;
        door.kind = kind;
        require(door.getKey() == expected.first);
        require(door.getType() == expected.second);
        require(rdg::key(kind) == expected.first);
        require(rdg::to_string(kind) == expected.second);
    }

    const std::array stair_cases{
            std::pair{rdg::StairKind::None, ""},
            std::pair{rdg::StairKind::Down, "down"},
            std::pair{rdg::StairKind::Up, "up"},
    };
    for (const auto &[kind, expected]: stair_cases) {
        rdg::Stairs stairs;
        stairs.kind = kind;
        require(stairs.getKey() == expected);
        require(rdg::key(kind) == expected);
    }
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
    test_generation_option_matrix();
    test_generated_door_and_stair_metadata();
    test_option_helpers();
    test_option_validation_errors();
    test_compact_metadata_helpers();
    test_seeded_rng_is_deterministic();
    return 0;
}
