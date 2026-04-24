#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <map>
#include <optional>
#include <queue>
#include <random>
#include <set>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tuple>
#include <utility>
#include <vector>

#include <vstd.h>

namespace rdg {

enum class CellType : std::uint8_t {
    BLOCKED,
    ROOM,
    CORRIDOR,
    PERIMETER,
    ENTRANCE,
    ARCH,
    DOOR,
    LOCKED,
    TRAPPED,
    SECRET,
    PORTC,
    STAIR_DN,
    STAIR_UP
};

enum class CorridorLayout : int {
    BENT = 50,
    STRAIGHT = 100,
    LABYRINTH = 0
};

enum class Direction : std::uint8_t {
    NORTH = 0,
    SOUTH = 1,
    EAST = 2,
    WEST = 3
};

enum class DungeonLayout : std::uint8_t {
    None,
    Box,
    Cross,
    Round
};

enum class RoomLayout : std::uint8_t {
    Scattered,
    Packed
};

enum class MapStyle : std::uint8_t {
    Standard
};

[[nodiscard]] inline std::optional<DungeonLayout> dungeon_layout_from_string(std::string_view value) {
    if (value == "None") {
        return DungeonLayout::None;
    }
    if (value == "Box") {
        return DungeonLayout::Box;
    }
    if (value == "Cross") {
        return DungeonLayout::Cross;
    }
    if (value == "Round") {
        return DungeonLayout::Round;
    }
    return std::nullopt;
}

[[nodiscard]] inline std::optional<RoomLayout> room_layout_from_string(std::string_view value) {
    if (value == "Scattered") {
        return RoomLayout::Scattered;
    }
    if (value == "Packed") {
        return RoomLayout::Packed;
    }
    return std::nullopt;
}

[[nodiscard]] inline std::optional<MapStyle> map_style_from_string(std::string_view value) {
    if (value == "Standard") {
        return MapStyle::Standard;
    }
    return std::nullopt;
}

[[nodiscard]] inline std::string_view to_string(DungeonLayout layout) {
    switch (layout) {
        case DungeonLayout::None:
            return "None";
        case DungeonLayout::Box:
            return "Box";
        case DungeonLayout::Cross:
            return "Cross";
        case DungeonLayout::Round:
            return "Round";
    }
    return "None";
}

[[nodiscard]] inline std::string_view to_string(RoomLayout layout) {
    switch (layout) {
        case RoomLayout::Scattered:
            return "Scattered";
        case RoomLayout::Packed:
            return "Packed";
    }
    return "Scattered";
}

[[nodiscard]] inline std::string_view to_string(MapStyle style) {
    switch (style) {
        case MapStyle::Standard:
            return "Standard";
    }
    return "Standard";
}

class Cell {
    std::uint16_t types = 0;
    int room_id = 0;
    std::string label;

    [[nodiscard]] static constexpr std::uint16_t type_bit(CellType type) {
        return static_cast<std::uint16_t>(1u << static_cast<unsigned>(type));
    }

public:
    void setType(CellType type) {
        types = 0;
        addType(type);
    }

    [[nodiscard]] bool isBlockedRoom() const {
        return hasType(CellType::BLOCKED)
               || hasType(CellType::ROOM);
    }

    [[nodiscard]] bool isBlockedCorridor() const {
        return hasType(CellType::BLOCKED)
               || hasType(CellType::PERIMETER)
               || hasType(CellType::CORRIDOR);
    }

    [[nodiscard]] bool isBlockedDoor() const {
        return hasType(CellType::BLOCKED)
               || isDoorspace();
    }

    [[nodiscard]] bool hasLabel() const {
        return !label.empty();
    }

    [[nodiscard]] const std::string &getLabel() const {
        return label;
    }

    [[nodiscard]] bool isEspace() const {
        return hasType(CellType::ENTRANCE)
               || isDoorspace()
               || hasLabel();
    }

    void addType(CellType type) {
        types |= type_bit(type);
    }

    void removeType(CellType type) {
        types &= static_cast<std::uint16_t>(~type_bit(type));
    }

    [[nodiscard]] bool hasType(CellType type) const {
        return (types & type_bit(type)) != 0;
    }

    [[nodiscard]] bool isOpenspace() const {
        return hasType(CellType::ROOM)
               || hasType(CellType::CORRIDOR);
    }

    [[nodiscard]] bool isDoorspace() const {
        return hasType(CellType::ARCH)
               || hasType(CellType::DOOR)
               || hasType(CellType::LOCKED)
               || hasType(CellType::TRAPPED)
               || hasType(CellType::SECRET)
               || hasType(CellType::PORTC);
    }

    [[nodiscard]] bool isStairs() const {
        return hasType(CellType::STAIR_UP)
               || hasType(CellType::STAIR_DN);
    }

    void setRoomId(int room_id) {
        this->room_id = room_id;
    }

    [[nodiscard]] int getRoomId() const {
        return room_id;
    }

    void setLabel(std::string label) {
        this->label = std::move(label);
    }

    void clearTypes() {
        types = 0;
    }

    void clearLabel() {
        label.clear();
    }

