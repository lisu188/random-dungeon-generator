#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <random>
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

enum class DoorKind : std::uint8_t {
    None,
    Arch,
    Open,
    Locked,
    Trapped,
    Secret,
    Portcullis
};

enum class StairKind : std::uint8_t {
    None,
    Down,
    Up
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

[[nodiscard]] inline std::string_view key(DoorKind kind) {
    switch (kind) {
        case DoorKind::Arch:
            return "arch";
        case DoorKind::Open:
            return "open";
        case DoorKind::Locked:
            return "lock";
        case DoorKind::Trapped:
            return "trap";
        case DoorKind::Secret:
            return "secret";
        case DoorKind::Portcullis:
            return "portc";
        case DoorKind::None:
            return "";
    }
    return "";
}

[[nodiscard]] inline std::string_view to_string(DoorKind kind) {
    switch (kind) {
        case DoorKind::Arch:
            return "Archway";
        case DoorKind::Open:
            return "Unlocked Door";
        case DoorKind::Locked:
            return "Locked Door";
        case DoorKind::Trapped:
            return "Trapped Door";
        case DoorKind::Secret:
            return "Secret Door";
        case DoorKind::Portcullis:
            return "Portcullis";
        case DoorKind::None:
            return "";
    }
    return "";
}

[[nodiscard]] inline std::string_view key(StairKind kind) {
    switch (kind) {
        case StairKind::Down:
            return "down";
        case StairKind::Up:
            return "up";
        case StairKind::None:
            return "";
    }
    return "";
}

class Cell {
    std::uint16_t types = 0;
    int room_id = 0;
    char label = '\0';

    [[nodiscard]] static constexpr std::uint16_t type_bit(CellType type) {
        return static_cast<std::uint16_t>(1u << static_cast<unsigned>(type));
    }

    static constexpr std::uint16_t BLOCKED_BIT =
            static_cast<std::uint16_t>(1u << static_cast<unsigned>(CellType::BLOCKED));
    static constexpr std::uint16_t ROOM_BIT = static_cast<std::uint16_t>(1u << static_cast<unsigned>(CellType::ROOM));
    static constexpr std::uint16_t CORRIDOR_BIT =
            static_cast<std::uint16_t>(1u << static_cast<unsigned>(CellType::CORRIDOR));
    static constexpr std::uint16_t PERIMETER_BIT =
            static_cast<std::uint16_t>(1u << static_cast<unsigned>(CellType::PERIMETER));
    static constexpr std::uint16_t ENTRANCE_BIT =
            static_cast<std::uint16_t>(1u << static_cast<unsigned>(CellType::ENTRANCE));
    static constexpr std::uint16_t ARCH_BIT = static_cast<std::uint16_t>(1u << static_cast<unsigned>(CellType::ARCH));
    static constexpr std::uint16_t DOOR_BIT = static_cast<std::uint16_t>(1u << static_cast<unsigned>(CellType::DOOR));
    static constexpr std::uint16_t LOCKED_BIT =
            static_cast<std::uint16_t>(1u << static_cast<unsigned>(CellType::LOCKED));
    static constexpr std::uint16_t TRAPPED_BIT =
            static_cast<std::uint16_t>(1u << static_cast<unsigned>(CellType::TRAPPED));
    static constexpr std::uint16_t SECRET_BIT =
            static_cast<std::uint16_t>(1u << static_cast<unsigned>(CellType::SECRET));
    static constexpr std::uint16_t PORTC_BIT = static_cast<std::uint16_t>(1u << static_cast<unsigned>(CellType::PORTC));
    static constexpr std::uint16_t STAIR_DN_BIT =
            static_cast<std::uint16_t>(1u << static_cast<unsigned>(CellType::STAIR_DN));
    static constexpr std::uint16_t STAIR_UP_BIT =
            static_cast<std::uint16_t>(1u << static_cast<unsigned>(CellType::STAIR_UP));

    static constexpr std::uint16_t DOORSPACE_BITS =
            ARCH_BIT | DOOR_BIT | LOCKED_BIT | TRAPPED_BIT | SECRET_BIT | PORTC_BIT;
    static constexpr std::uint16_t ESPACE_BITS = ENTRANCE_BIT | DOORSPACE_BITS;
    static constexpr std::uint16_t STAIR_BITS = STAIR_DN_BIT | STAIR_UP_BIT;

    [[nodiscard]] bool hasAny(std::uint16_t mask) const {
        return (types & mask) != 0;
    }

public:
    void setType(CellType type) {
        types = type_bit(type);
    }

