#include "rdg.h"

#include <algorithm>
#include <cassert>
#include <string>

static void test_cell_type_management() {
    rdg<>::Cell cell;

    cell.setType(rdg<>::ROOM);
    assert(cell.hasType(rdg<>::ROOM));
    assert(!cell.hasType(rdg<>::CORRIDOR));
    assert(cell.isBlockedRoom());
    assert(cell.isOpenspace());

    cell.addType(rdg<>::ENTRANCE);
    assert(cell.isEspace());

    cell.addType(rdg<>::LOCKED);
    assert(cell.isDoorspace());
    assert(cell.isBlockedDoor());

    cell.clearEspace();
    assert(!cell.hasType(rdg<>::ENTRANCE));
    assert(!cell.hasType(rdg<>::LOCKED));

    cell.clearTypes();
    assert(!cell.hasType(rdg<>::ROOM));
}

static void test_cell_labels_and_stairs() {
    rdg<>::Cell cell;

    assert(!cell.hasLabel());
    cell.setLabel("A1");
    assert(cell.hasLabel());
    assert(cell.getLabel() == "A1");

    cell.clearLabel();
    assert(!cell.hasLabel());

    cell.addType(rdg<>::STAIR_UP);
    assert(cell.isStairs());
    cell.removeType(rdg<>::STAIR_UP);
    cell.addType(rdg<>::STAIR_DN);
    assert(cell.isStairs());
}

static void test_generate_default_dungeon() {
    rdg<>::Options options;
    options.n_rows = 21;
    options.n_cols = 21;
    options.add_stairs = 2;
    options.remove_deadends = 50;

    auto dungeon = rdg<>::create_dungeon(options);

    const auto &cells = dungeon.getCells();
    assert(!cells.empty());
    assert(static_cast<int>(cells.size()) == options.n_rows);
    assert(static_cast<int>(cells.front().size()) == options.n_cols);

    bool has_open_space = false;
    bool has_room_label = false;
    for (const auto &row : cells) {
        for (const auto &cell : row) {
            if (cell.isOpenspace()) {
                has_open_space = true;
            }
            if (cell.hasLabel()) {
                has_room_label = true;
            }
        }
    }

    assert(has_open_space);
    assert(has_room_label);
    assert(!dungeon.getRooms().empty());
}

static void test_layout_variants() {
    rdg<>::Options options;
    options.n_rows = 25;
    options.n_cols = 25;
    options.dungeon_layout = "Cross";
    options.room_layout = "Packed";
    options.corridor_layout = rdg<>::STRAIGHT;
    options.add_stairs = 0;
    options.remove_deadends = 100;

    auto dungeon = rdg<>::create_dungeon(options);

    assert(!dungeon.getRooms().empty());

    const auto &cells = dungeon.getCells();
    int open_cells = 0;
    for (const auto &row : cells) {
        for (const auto &cell : row) {
            if (cell.isOpenspace() || cell.hasLabel()) {
                open_cells++;
            }
        }
    }

    assert(open_cells > 0);
}

int main() {
    test_cell_type_management();
    test_cell_labels_and_stairs();
    test_generate_default_dungeon();
    test_layout_variants();
    return 0;
}