    void clearEspace() {
        clearLabel();
        removeType(CellType::ENTRANCE);
        removeType(CellType::ARCH);
        removeType(CellType::DOOR);
        removeType(CellType::LOCKED);
        removeType(CellType::TRAPPED);
        removeType(CellType::SECRET);
        removeType(CellType::PORTC);
    }
};

struct Door;

struct Room {
    int id = 0;
    int row = 0;
    int col = 0;
    int north = 0;
    int south = 0;
    int west = 0;
    int east = 0;
    int height = 0;
    int width = 0;
    int area = 0;

    std::multimap<Direction, Door> door;
};

struct Stairs {
    int row = 0;
    int col = 0;
    int next_row = 0;
    int next_col = 0;
    std::string key;
};

struct Options {
    int n_rows = 33;  // must be an odd number
    int n_cols = 33;  // must be an odd number
    DungeonLayout dungeon_layout = DungeonLayout::None;
    int room_min = 3; // minimum room size
    int room_max = 9; // maximum room size
    RoomLayout room_layout = RoomLayout::Scattered;
    CorridorLayout corridor_layout = CorridorLayout::LABYRINTH;
    int remove_deadends = 100; // percentage
    int add_stairs = 2; // number of stairs
    MapStyle map_style = MapStyle::Standard;
    int cell_size = 18; // pixels
};

struct ValidationError {
    std::string message;
};

[[nodiscard]] inline std::optional<ValidationError> validate_options(const Options &options) {
    if (options.n_rows < 3 || options.n_cols < 3) {
        return ValidationError{"n_rows and n_cols must both be at least 3"};
    }
    if (options.n_rows % 2 == 0 || options.n_cols % 2 == 0) {
        return ValidationError{"n_rows and n_cols must be odd"};
    }
    if (options.room_min < 1 || options.room_max < 1) {
        return ValidationError{"room_min and room_max must be positive"};
    }
    if (options.room_min > options.room_max) {
        return ValidationError{"room_min must not exceed room_max"};
    }
    if (options.remove_deadends < 0 || options.remove_deadends > 100) {
        return ValidationError{"remove_deadends must be between 0 and 100"};
    }
    if (options.add_stairs < 0) {
        return ValidationError{"add_stairs must not be negative"};
    }
    if (options.cell_size <= 0) {
        return ValidationError{"cell_size must be positive"};
    }
    return std::nullopt;
}

struct Door {
    int row = 0;
    int col = 0;
    std::string key;
    std::string type;
    int out_id = 0;
};

class Dungeon;

[[nodiscard]] Dungeon create_dungeon(Options options);
[[nodiscard]] Dungeon create_dungeon(Options options, std::mt19937 &rng);

namespace detail {

using Offset = std::array<int, 2>;
using Mask = std::array<std::array<int, 3>, 3>;

struct Sill {
    int sill_r = 0;
    int sill_c = 0;
    Direction dir = Direction::NORTH;
    int door_r = 0;
    int door_c = 0;
    int out_id = 0;
};

struct TunnelOffsets {
    std::span<const Offset> walled;
    std::span<const Offset> corridor;
    std::span<const Offset> stair;
    std::span<const Offset> next;
    std::span<const Offset> close;
    std::span<const Offset> open;
    std::span<const Offset> recurse;
};

inline constexpr std::array<Direction, 4> DIRECTIONS = {
        Direction::NORTH,
        Direction::SOUTH,
        Direction::EAST,
        Direction::WEST
};

inline constexpr std::array<int, 4> DI = {-1, 1, 0, 0};
inline constexpr std::array<int, 4> DJ = {0, 0, 1, -1};

inline constexpr std::array<Direction, 4> OPPOSITE = {
        Direction::SOUTH,
        Direction::NORTH,
        Direction::WEST,
        Direction::EAST
};

[[nodiscard]] constexpr std::size_t direction_index(Direction dir) {
    return static_cast<std::size_t>(dir);
}

[[nodiscard]] constexpr int dir_i(Direction dir) {
    return DI[direction_index(dir)];
}

[[nodiscard]] constexpr int dir_j(Direction dir) {
    return DJ[direction_index(dir)];
}

[[nodiscard]] constexpr Direction opposite(Direction dir) {
    return OPPOSITE[direction_index(dir)];
}

inline constexpr Mask BOX_MASK = {{
        {{1, 1, 1}},
        {{1, 0, 1}},
        {{1, 1, 1}}
}};

inline constexpr Mask CROSS_MASK = {{
        {{0, 1, 0}},
        {{1, 1, 1}},
        {{0, 1, 0}}
}};

[[nodiscard]] constexpr const Mask *mask_for(DungeonLayout layout) {
    switch (layout) {
        case DungeonLayout::Box:
            return &BOX_MASK;
        case DungeonLayout::Cross:
            return &CROSS_MASK;
        case DungeonLayout::None:
        case DungeonLayout::Round:
            return nullptr;
    }
    return nullptr;
}

inline constexpr std::array<Offset, 7> STAIR_N_WALLED = {
        Offset{1, -1}, Offset{0, -1}, Offset{-1, -1}, Offset{-1, 0}, Offset{-1, 1}, Offset{0, 1}, Offset{1, 1}
};
inline constexpr std::array<Offset, 3> STAIR_N_CORRIDOR = {Offset{0, 0}, Offset{1, 0}, Offset{2, 0}};
inline constexpr std::array<Offset, 1> STAIR_N_STAIR = {Offset{0, 0}};
inline constexpr std::array<Offset, 1> STAIR_N_NEXT = {Offset{1, 0}};

inline constexpr std::array<Offset, 7> STAIR_S_WALLED = {
        Offset{-1, -1}, Offset{0, -1}, Offset{1, -1}, Offset{1, 0}, Offset{1, 1}, Offset{0, 1}, Offset{-1, 1}
};
inline constexpr std::array<Offset, 3> STAIR_S_CORRIDOR = {Offset{0, 0}, Offset{-1, 0}, Offset{-2, 0}};
inline constexpr std::array<Offset, 1> STAIR_S_STAIR = {Offset{0, 0}};
inline constexpr std::array<Offset, 1> STAIR_S_NEXT = {Offset{-1, 0}};

inline constexpr std::array<Offset, 7> STAIR_E_WALLED = {
        Offset{-1, -1}, Offset{-1, 0}, Offset{-1, 1}, Offset{0, 1}, Offset{1, 1}, Offset{1, 0}, Offset{1, -1}
};
inline constexpr std::array<Offset, 3> STAIR_E_CORRIDOR = {Offset{0, 0}, Offset{0, -1}, Offset{0, -2}};
inline constexpr std::array<Offset, 1> STAIR_E_STAIR = {Offset{0, 0}};
inline constexpr std::array<Offset, 1> STAIR_E_NEXT = {Offset{0, -1}};

inline constexpr std::array<Offset, 7> STAIR_W_WALLED = {
        Offset{-1, 1}, Offset{-1, 0}, Offset{-1, -1}, Offset{0, -1}, Offset{1, -1}, Offset{1, 0}, Offset{1, 1}
};
inline constexpr std::array<Offset, 3> STAIR_W_CORRIDOR = {Offset{0, 0}, Offset{0, 1}, Offset{0, 2}};
inline constexpr std::array<Offset, 1> STAIR_W_STAIR = {Offset{0, 0}};
inline constexpr std::array<Offset, 1> STAIR_W_NEXT = {Offset{0, 1}};

inline constexpr std::array<TunnelOffsets, 4> STAIR_END = {{
        {STAIR_N_WALLED, STAIR_N_CORRIDOR, STAIR_N_STAIR, STAIR_N_NEXT, {}, {}, {}},
        {STAIR_S_WALLED, STAIR_S_CORRIDOR, STAIR_S_STAIR, STAIR_S_NEXT, {}, {}, {}},
        {STAIR_E_WALLED, STAIR_E_CORRIDOR, STAIR_E_STAIR, STAIR_E_NEXT, {}, {}, {}},
        {STAIR_W_WALLED, STAIR_W_CORRIDOR, STAIR_W_STAIR, STAIR_W_NEXT, {}, {}, {}}
}};

inline constexpr std::array<Offset, 5> CLOSE_N_WALLED = {
        Offset{0, -1}, Offset{1, -1}, Offset{1, 0}, Offset{1, 1}, Offset{0, 1}
};
inline constexpr std::array<Offset, 1> CLOSE_N_CLOSE = {Offset{0, 0}};
inline constexpr std::array<Offset, 1> CLOSE_N_RECURSE = {Offset{-1, 0}};

inline constexpr std::array<Offset, 5> CLOSE_S_WALLED = {
        Offset{0, -1}, Offset{-1, -1}, Offset{-1, 0}, Offset{-1, 1}, Offset{0, 1}
};
inline constexpr std::array<Offset, 1> CLOSE_S_CLOSE = {Offset{0, 0}};
inline constexpr std::array<Offset, 1> CLOSE_S_RECURSE = {Offset{1, 0}};

inline constexpr std::array<Offset, 5> CLOSE_E_WALLED = {
        Offset{-1, 0}, Offset{-1, -1}, Offset{0, -1}, Offset{1, -1}, Offset{1, 0}
};
inline constexpr std::array<Offset, 1> CLOSE_E_CLOSE = {Offset{0, 0}};
inline constexpr std::array<Offset, 1> CLOSE_E_RECURSE = {Offset{0, 1}};

inline constexpr std::array<Offset, 5> CLOSE_W_WALLED = {
        Offset{-1, 0}, Offset{-1, 1}, Offset{0, 1}, Offset{1, 1}, Offset{1, 0}
};
inline constexpr std::array<Offset, 1> CLOSE_W_CLOSE = {Offset{0, 0}};
inline constexpr std::array<Offset, 1> CLOSE_W_RECURSE = {Offset{0, -1}};

inline constexpr std::array<TunnelOffsets, 4> CLOSE_END = {{
        {CLOSE_N_WALLED, {}, {}, {}, CLOSE_N_CLOSE, {}, CLOSE_N_RECURSE},
        {CLOSE_S_WALLED, {}, {}, {}, CLOSE_S_CLOSE, {}, CLOSE_S_RECURSE},
        {CLOSE_E_WALLED, {}, {}, {}, CLOSE_E_CLOSE, {}, CLOSE_E_RECURSE},
        {CLOSE_W_WALLED, {}, {}, {}, CLOSE_W_CLOSE, {}, CLOSE_W_RECURSE}
}};

} // namespace detail

class Dungeon {
    friend Dungeon create_dungeon(Options options);
    friend Dungeon create_dungeon(Options options, std::mt19937 &rng);

public:
    [[nodiscard]] const auto &getCells() const {
        return cells;
    }