    [[nodiscard]] bool isBlockedRoom() const {
        return hasAny(BLOCKED_BIT | ROOM_BIT);
    }

    [[nodiscard]] bool isBlockedCorridor() const {
        return hasAny(BLOCKED_BIT | PERIMETER_BIT | CORRIDOR_BIT);
    }

    [[nodiscard]] bool isBlockedDoor() const {
        return hasAny(BLOCKED_BIT | DOORSPACE_BITS);
    }

    [[nodiscard]] bool hasLabel() const {
        return label != '\0';
    }

    [[nodiscard]] std::string_view getLabel() const {
        if (!hasLabel()) {
            return {};
        }
        return std::string_view(&label, 1);
    }

    [[nodiscard]] bool isEspace() const {
        return hasAny(ESPACE_BITS) || hasLabel();
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
        return hasAny(ROOM_BIT | CORRIDOR_BIT);
    }

    [[nodiscard]] bool isDoorspace() const {
        return hasAny(DOORSPACE_BITS);
    }

    [[nodiscard]] bool isStairs() const {
        return hasAny(STAIR_BITS);
    }

    void setRoomId(int room_id) {
        this->room_id = room_id;
    }

    [[nodiscard]] int getRoomId() const {
        return room_id;
    }

    void setLabel(char label) {
        this->label = label;
    }

    void setLabel(std::string_view label) {
        this->label = label.empty() ? '\0' : label.front();
    }

    void clearTypes() {
        types = 0;
    }

    void clearLabel() {
        label = '\0';
    }

    void clearEspace() {
        clearLabel();
        types &= static_cast<std::uint16_t>(~ESPACE_BITS);
    }
};

struct Stairs {
    int row = 0;
    int col = 0;
    int next_row = 0;
    int next_col = 0;
    StairKind kind = StairKind::None;

    [[nodiscard]] std::string_view getKey() const {
        return key(kind);
    }
};

struct Door {
    int row = 0;
    int col = 0;
    DoorKind kind = DoorKind::None;
    int out_id = 0;

    [[nodiscard]] std::string_view getKey() const {
        return key(kind);
    }

    [[nodiscard]] std::string_view getType() const {
        return to_string(kind);
    }
};

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

    std::array<std::vector<Door>, 4> door;
};

struct Options {
    int n_rows = 33; // must be an odd number
    int n_cols = 33; // must be an odd number
    DungeonLayout dungeon_layout = DungeonLayout::None;
    int room_min = 3; // minimum room size
    int room_max = 9; // maximum room size
    RoomLayout room_layout = RoomLayout::Scattered;
    CorridorLayout corridor_layout = CorridorLayout::LABYRINTH;
    int remove_deadends = 100; // percentage
    int add_stairs = 2;        // number of stairs
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
    std::span<const Offset> next;
};

inline constexpr std::array<Direction, 4> DIRECTIONS = {Direction::NORTH, Direction::SOUTH, Direction::EAST,
                                                        Direction::WEST};

inline constexpr std::array<int, 4> DI = {-1, 1, 0, 0};
inline constexpr std::array<int, 4> DJ = {0, 0, 1, -1};

inline constexpr std::array<Direction, 4> OPPOSITE = {Direction::SOUTH, Direction::NORTH, Direction::WEST,
                                                      Direction::EAST};

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

inline constexpr Mask BOX_MASK = {{{{1, 1, 1}}, {{1, 0, 1}}, {{1, 1, 1}}}};

inline constexpr Mask CROSS_MASK = {{{{0, 1, 0}}, {{1, 1, 1}}, {{0, 1, 0}}}};

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

inline constexpr std::array<Offset, 7> STAIR_N_WALLED = {Offset{1, -1}, Offset{0, -1}, Offset{-1, -1}, Offset{-1, 0},
                                                         Offset{-1, 1}, Offset{0, 1},  Offset{1, 1}};
inline constexpr std::array<Offset, 3> STAIR_N_CORRIDOR = {Offset{0, 0}, Offset{1, 0}, Offset{2, 0}};
inline constexpr std::array<Offset, 1> STAIR_N_NEXT = {Offset{1, 0}};

inline constexpr std::array<Offset, 7> STAIR_S_WALLED = {Offset{-1, -1}, Offset{0, -1}, Offset{1, -1}, Offset{1, 0},
                                                         Offset{1, 1},   Offset{0, 1},  Offset{-1, 1}};