    [[nodiscard]] const auto &getStairs() const {
        return stairs;
    }

    [[nodiscard]] const auto &getRooms() const {
        return rooms;
    }

    [[nodiscard]] const auto &getDoors() const {
        return doors;
    }

    [[nodiscard]] int rowCount() const {
        return n_rows + 1;
    }

    [[nodiscard]] int colCount() const {
        return n_cols + 1;
    }

    [[nodiscard]] const Cell &cellAt(int row, int col) const {
        return cell(row, col);
    }

private:
    Options options;
    std::mt19937 &rng;
    std::vector<Cell> cells;
    std::vector<Room> rooms;
    std::vector<Stairs> stairs;
    std::vector<std::vector<Door>> doors;

    const int n_i;
    const int n_j;
    const int n_rows;
    const int n_cols;
    const int max_row;
    const int max_col;
    const int room_base;
    const int room_radix;
    int n_rooms = 0;
    int last_room_id = 0;

    explicit Dungeon(Options options, std::mt19937 &rng) :
            options(std::move(options)),
            rng(rng),
            n_i(this->options.n_rows / 2),
            n_j(this->options.n_cols / 2),
            n_rows(n_i * 2),
            n_cols(n_j * 2),
            max_row(n_rows - 1),
            max_col(n_cols - 1),
            room_base((this->options.room_min + 1) / 2),
            room_radix(((this->options.room_max - this->options.room_min) / 2) + 1) {}

    [[nodiscard]] std::size_t index(int row, int col) const {
        return static_cast<std::size_t>(row * colCount() + col);
    }

    [[nodiscard]] Cell &cell(int row, int col) {
        return cells[index(row, col)];
    }

    [[nodiscard]] const Cell &cell(int row, int col) const {
        return cells[index(row, col)];
    }

    [[nodiscard]] Room &room_by_id(int id) {
        return rooms[static_cast<std::size_t>(id - 1)];
    }

    [[nodiscard]] const Room &room_by_id(int id) const {
        return rooms[static_cast<std::size_t>(id - 1)];
    }

    [[nodiscard]] int random_int(int max_exclusive) {
        if (max_exclusive <= 0) {
            return 0;
        }
        std::uniform_int_distribution<int> dist(0, max_exclusive - 1);
        return dist(rng);
    }

    [[nodiscard]] int random_int(int min_inclusive, int max_inclusive) {
        if (max_inclusive < min_inclusive) {
            return min_inclusive;
        }
        std::uniform_int_distribution<int> dist(min_inclusive, max_inclusive);
        return dist(rng);
    }

    template<typename E>
    [[nodiscard]] E pop_random(std::vector<E> &items) {
        auto index = static_cast<std::size_t>(random_int(static_cast<int>(items.size())));
        E value = std::move(items[index]);
        if (index != items.size() - 1) {
            items[index] = std::move(items.back());
        }
        items.pop_back();
        return value;
    }

    void init_cells() {
        cells.resize(static_cast<std::size_t>(rowCount() * colCount()));

        if (const auto *mask = detail::mask_for(options.dungeon_layout)) {
            mask_cells(*mask);
        } else if (options.dungeon_layout == DungeonLayout::Round) {
            round_mask();
        }
    }

    void mask_cells(const detail::Mask &mask) {
        const auto r_scale = static_cast<double>(mask.size()) / static_cast<double>(rowCount());
        const auto c_scale = static_cast<double>(mask.front().size()) / static_cast<double>(colCount());

        for (int r = 0; r < n_rows; r++) {
            for (int c = 0; c < n_cols; c++) {
                const auto mask_r = static_cast<std::size_t>(static_cast<double>(r) * r_scale);
                const auto mask_c = static_cast<std::size_t>(static_cast<double>(c) * c_scale);
                if (!mask[mask_r][mask_c]) {
                    cell(r, c).setType(CellType::BLOCKED);
                }
            }
        }
    }

    void round_mask() {
        const int center_r = n_rows / 2;
        const int center_c = n_cols / 2;
        const int radius_sq = center_c * center_c;

        for (int r = 0; r < n_rows; r++) {
            for (int c = 0; c < n_cols; c++) {
                const int dr = r - center_r;
                const int dc = c - center_c;
                if ((dr * dr) + (dc * dc) > radius_sq) {
                    cell(r, c).setType(CellType::BLOCKED);
                }
            }
        }
    }

    void emplace_rooms() {
        rooms.reserve(static_cast<std::size_t>(n_i * n_j));
        if (options.room_layout == RoomLayout::Packed) {
            pack_rooms();
        } else {
            scatter_rooms();
        }
    }