inline constexpr std::array<Offset, 3> STAIR_S_CORRIDOR = {Offset{0, 0}, Offset{-1, 0}, Offset{-2, 0}};
inline constexpr std::array<Offset, 1> STAIR_S_NEXT = {Offset{-1, 0}};

inline constexpr std::array<Offset, 7> STAIR_E_WALLED = {Offset{-1, -1}, Offset{-1, 0}, Offset{-1, 1}, Offset{0, 1},
                                                         Offset{1, 1},   Offset{1, 0},  Offset{1, -1}};
inline constexpr std::array<Offset, 3> STAIR_E_CORRIDOR = {Offset{0, 0}, Offset{0, -1}, Offset{0, -2}};
inline constexpr std::array<Offset, 1> STAIR_E_NEXT = {Offset{0, -1}};

inline constexpr std::array<Offset, 7> STAIR_W_WALLED = {Offset{-1, 1}, Offset{-1, 0}, Offset{-1, -1}, Offset{0, -1},
                                                         Offset{1, -1}, Offset{1, 0},  Offset{1, 1}};
inline constexpr std::array<Offset, 3> STAIR_W_CORRIDOR = {Offset{0, 0}, Offset{0, 1}, Offset{0, 2}};
inline constexpr std::array<Offset, 1> STAIR_W_NEXT = {Offset{0, 1}};

inline constexpr std::array<TunnelOffsets, 4> STAIR_END = {{{STAIR_N_WALLED, STAIR_N_CORRIDOR, STAIR_N_NEXT},
                                                            {STAIR_S_WALLED, STAIR_S_CORRIDOR, STAIR_S_NEXT},
                                                            {STAIR_E_WALLED, STAIR_E_CORRIDOR, STAIR_E_NEXT},
                                                            {STAIR_W_WALLED, STAIR_W_CORRIDOR, STAIR_W_NEXT}}};

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
        return cell_rows;
    }

    [[nodiscard]] int colCount() const {
        return cell_cols;
    }

    [[nodiscard]] const Cell &cellAt(int row, int col) const {
        return cell(row, col);
    }