    void pack_rooms() {
        for (int i = 0; i < n_i; i++) {
            const auto r = (i * 2) + 1;
            for (int j = 0; j < n_j; j++) {
                const auto c = (j * 2) + 1;

                if (cell(r, c).hasType(CellType::ROOM)) {
                    continue;
                }
                if ((i == 0 || j == 0) && random_int(0, 1)) {
                    continue;
                }

                emplace_room(i, j);
            }
        }
    }

    void emplace_room(int room_i = -1, int room_j = -1, int height = -1, int width = -1) {
        if (n_rooms == 999) {
            return;
        }

        auto [i, j, room_height, room_width] = set_room(room_i, room_j, height, width);

        const int r1 = i * 2 + 1;
        const int c1 = j * 2 + 1;
        const int r2 = (i + room_height) * 2 - 1;
        const int c2 = (j + room_width) * 2 - 1;

        if (r1 < 1 || r2 > max_row) {
            return;
        }
        if (c1 < 1 || c2 > max_col) {
            return;
        }

        auto [hit, blocked] = sound_room(r1, c1, r2, c2);

        if (blocked || hit) {
            return;
        }

        const int room_id = static_cast<int>(rooms.size()) + 1;
        n_rooms = room_id;
        last_room_id = room_id;

        for (int r = r1; r <= r2; r++) {
            for (int c = c1; c <= c2; c++) {
                auto &target = cell(r, c);
                if (target.hasType(CellType::ENTRANCE)) {
                    target.clearEspace();
                } else if (target.hasType(CellType::PERIMETER)) {
                    target.removeType(CellType::PERIMETER);
                }
                target.addType(CellType::ROOM);
                target.setRoomId(room_id);
            }
        }

        const int h = (r2 - r1) + 1;
        const int w = (c2 - c1) + 1;
        rooms.push_back({room_id, r1, c1, r1, r2, c1, c2, h, w, h * w, {}});

        for (int r = r1 - 1; r <= r2 + 1; r++) {
            if (!(cell(r, c1 - 1).hasType(CellType::ROOM)
                  || cell(r, c1 - 1).hasType(CellType::ENTRANCE))) {
                cell(r, c1 - 1).addType(CellType::PERIMETER);
            }
            if (!(cell(r, c2 + 1).hasType(CellType::ROOM)
                  || cell(r, c2 + 1).hasType(CellType::ENTRANCE))) {
                cell(r, c2 + 1).addType(CellType::PERIMETER);
            }
        }
        for (int c = c1 - 1; c <= c2 + 1; c++) {
            if (!(cell(r1 - 1, c).hasType(CellType::ROOM)
                  || cell(r1 - 1, c).hasType(CellType::ENTRANCE))) {
                cell(r1 - 1, c).addType(CellType::PERIMETER);
            }
            if (!(cell(r2 + 1, c).hasType(CellType::ROOM)
                  || cell(r2 + 1, c).hasType(CellType::ENTRANCE))) {
                cell(r2 + 1, c).addType(CellType::PERIMETER);
            }
        }
    }

    [[nodiscard]] std::tuple<int, int, int, int> set_room(int room_i, int room_j, int height, int width) {
        if (height < 0) {
            if (room_i < 0) {
                height = random_int(room_radix) + room_base;
            } else {
                int a = n_i - room_base - room_i;
                a = a < 0 ? 0 : a;
                auto r = (a < room_radix) ? a : room_radix;

                height = random_int(r) + room_base;
            }
        }
        if (width < 0) {
            if (room_j < 0) {
                width = random_int(room_radix) + room_base;
            } else {
                int a = n_j - room_base - room_j;
                a = a < 0 ? 0 : a;
                auto r = (a < room_radix) ? a : room_radix;

                width = random_int(r) + room_base;
            }
        }

        return std::make_tuple(room_i < 0 ? random_int(n_i - height) : room_i,
                               room_j < 0 ? random_int(n_j - width) : room_j,
                               height,
                               width);
    }

    [[nodiscard]] std::tuple<bool, bool> sound_room(int r1, int c1, int r2, int c2) const {
        for (int r = r1; r <= r2; r++) {
            for (int c = c1; c <= c2; c++) {
                if (cell(r, c).hasType(CellType::BLOCKED)) {
                    return std::make_tuple(false, true);
                }
                if (cell(r, c).hasType(CellType::ROOM)) {
                    return std::make_tuple(true, false);
                }
            }
        }
        return std::make_tuple(false, false);
    }

    void scatter_rooms() {
        for (int i = 0; i < alloc_rooms(); i++) {
            emplace_room();
        }
    }

    [[nodiscard]] int alloc_rooms() const {
        const int dungeon_area = n_cols * n_rows;
        const int room_area = options.room_max * options.room_max;
        return dungeon_area / room_area;
    }

    void open_room(Room &room, std::set<std::pair<int, int>> &connected) {
        auto candidates = door_sills(room);
        if (candidates.empty()) {
            return;
        }
        auto n_opens = alloc_opens(room);

        for (int i = 0; i < n_opens && !candidates.empty(); i++) {
            const auto sill = pop_random(candidates);
            const auto door_r = sill.door_r;
            const auto door_c = sill.door_c;
            const auto &door_cell = cell(door_r, door_c);

            if (door_cell.isDoorspace()) {
                n_opens--;
                continue;
            }

            const auto out_id = sill.out_id;
            if (out_id) {
                auto connect = std::make_pair(std::min(room.id, out_id), std::max(room.id, out_id));

                if (vstd::ctn(connected, connect)) {
                    n_opens--;
                    continue;
                }

                connected.insert(connect);
            }
            const auto open_r = sill.sill_r;
            const auto open_c = sill.sill_c;
            const auto open_dir = sill.dir;

            for (auto x = 0; x < 3; x++) {
                const auto r = open_r + (detail::dir_i(open_dir) * x);
                const auto c = open_c + (detail::dir_j(open_dir) * x);

                cell(r, c).removeType(CellType::PERIMETER);
                cell(r, c).addType(CellType::ENTRANCE);
            }

            Door door;
            door.row = door_r;
            door.col = door_c;
            apply_door_type(generate_door_type(), door);

            door.out_id = out_id;
            if (out_id) {
                room.door.insert(std::make_pair(open_dir, door));
            }
        }
    }

    void apply_door_type(CellType door_type, Door &door) {
        cell(door.row, door.col).addType(door_type);
        switch (door_type) {
            case CellType::ARCH:
                cell(door.row, door.col).setLabel("a");
                door.key = "arch";
                door.type = "Archway";
                break;
            case CellType::DOOR:
                cell(door.row, door.col).setLabel("o");
                door.key = "open";
                door.type = "Unlocked Door";
                break;
            case CellType::LOCKED:
                cell(door.row, door.col).setLabel("x");
                door.key = "lock";
                door.type = "Locked Door";
                break;
            case CellType::TRAPPED:
                cell(door.row, door.col).setLabel("t");
                door.key = "trap";
                door.type = "Trapped Door";
                break;
            case CellType::SECRET:
                cell(door.row, door.col).setLabel("s");
                door.key = "secret";
                door.type = "Secret Door";
                break;
            case CellType::PORTC:
                cell(door.row, door.col).setLabel("p");
                door.key = "portc";
                door.type = "Portcullis";
                break;
            default:
                break;
        }
    }

    [[nodiscard]] CellType generate_door_type() {
        const auto i = random_int(110);

        if (i < 15) {
            return CellType::ARCH;
        }
        if (i < 60) {
            return CellType::DOOR;
        }
        if (i < 75) {
            return CellType::LOCKED;
        }
        if (i < 90) {
            return CellType::TRAPPED;
        }
        if (i < 100) {
            return CellType::SECRET;
        }
        return CellType::PORTC;
    }

    [[nodiscard]] int alloc_opens(const Room &room) {
        const auto room_h = ((room.south - room.north) / 2) + 1;
        const auto room_w = ((room.east - room.west) / 2) + 1;
        const auto flumph = static_cast<int>(std::sqrt(static_cast<double>(room_w * room_h)));
        return flumph + random_int(flumph);
    }

    [[nodiscard]] std::optional<detail::Sill> check_sill(int sill_r, int sill_c, Direction dir) const {
        const auto door_r = sill_r + detail::dir_i(dir);
        const auto door_c = sill_c + detail::dir_j(dir);
        const auto &door_cell = cell(door_r, door_c);
        if (!(door_cell.hasType(CellType::PERIMETER))) {
            return {};
        }
        if (door_cell.isBlockedDoor()) {
            return {};
        }
        const auto out_r = door_r + detail::dir_i(dir);
        const auto out_c = door_c + detail::dir_j(dir);
        const auto &out_cell = cell(out_r, out_c);
        if (out_cell.hasType(CellType::BLOCKED)) {
            return {};
        }
        auto out_id = 0;
        if (out_cell.hasType(CellType::ROOM)) {
            out_id = out_cell.getRoomId();
        }
        return detail::Sill{sill_r, sill_c, dir, door_r, door_c, out_id};
    }

    [[nodiscard]] std::vector<detail::Sill> door_sills(const Room &room) const {
        std::vector<detail::Sill> sills;
        sills.reserve(static_cast<std::size_t>(
                ((room.east - room.west) / 2 + 1) * 2
                + ((room.south - room.north) / 2 + 1) * 2));
        if (room.north >= 3) {
            for (int c = room.west; c <= room.east; c += 2) {
                if (auto sill = check_sill(room.north, c, Direction::NORTH)) {
                    sills.push_back(sill.value());
                }
            }
        }
        if (room.south <= n_rows - 3) {
            for (int c = room.west; c <= room.east; c += 2) {
                if (auto sill = check_sill(room.south, c, Direction::SOUTH)) {
                    sills.push_back(sill.value());
                }
            }
        }
        if (room.west >= 3) {
            for (int r = room.north; r <= room.south; r += 2) {
                if (auto sill = check_sill(r, room.west, Direction::WEST)) {
                    sills.push_back(sill.value());
                }
            }
        }
        if (room.east <= n_cols - 3) {
            for (int r = room.north; r <= room.south; r += 2) {
                if (auto sill = check_sill(r, room.east, Direction::EAST)) {
                    sills.push_back(sill.value());
                }
            }
        }
        return sills;
    }