private:
    struct CorridorNode {
        int i = 0;
        int j = 0;
    };

    struct TunnelStep {
        int i = 0;
        int j = 0;
        Direction last_dir = Direction::NORTH;
        bool has_last_dir = false;
    };

    Options options;
    std::mt19937 &rng;
    std::uint64_t random_state;
    std::vector<Cell> cells;
    std::vector<Room> rooms;
    std::vector<Stairs> stairs;
    std::vector<std::vector<Door>> doors;
    std::vector<std::uint8_t> corridor_degrees;
    std::vector<std::uint8_t> corridor_links;
    std::vector<std::uint8_t> corridor_node_seen;
    std::vector<CorridorNode> corridor_nodes;
    std::vector<TunnelStep> tunnel_stack;
    std::vector<detail::Sill> door_sill_scratch;
    std::vector<std::size_t> blocked_cells;

    const int n_i;
    const int n_j;
    const int n_rows;
    const int n_cols;
    const int cell_rows;
    const int cell_cols;
    const int max_row;
    const int max_col;
    const int room_base;
    const int room_radix;
    int n_rooms = 0;
    int last_room_id = 0;

    explicit Dungeon(Options options, std::mt19937 &rng)
        : options(std::move(options)), rng(rng), random_state(seed_random_state(rng)), n_i(this->options.n_rows / 2),
          n_j(this->options.n_cols / 2), n_rows(n_i * 2), n_cols(n_j * 2), cell_rows(n_rows + 1), cell_cols(n_cols + 1),
          max_row(n_rows - 1), max_col(n_cols - 1), room_base((this->options.room_min + 1) / 2),
          room_radix(((this->options.room_max - this->options.room_min) / 2) + 1) {
    }

    [[nodiscard]] static std::uint64_t seed_random_state(std::mt19937 &rng) {
        const auto high = static_cast<std::uint64_t>(rng()) << 32u;
        const auto low = static_cast<std::uint64_t>(rng());
        return high ^ low ^ 0x9E3779B97F4A7C15ull;
    }

    [[nodiscard]] std::uint32_t random_u32() {
        random_state = (random_state * 6364136223846793005ull) + 1442695040888963407ull;
        return static_cast<std::uint32_t>(random_state >> 32u);
    }

    [[nodiscard]] std::size_t index(int row, int col) const {
        return static_cast<std::size_t>(row * cell_cols + col);
    }

    [[nodiscard]] std::size_t node_index(int i, int j) const {
        return static_cast<std::size_t>(i * n_j + j);
    }

    [[nodiscard]] bool is_node(int i, int j) const {
        return i >= 0 && i < n_i && j >= 0 && j < n_j;
    }

    [[nodiscard]] Cell &cell(int row, int col) {
        return cells[index(row, col)];
    }

    [[nodiscard]] const Cell &cell(int row, int col) const {
        return cells[index(row, col)];
    }

    [[nodiscard]] int node_row(int i) const {
        return (i * 2) + 1;
    }

    [[nodiscard]] int node_col(int j) const {
        return (j * 2) + 1;
    }

    [[nodiscard]] Cell &node_cell(CorridorNode node) {
        return cell(node_row(node.i), node_col(node.j));
    }

    [[nodiscard]] const Cell &node_cell(CorridorNode node) const {
        return cell(node_row(node.i), node_col(node.j));
    }

    [[nodiscard]] Room &room_by_id(int id) {
        return rooms[static_cast<std::size_t>(id - 1)];
    }

    [[nodiscard]] const Room &room_by_id(int id) const {
        return rooms[static_cast<std::size_t>(id - 1)];
    }

    [[nodiscard]] static constexpr std::uint8_t direction_bit(Direction dir) {
        return static_cast<std::uint8_t>(1u << detail::direction_index(dir));
    }

    [[nodiscard]] static Direction first_link_direction(std::uint8_t links) {
        if (links & direction_bit(Direction::NORTH)) {
            return Direction::NORTH;
        }
        if (links & direction_bit(Direction::SOUTH)) {
            return Direction::SOUTH;
        }
        if (links & direction_bit(Direction::EAST)) {
            return Direction::EAST;
        }
        return Direction::WEST;
    }

    [[nodiscard]] static CorridorNode corridor_neighbor(CorridorNode node, Direction dir) {
        return {node.i + detail::dir_i(dir), node.j + detail::dir_j(dir)};
    }

    void block_cell(int row, int col) {
        const auto target_index = index(row, col);
        cells[target_index].setType(CellType::BLOCKED);
        blocked_cells.push_back(target_index);
    }

    void reset_corridor_graph() {
        corridor_degrees.assign(static_cast<std::size_t>(n_i * n_j), 0);
        corridor_links.assign(static_cast<std::size_t>(n_i * n_j), 0);
        corridor_node_seen.assign(static_cast<std::size_t>(n_i * n_j), 0);
        corridor_nodes.clear();
        corridor_nodes.reserve(static_cast<std::size_t>(n_i * n_j));
        tunnel_stack.reserve(static_cast<std::size_t>(n_i * n_j));
    }

    void track_corridor_node(CorridorNode node) {
        const auto index = node_index(node.i, node.j);
        if (!corridor_node_seen[index]) {
            corridor_node_seen[index] = 1;
            corridor_nodes.push_back(node);
        }
    }

    void add_corridor_edge(CorridorNode from, CorridorNode to, Direction dir) {
        if (!is_node(from.i, from.j) || !is_node(to.i, to.j)) {
            return;
        }
        track_corridor_node(from);
        track_corridor_node(to);
        const auto from_index = node_index(from.i, from.j);
        const auto to_index = node_index(to.i, to.j);
        auto &from_degree = corridor_degrees[from_index];
        auto &to_degree = corridor_degrees[to_index];
        if (from_degree < 255) {
            from_degree++;
        }
        if (to_degree < 255) {
            to_degree++;
        }
        corridor_links[from_index] |= direction_bit(dir);
        corridor_links[to_index] |= direction_bit(detail::opposite(dir));
    }

    [[nodiscard]] int corridor_degree(CorridorNode node) const {
        return corridor_degrees[node_index(node.i, node.j)];
    }

    [[nodiscard]] std::size_t connection_index(int first_room, int second_room) const {
        const auto low = static_cast<std::size_t>(std::min(first_room, second_room));
        const auto high = static_cast<std::size_t>(std::max(first_room, second_room));
        return (low * static_cast<std::size_t>(n_rooms + 1)) + high;
    }

    [[nodiscard]] int random_int(int max_exclusive) {
        if (max_exclusive <= 0) {
            return 0;
        }
        return static_cast<int>(random_u32() % static_cast<std::uint32_t>(max_exclusive));
    }

    [[nodiscard]] int random_int(int min_inclusive, int max_inclusive) {
        if (max_inclusive < min_inclusive) {
            return min_inclusive;
        }
        return min_inclusive + random_int((max_inclusive - min_inclusive) + 1);
    }

    template <typename E> [[nodiscard]] E pop_random(std::vector<E> &items) {
        auto index = static_cast<std::size_t>(random_int(static_cast<int>(items.size())));
        E value = std::move(items[index]);
        if (index != items.size() - 1) {
            items[index] = std::move(items.back());
        }
        items.pop_back();
        return value;
    }

    void init_cells() {
        cells.resize(static_cast<std::size_t>(cell_rows * cell_cols));
        blocked_cells.clear();
        if (options.dungeon_layout != DungeonLayout::None) {
            blocked_cells.reserve(cells.size());
        }

        if (const auto *mask = detail::mask_for(options.dungeon_layout)) {
            mask_cells(*mask);
        } else if (options.dungeon_layout == DungeonLayout::Round) {
            round_mask();
        }
    }

    void mask_cells(const detail::Mask &mask) {
        const auto mask_rows = static_cast<int>(mask.size());
        const auto mask_cols = static_cast<int>(mask.front().size());

        for (int r = 0; r < n_rows; r++) {
            for (int c = 0; c < n_cols; c++) {
                const auto mask_r = static_cast<std::size_t>((r * mask_rows) / cell_rows);
                const auto mask_c = static_cast<std::size_t>((c * mask_cols) / cell_cols);
                if (!mask[mask_r][mask_c]) {
                    block_cell(r, c);
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
                    block_cell(r, c);
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
        auto &room = rooms.emplace_back();
        room.id = room_id;
        room.row = r1;
        room.col = c1;
        room.north = r1;
        room.south = r2;
        room.west = c1;
        room.east = c2;
        room.height = h;
        room.width = w;
        room.area = h * w;

        for (int r = r1 - 1; r <= r2 + 1; r++) {
            if (!(cell(r, c1 - 1).hasType(CellType::ROOM) || cell(r, c1 - 1).hasType(CellType::ENTRANCE))) {
                cell(r, c1 - 1).addType(CellType::PERIMETER);
            }
            if (!(cell(r, c2 + 1).hasType(CellType::ROOM) || cell(r, c2 + 1).hasType(CellType::ENTRANCE))) {
                cell(r, c2 + 1).addType(CellType::PERIMETER);
            }
        }
        for (int c = c1 - 1; c <= c2 + 1; c++) {
            if (!(cell(r1 - 1, c).hasType(CellType::ROOM) || cell(r1 - 1, c).hasType(CellType::ENTRANCE))) {
                cell(r1 - 1, c).addType(CellType::PERIMETER);
            }
            if (!(cell(r2 + 1, c).hasType(CellType::ROOM) || cell(r2 + 1, c).hasType(CellType::ENTRANCE))) {
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
                               room_j < 0 ? random_int(n_j - width) : room_j, height, width);
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

    void open_room(Room &room, std::vector<std::uint8_t> &connected) {
        auto &candidates = door_sill_scratch;
        door_sills(room, candidates);
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
                const auto connect = connection_index(room.id, out_id);

                if (connected[connect]) {
                    n_opens--;
                    continue;
                }

                connected[connect] = 1;
            }
            const auto open_r = sill.sill_r;
            const auto open_c = sill.sill_c;
            const auto open_dir = sill.dir;
            const auto open_di = detail::dir_i(open_dir);
            const auto open_dj = detail::dir_j(open_dir);

            for (auto x = 0; x < 3; x++) {
                const auto r = open_r + (open_di * x);
                const auto c = open_c + (open_dj * x);

                cell(r, c).removeType(CellType::PERIMETER);
                cell(r, c).addType(CellType::ENTRANCE);
            }

            Door door;
            door.row = door_r;
            door.col = door_c;
            apply_door_type(generate_door_kind(), door);

            door.out_id = out_id;
            if (out_id) {
                room.door[detail::direction_index(open_dir)].push_back(door);
            }
        }
    }

    void apply_door_type(DoorKind door_kind, Door &door) {
        door.kind = door_kind;
        switch (door_kind) {
            case DoorKind::Arch:
                cell(door.row, door.col).addType(CellType::ARCH);
                cell(door.row, door.col).setLabel('a');
                break;
            case DoorKind::Open:
                cell(door.row, door.col).addType(CellType::DOOR);
                cell(door.row, door.col).setLabel('o');
                break;
            case DoorKind::Locked:
                cell(door.row, door.col).addType(CellType::LOCKED);
                cell(door.row, door.col).setLabel('x');
                break;
            case DoorKind::Trapped:
                cell(door.row, door.col).addType(CellType::TRAPPED);
                cell(door.row, door.col).setLabel('t');
                break;
            case DoorKind::Secret:
                cell(door.row, door.col).addType(CellType::SECRET);
                cell(door.row, door.col).setLabel('s');
                break;
            case DoorKind::Portcullis:
                cell(door.row, door.col).addType(CellType::PORTC);
                cell(door.row, door.col).setLabel('p');
                break;
            case DoorKind::None:
                break;
        }
    }

    [[nodiscard]] DoorKind generate_door_kind() {
        const auto i = random_int(110);

        if (i < 15) {
            return DoorKind::Arch;
        }
        if (i < 60) {
            return DoorKind::Open;
        }
        if (i < 75) {
            return DoorKind::Locked;
        }
        if (i < 90) {
            return DoorKind::Trapped;
        }
        if (i < 100) {
            return DoorKind::Secret;
        }
        return DoorKind::Portcullis;
    }

    [[nodiscard]] int alloc_opens(const Room &room) {
        const auto room_h = ((room.south - room.north) / 2) + 1;
        const auto room_w = ((room.east - room.west) / 2) + 1;
        const auto flumph = static_cast<int>(std::sqrt(static_cast<double>(room_w * room_h)));
        return flumph + random_int(flumph);
    }

    void append_sill(int sill_r, int sill_c, Direction dir, std::vector<detail::Sill> &sills) const {
        const auto di = detail::dir_i(dir);
        const auto dj = detail::dir_j(dir);
        const auto door_r = sill_r + di;
        const auto door_c = sill_c + dj;
        const auto &door_cell = cell(door_r, door_c);
        if (!(door_cell.hasType(CellType::PERIMETER))) {
            return;
        }
        if (door_cell.isBlockedDoor()) {
            return;
        }
        const auto out_r = door_r + di;
        const auto out_c = door_c + dj;
        const auto &out_cell = cell(out_r, out_c);
        if (out_cell.hasType(CellType::BLOCKED)) {
            return;
        }
        auto out_id = 0;
        if (out_cell.hasType(CellType::ROOM)) {
            out_id = out_cell.getRoomId();
        }
        sills.push_back({sill_r, sill_c, dir, door_r, door_c, out_id});
    }

    void door_sills(const Room &room, std::vector<detail::Sill> &sills) const {
        sills.clear();
        sills.reserve(static_cast<std::size_t>(((room.east - room.west) / 2 + 1) * 2 +
                                               ((room.south - room.north) / 2 + 1) * 2));
        if (room.north >= 3) {
            for (int c = room.west; c <= room.east; c += 2) {
                append_sill(room.north, c, Direction::NORTH, sills);
            }
        }
        if (room.south <= n_rows - 3) {
            for (int c = room.west; c <= room.east; c += 2) {
                append_sill(room.south, c, Direction::SOUTH, sills);
            }
        }
        if (room.west >= 3) {
            for (int r = room.north; r <= room.south; r += 2) {
                append_sill(r, room.west, Direction::WEST, sills);
            }
        }
        if (room.east <= n_cols - 3) {
            for (int r = room.north; r <= room.south; r += 2) {
                append_sill(r, room.east, Direction::EAST, sills);
            }
        }
    }

    void open_rooms() {
        std::vector<std::uint8_t> connected(static_cast<std::size_t>((n_rooms + 1) * (n_rooms + 1)), 0);
        for (int id = 1; id <= n_rooms; id++) {
            open_room(room_by_id(id), connected);
        }
    }

    void label_rooms() {
        for (const auto &room: rooms) {
            std::array<char, 3> label{};
            std::size_t len = 0;
            if (room.id >= 100) {
                label[len++] = static_cast<char>('0' + (room.id / 100));
            }
            if (room.id >= 10) {
                label[len++] = static_cast<char>('0' + ((room.id / 10) % 10));
            }
            label[len++] = static_cast<char>('0' + (room.id % 10));
            const auto label_r = int((room.north + room.south) / 2);
            const auto label_c = int((room.west + room.east - static_cast<int>(len)) / 2) + 1;

            for (decltype(len) c = 0; c < len; c++) {
                cell(label_r, label_c + static_cast<int>(c)).setLabel(label[c]);
            }
        }
    }

    void corridors() {
        reset_corridor_graph();
        for (auto i = 1; i < n_i; i++) {
            const auto r = node_row(i);
            for (auto j = 1; j < n_j; j++) {
                const auto c = node_col(j);

                if (cell(r, c).hasType(CellType::CORRIDOR)) {
                    continue;
                }
                tunnel(i, j);
            }
        }
    }

    void tunnel(int start_i, int start_j, std::optional<Direction> last_dir = std::nullopt) {
        auto &args = tunnel_stack;
        args.clear();
        args.push_back({start_i, start_j, last_dir.value_or(Direction::NORTH), last_dir.has_value()});
        while (!args.empty()) {
            const auto arg = args.back();
            args.pop_back();
            auto dirs = tunnel_dirs(arg.last_dir, arg.has_last_dir);
            for (const auto &dir: dirs) {
                if (open_tunnel(arg.i, arg.j, dir)) {
                    const auto next_i = arg.i + detail::dir_i(dir);
                    const auto next_j = arg.j + detail::dir_j(dir);

                    args.push_back({next_i, next_j, dir, true});
                }
            }
        }
    }

    [[nodiscard]] std::array<Direction, 4> tunnel_dirs(Direction last_dir, bool has_last_dir) {
        const auto p = static_cast<int>(options.corridor_layout);
        auto dirs = detail::DIRECTIONS;
        for (std::size_t i = dirs.size() - 1; i > 0; --i) {
            const auto j = static_cast<std::size_t>(random_int(static_cast<int>(i + 1)));
            std::swap(dirs[i], dirs[j]);
        }

        if (has_last_dir && p > 0 && random_int(100) < p) {
            for (std::size_t i = 0; i < dirs.size(); ++i) {
                if (dirs[i] == last_dir) {
                    std::swap(dirs[0], dirs[i]);
                    break;
                }
            }
        }

        return dirs;
    }

    [[nodiscard]] bool open_tunnel(int i, int j, Direction dir) {
        const auto di = detail::dir_i(dir);
        const auto dj = detail::dir_j(dir);
        const auto next_i = i + di;
        const auto next_j = j + dj;
        const auto this_r = node_row(i);
        const auto this_c = node_col(j);
        const auto mid_r = this_r + di;
        const auto mid_c = this_c + dj;
        const auto next_r = this_r + (di * 2);
        const auto next_c = this_c + (dj * 2);

        if (sound_tunnel(mid_r, mid_c, next_r, next_c)) {
            carve_corridor_cell(this_r, this_c);
            carve_corridor_cell(mid_r, mid_c);
            carve_corridor_cell(next_r, next_c);
            add_corridor_edge({i, j}, {next_i, next_j}, dir);
            return true;
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
        return !cell(mid_r, mid_c).isBlockedCorridor() && !cell(next_r, next_c).isBlockedCorridor();
    }

    void carve_corridor_cell(int row, int col) {
        cell(row, col).removeType(CellType::ENTRANCE);
        cell(row, col).addType(CellType::CORRIDOR);
    }

    void emplace_stairs() {
        const auto n = options.add_stairs;
        if (n <= 0) {
            return;
        }
        stairs.reserve(static_cast<std::size_t>(n));
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
                cell(r, c).setLabel('d');
                stair.kind = StairKind::Down;
            } else {
                cell(r, c).addType(CellType::STAIR_UP);
                cell(r, c).setLabel('u');
                stair.kind = StairKind::Up;
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

    [[nodiscard]] const detail::TunnelOffsets &stair_end_for_direction(Direction neighbor_direction) const {
        return detail::STAIR_END[detail::direction_index(detail::opposite(neighbor_direction))];
    }

    [[nodiscard]] std::vector<Stairs> stair_ends() const {
        std::vector<Stairs> candidates;
        candidates.reserve(corridor_nodes.size() / 4);

        for (const auto node: corridor_nodes) {
            if (corridor_degree(node) != 1) {
                continue;
            }
            const auto r = node_row(node.i);
            const auto c = node_col(node.j);

            if (!cell(r, c).hasType(CellType::CORRIDOR) || cell(r, c).isStairs()) {
                continue;
            }
            const auto neighbor_direction = sole_corridor_direction(node);
            if (!neighbor_direction) {
                continue;
            }
            const auto &dir_value = stair_end_for_direction(neighbor_direction.value());
            if (!check_tunnel(r, c, dir_value)) {
                continue;
            }

            Stairs end;
            end.row = r;
            end.col = c;
            end.next_row = end.row + dir_value.next[0][0];
            end.next_col = end.col + dir_value.next[0][1];
            candidates.push_back(end);
        }
        return candidates;
    }

    [[nodiscard]] std::optional<Direction> sole_corridor_direction(CorridorNode node) const {
        const auto links = corridor_links[node_index(node.i, node.j)];
        if (!links) {
            return {};
        }
        return first_link_direction(links);
    }

    [[nodiscard]] bool is_dead_end_node(CorridorNode node) const {
        const auto &target = node_cell(node);
        if (!target.hasType(CellType::CORRIDOR) || target.hasType(CellType::ROOM) || target.isStairs()) {
            return false;
        }
        const auto degree = corridor_degree(node);
        if (degree > 1) {
            return false;
        }
        if (degree == 1) {
            const auto dir = sole_corridor_direction(node);
            if (dir && node_cell(corridor_neighbor(node, dir.value())).isStairs()) {
                return false;
            }
        }
        return true;
    }

    [[nodiscard]] std::optional<CorridorNode> collapse_dead_end_node(CorridorNode node) {
        const auto dir = sole_corridor_direction(node);
        const auto node_row_value = node_row(node.i);
        const auto node_col_value = node_col(node.j);
        const auto node_index_value = node_index(node.i, node.j);

        cell(node_row_value, node_col_value).clearTypes();
        corridor_degrees[node_index_value] = 0;
        corridor_links[node_index_value] = 0;

        if (!dir) {
            return {};
        }

        const auto neighbor = corridor_neighbor(node, dir.value());
        const auto mid_r = node_row_value + detail::dir_i(dir.value());
        const auto mid_c = node_col_value + detail::dir_j(dir.value());
        cell(mid_r, mid_c).clearTypes();

        const auto neighbor_index = node_index(neighbor.i, neighbor.j);
        auto &neighbor_degree = corridor_degrees[neighbor_index];
        if (neighbor_degree > 0) {
            neighbor_degree--;
        }
        corridor_links[neighbor_index] &= static_cast<std::uint8_t>(~direction_bit(detail::opposite(dir.value())));
        return neighbor;
    }

    void collapse_tunnels(int percent) {
        if (!percent) {
            return;
        }
        const auto all = percent == 100;
        std::vector<CorridorNode> pending;
        pending.reserve(corridor_nodes.size());

        for (const auto node: corridor_nodes) {
            if (!is_dead_end_node(node)) {
                continue;
            }
            if (!all && random_int(100) >= percent) {
                continue;
            }
            pending.push_back(node);
        }

        for (std::size_t index = 0; index < pending.size(); ++index) {
            const auto node = pending[index];
            if (!is_dead_end_node(node)) {
                continue;
            }
            const auto neighbor = collapse_dead_end_node(node);
            if (neighbor && is_dead_end_node(neighbor.value())) {
                pending.push_back(neighbor.value());
            }
        }
    }

    void remove_deadends() {
        collapse_tunnels(options.remove_deadends);
    }

    void fix_doors() {
        std::vector<std::uint8_t> fixed(cells.size(), 0);
        doors.reserve(rooms.size() * detail::DIRECTIONS.size());

        for (auto &room_data: rooms) {
            for (std::size_t dir_index = 0; dir_index < room_data.door.size(); ++dir_index) {
                auto &dir_doors = room_data.door[dir_index];
                if (dir_doors.empty()) {
                    continue;
                }
                std::size_t write_index = 0;
                for (const auto &door: dir_doors) {
                    const auto door_r = door.row;
                    const auto door_c = door.col;
                    const auto &door_cell = cell(door_r, door_c);
                    if (!(door_cell.isOpenspace())) {
                        continue;
                    }

                    const auto fixed_index = index(door_r, door_c);
                    if (fixed[fixed_index]) {
                        dir_doors[write_index++] = door;
                    } else {
                        if (auto out_id = door.out_id) {
                            const auto out_dir = detail::opposite(static_cast<Direction>(dir_index));
                            room_by_id(out_id).door[detail::direction_index(out_dir)].push_back(door);
                        }
                        dir_doors[write_index++] = door;
                        fixed[fixed_index] = 1;
                    }
                }
                if (write_index > 0) {
                    dir_doors.resize(write_index);
                    doors.push_back(dir_doors);
                } else {
                    dir_doors.clear();
                }
            }
        }
    }

    void empty_blocks() {
        for (const auto target_index: blocked_cells) {
            auto &current_cell = cells[target_index];
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