    void open_rooms() {
        std::set<std::pair<int, int>> connected;
        for (int id = 1; id <= n_rooms; id++) {
            open_room(room_by_id(id), connected);
        }
    }

    void label_rooms() {
        for (const auto &room: rooms) {
            auto label = std::to_string(room.id);
            auto len = label.length();
            const auto label_r = int((room.north + room.south) / 2);
            const auto label_c = int((room.west + room.east - static_cast<int>(len)) / 2) + 1;

            for (decltype(len) c = 0; c < len; c++) {
                cell(label_r, label_c + static_cast<int>(c)).setLabel(label.substr(c, 1));
            }
        }
    }

    void corridors() {
        for (auto i = 1; i < n_i; i++) {
            const auto r = (i * 2) + 1;
            for (auto j = 1; j < n_j; j++) {
                const auto c = (j * 2) + 1;

                if (cell(r, c).hasType(CellType::CORRIDOR)) {
                    continue;
                }
                tunnel(i, j);
            }
        }
    }

    struct TunnelStep {
        int i = 0;
        int j = 0;
        std::optional<Direction> last_dir;
    };

    void tunnel(int start_i, int start_j, std::optional<Direction> last_dir = std::nullopt) {
        std::queue<TunnelStep> args;
        args.push({start_i, start_j, last_dir});
        while (!args.empty()) {
            auto arg = args.front();
            args.pop();
            auto dirs = tunnel_dirs(arg.last_dir);
            for (const auto &dir: dirs) {
                if (open_tunnel(arg.i, arg.j, dir)) {
                    const auto next_i = arg.i + detail::dir_i(dir);
                    const auto next_j = arg.j + detail::dir_j(dir);

                    args.push({next_i, next_j, dir});
                }
            }
        }
    }

    [[nodiscard]] std::array<Direction, 4> tunnel_dirs(const std::optional<Direction> &last_dir) {
        auto p = static_cast<int>(options.corridor_layout);
        auto dirs = detail::DIRECTIONS;
        std::shuffle(dirs.begin(), dirs.end(), rng);

        if (last_dir.has_value() && p > 0 && random_int(100) < p) {
            std::stable_partition(dirs.begin(), dirs.end(), [&](Direction dir) {
                return dir == last_dir.value();
            });
        }

        return dirs;
    }

    [[nodiscard]] bool open_tunnel(int i, int j, Direction dir) {
        const auto this_r = (i * 2) + 1;
        const auto this_c = (j * 2) + 1;
        const auto next_r = ((i + detail::dir_i(dir)) * 2) + 1;
        const auto next_c = ((j + detail::dir_j(dir)) * 2) + 1;
        const auto mid_r = (this_r + next_r) / 2;
        const auto mid_c = (this_c + next_c) / 2;

        if (sound_tunnel(mid_r, mid_c, next_r, next_c)) {
            return delve_tunnel(this_r, this_c, next_r, next_c);
        }
        return false;
    }

    [[nodiscard]] bool sound_tunnel(int mid_r, int mid_c, int next_r, int next_c) const {
        if (next_r < 0 || next_r > n_rows) {
            return false;
        }
        if (next_c < 0 || next_c > n_cols) {
            return false;
        }
        const auto r1 = std::min(mid_r, next_r);
        const auto r2 = std::max(mid_r, next_r);
        const auto c1 = std::min(mid_c, next_c);
        const auto c2 = std::max(mid_c, next_c);

        for (auto r = r1; r <= r2; r++) {
            for (auto c = c1; c <= c2; c++) {
                if (cell(r, c).isBlockedCorridor()) {
                    return false;
                }
            }
        }

        return true;
    }

    bool delve_tunnel(int this_r, int this_c, int next_r, int next_c) {
        const auto r1 = std::min(this_r, next_r);
        const auto r2 = std::max(this_r, next_r);
        const auto c1 = std::min(this_c, next_c);
        const auto c2 = std::max(this_c, next_c);

        for (auto r = r1; r <= r2; r++) {
            for (auto c = c1; c <= c2; c++) {
                cell(r, c).removeType(CellType::ENTRANCE);
                cell(r, c).addType(CellType::CORRIDOR);
            }
        }
        return true;
    }

    void emplace_stairs() {
        const auto n = options.add_stairs;
        if (n <= 0) {
            return;
        }
        auto candidates = stair_ends();

        if (candidates.empty()) {
            return;
        }

        for (int i = 0; i < n && !candidates.empty(); i++) {
            auto stair = pop_random(candidates);

            const auto r = stair.row;
            const auto c = stair.col;
            const auto type = (i < 2) ? i : random_int(2);

            if (type == 0) {
                cell(r, c).addType(CellType::STAIR_DN);
                cell(r, c).setLabel("d");
                stair.key = "down";
            } else {
                cell(r, c).addType(CellType::STAIR_UP);
                cell(r, c).setLabel("u");
                stair.key = "up";
            }
            stairs.push_back(stair);
        }
    }

    [[nodiscard]] bool check_tunnel(int r, int c, const detail::TunnelOffsets &check) const {
        for (const auto &p: check.corridor) {
            if (!cell(r + p[0], c + p[1]).hasType(CellType::CORRIDOR)) {
                return false;
            }
        }
        for (const auto &p: check.walled) {
            if (cell(r + p[0], c + p[1]).isOpenspace()) {
                return false;
            }
        }
        return true;
    }

    [[nodiscard]] std::vector<Stairs> stair_ends() const {
        std::vector<Stairs> candidates;

        for (auto i = 0; i < n_i; i++) {
            const auto r = (i * 2) + 1;
            for (auto j = 0; j < n_j; j++) {
                const auto c = (j * 2) + 1;

                if (!cell(r, c).hasType(CellType::CORRIDOR) || cell(r, c).isStairs()) {
                    continue;
                }
                for (auto dir: detail::DIRECTIONS) {
                    const auto &dir_value = detail::STAIR_END[detail::direction_index(dir)];
                    if (check_tunnel(r, c, dir_value)) {
                        Stairs end;
                        end.row = r;
                        end.col = c;

                        end.next_row = end.row + dir_value.next[0][0];
                        end.next_col = end.col + dir_value.next[0][1];

                        candidates.push_back(end);
                        break;
                    }
                }
            }
        }
        return candidates;
    }

    void collapse(int r, int c) {
        if (!(cell(r, c).isOpenspace())) {
            return;
        }
        for (auto dir: detail::DIRECTIONS) {
            const auto &value = detail::CLOSE_END[detail::direction_index(dir)];
            if (check_tunnel(r, c, value)) {
                for (const auto &p: value.close) {
                    cell(r + p[0], c + p[1]).clearTypes();
                }
                for (const auto &p: value.open) {
                    cell(r + p[0], c + p[1]).addType(CellType::CORRIDOR);
                }
                for (const auto &p: value.recurse) {
                    collapse(r + p[0], c + p[1]);
                }
            }
        }
    }

    void collapse_tunnels(int p) {
        if (!p) {
            return;
        }
        const auto all = p == 100;

        for (int i = 0; i < n_i; i++) {
            const auto r = (i * 2) + 1;
            for (int j = 0; j < n_j; j++) {
                const auto c = (j * 2) + 1;

                if (!(cell(r, c).isOpenspace())) {
                    continue;
                }
                if (cell(r, c).isStairs()) {
                    continue;
                }
                if (!(all || random_int(100) < p)) {
                    continue;
                }
                collapse(r, c);
            }
        }
    }

    void remove_deadends() {
        collapse_tunnels(options.remove_deadends);
    }

    void fix_doors() {
        std::set<std::pair<int, int>> fixed;

        for (auto &room_data: rooms) {
            std::set<Direction> dirs;
            for (const auto &door_entry: room_data.door) {
                dirs.insert(door_entry.first);
            }
            for (const auto &dir: dirs) {
                std::vector<Door> shiny;
                auto range = room_data.door.equal_range(dir);
                for (auto it = range.first; it != range.second; it++) {
                    const auto &door = it->second;
                    const auto door_r = door.row;
                    const auto door_c = door.col;
                    const auto &door_cell = cell(door_r, door_c);
                    if (!(door_cell.isOpenspace())) {
                        continue;
                    }

                    if (vstd::ctn(fixed, std::make_pair(door_r, door_c))) {
                        shiny.push_back(door);
                    } else {
                        if (auto out_id = door.out_id) {
                            const auto out_dir = detail::opposite(dir);
                            room_by_id(out_id).door.insert(std::make_pair(out_dir, door));
                        }
                        shiny.push_back(door);
                        fixed.insert(std::make_pair(door_r, door_c));
                    }
                }
                if (!shiny.empty()) {
                    room_data.door.erase(dir);
                    for (const auto &shiny_door: shiny) {
                        room_data.door.insert(std::make_pair(dir, shiny_door));
                    }
                    doors.push_back(std::move(shiny));
                } else {
                    room_data.door.erase(dir);
                }
            }
        }
    }

    void empty_blocks() {
        for (auto &current_cell: cells) {
            if (current_cell.hasType(CellType::BLOCKED)) {
                current_cell.clearTypes();
            }
        }
    }

    void clean_dungeon() {
        if (options.remove_deadends) {
            remove_deadends();
        }
        fix_doors();
        empty_blocks();
    }
};

[[nodiscard]] inline Dungeon create_dungeon(Options options, std::mt19937 &rng) {
    if (const auto validation = validate_options(options)) {
        throw std::invalid_argument(validation->message);
    }

    Dungeon dungeon(std::move(options), rng);

    dungeon.init_cells();
    dungeon.emplace_rooms();
    dungeon.open_rooms();
    dungeon.label_rooms();
    dungeon.corridors();

    if (dungeon.options.add_stairs) {
        dungeon.emplace_stairs();
    }
    dungeon.clean_dungeon();

    return dungeon;
}

[[nodiscard]] inline Dungeon create_dungeon(Options options) {
    return create_dungeon(std::move(options), vstd::rng());
}

} // namespace rdg
