#include <iostream>
#include <utility>
#include <map>
#include <vector>
#include <cmath>
#include <vstd.h>

template<typename T=void>
class rdg {
public:
    enum CellType {
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

    enum CorridorLayout {
        BENT = 50,
        STRAIGHT = 100,
        LABYRINTH = 0
    };
private:
    static std::map<std::string, std::vector<std::vector<int>>> DUNGEON_LAYOUT;
    static std::map<std::string, int> DI;
    static std::map<std::string, int> DJ;
    static std::map<std::string, std::map<std::string, std::vector<std::vector<int>>>> STAIR_END;
    static std::map<std::string, std::map<std::string, std::vector<std::vector<int>>>> CLOSE_END;
    static std::map<std::string, std::string> OPPOSITE;
public:
    class Cell {
        std::set<CellType> types;
        int room_id;
        std::string label;
    public:
        void setType(CellType type) {
            types.clear();
            addType(type);
        }

        bool isBlockedRoom() {
            return hasType(BLOCKED)
                   || hasType(ROOM);
        }

        bool isBlockedCorridor() {
            return hasType(BLOCKED)
                   || hasType(PERIMETER)
                   || hasType(CORRIDOR);
        }

        bool isBlockedDoor() {
            return hasType(BLOCKED)
                   || isDoorspace();
        }

        bool hasLabel() {
            return !label.empty();
        }

        std::string getLabel() {
            return label;
        }

        bool isEspace() {
            return hasType(ENTRANCE)
                   || isDoorspace()
                   || hasLabel();
        }

        void addType(CellType type) {
            types.insert(type);
        }

        void removeType(CellType type) {
            types.erase(type);
        }

        bool hasType(CellType type) const {
            return vstd::ctn(types, type);
        }

        bool isOpenspace() const {
            return hasType(ROOM)
                   || hasType(CORRIDOR);
        }

        bool isDoorspace() {
            return hasType(ARCH)
                   || hasType(DOOR)
                   || hasType(LOCKED)
                   || hasType(TRAPPED)
                   || hasType(SECRET)
                   || hasType(PORTC);
        }

        bool isStairs() {
            return hasType(STAIR_UP)
                   || hasType(STAIR_DN);
        }

        void setRoomId(int room_id) {
            this->room_id = room_id;
        }

        int getRoomId() {
            return room_id;
        }

        void setLabel(std::string label) {
            this->label = std::move(label);
        }

        void clearTypes() {
            types.clear();
        }

        void clearLabel() {
            label = "";
        }

        void clearEspace() {
            clearLabel();
            removeType(ENTRANCE);
            removeType(ARCH);
            removeType(DOOR);
            removeType(LOCKED);
            removeType(TRAPPED);
            removeType(SECRET);
            removeType(PORTC);
        }
    };

    struct Door;

    struct Room {
        int id;
        int row;
        int col;
        int north;
        int south;
        int west;
        int east;
        int height;
        int width;
        int area;

        std::multimap<std::string, Door> door;
    };

    struct Stairs {
        int row;
        int col;
        int next_row;
        int next_col;
        std::string key;
    };

    struct Options {
        int n_rows = 33;  //must be an odd number
        int n_cols = 33;  //must be an odd number
        std::string dungeon_layout = "None";
        int room_min = 3; //minimum rooms size
        int room_max = 9; //maximum rooms size
        std::string room_layout = "Scattered";  //Packed, Scattered
        CorridorLayout corridor_layout = LABYRINTH;
        int remove_deadends = 100;//percentage
        int add_stairs = 2; //number of stairs
        std::string map_style = "Standard";
        int cell_size = 18; //pixels
    };

    struct Sill {
        const int sill_r;
        const int sill_c;
        const std::string dir;
        const int door_r;
        const int door_c;
        const int out_id;
    };

    struct Door {
        int row;
        int col;
        std::string key;
        std::string type;
        int out_id;
    };

    class Dungeon {
        friend Dungeon rdg<T>::create_dungeon(Options options);

    public:
        const auto &getCells() {
            return cells;
        }

        const auto &getStairs() {
            return stairs;
        }

        const auto &getRooms() {
            return rooms;
        }

        const auto &getDoors() {
            return doors;
        }

    private:
        const Options options;
        std::vector<std::vector<Cell>> cells;
        std::map<int, Room> rooms;
        std::list<Stairs> stairs;
        std::list<std::list<Door>> doors;

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

        explicit Dungeon(Options
                         _options) :
                options(std::move(_options)),
                n_i(options.n_rows / 2),
                n_j(options.n_cols / 2),
                n_rows(n_i * 2),
                n_cols(n_j * 2),
                max_row(n_rows - 1),
                max_col(n_cols - 1),
                room_base((options.room_min + 1) / 2),
                room_radix(((options.room_max - options.room_min) / 2) + 1) {}

        void init_cells() {
            for (int r = 0; r <= n_rows; r++) {
                cells.emplace_back();
                for (int c = 0; c <= n_cols; c++) {
                    cells[r].emplace_back();
                }
            }

            auto mask = DUNGEON_LAYOUT.find(options.dungeon_layout);
            if (mask != DUNGEON_LAYOUT.end()) {
                mask_cells((*mask).second);
            } else if (options.dungeon_layout == "Round") {
                round_mask();
            }
        }

        void mask_cells(std::vector<std::vector<int>> mask) {
            double r_x = mask.size() * 1.0 / (n_rows + 1);
            double c_x = mask[0].size() * 1.0 / (n_cols + 1);


            for (int r = 0; r < n_rows; r++) {
                for (int c = 0; c < n_cols; c++) {
                    if (!mask[r * r_x][c * c_x]) {
                        cells[r][c].setType(BLOCKED);
                    }
                }
            }
        }

        void round_mask() {
            int center_r = n_rows / 2;
            int center_c = n_cols / 2;

            for (int r = 0; r < n_rows; r++) {
                for (int c = 0; c < n_cols; c++) {
                    double d = sqrt((r - center_r) * (r - center_r) + (c - center_c) * (c - center_c));
                    if (d > center_c) {
                        cells[r][c].setType(BLOCKED);
                    }
                }
            }
        }

        void emplace_rooms() {
            if (options.room_layout == "Packed") {
                pack_rooms();
            } else {
                scatter_rooms();
            }
        }

        void pack_rooms() {
            for (int i = 0; i < n_i; i++) {
                auto r = (i * 2) + 1;
                for (int j = 0; j < n_j; j++) {
                    auto c = (j * 2) + 1;

                    if (cells[r][c].hasType(ROOM)) {
                        continue;
                    }
                    if ((i == 0 || j == 0) && vstd::rand(0, 1)) {
                        continue;
                    }

                    emplace_room(i, j);
                }
            }
        }

        void emplace_room(int _i = -1, int _j = -1, int _height = -1, int _width = -1) {
            if (n_rooms == 999) {
                return;
            }

            auto[i, j, height, width]=set_room(_i, _j, _height, _width);

            int r1 = i * 2 + 1;
            int c1 = j * 2 + 1;
            int r2 = (i + height) * 2 - 1;
            int c2 = (j + width) * 2 - 1;

            if (r1 < 1 || r2 > max_row) {
                return;
            }
            if (c1 < 1 || c2 > max_col) {
                return;
            }

            auto[hit, blocked] = sound_room(r1, c1, r2, c2);

            if (blocked) {
                return;
            }

            if (!hit.empty()) {
                return;
            }

            int room_id = n_rooms + 1;
            n_rooms = room_id;

            last_room_id = room_id;

            for (int r = r1; r <= r2; r++) {
                for (int c = c1; c <= c2; c++) {
                    if (cells[r][c].hasType(ENTRANCE)) {
                        cells[r][c].clearEspace();
                    } else if (cells[r][c].hasType(PERIMETER)) {
                        cells[r][c].removeType(PERIMETER);
                    }
                    cells[r][c].addType(ROOM);
                    cells[r][c].setRoomId(room_id);
                }
            }

            int h = (r2 - r1) + 1;
            int w = (c2 - c1) + 1;
            Room _room = {room_id, r1, c1, r1, r2, c1, c2, h, w, h * w};
            rooms[room_id] = _room;

            for (int r = r1 - 1; r <= r2 + 1; r++) {
                if (!(cells[r][c1 - 1].hasType(ROOM)
                      || cells[r][c1 - 1].hasType(ENTRANCE))) {
                    cells[r][c1 - 1].addType(PERIMETER);
                }
                if (!(cells[r][c2 + 1].hasType(ROOM)
                      || cells[r][c2 + 1].hasType(ENTRANCE))) {
                    cells[r][c2 + 1].addType(PERIMETER);
                }
            }
            for (int c = c1 - 1; c <= c2 + 1; c++) {
                if (!(cells[r1 - 1][c].hasType(ROOM)
                      || cells[r1 - 1][c].hasType(ENTRANCE))) {
                    cells[r1 - 1][c].addType(PERIMETER);
                }
                if (!(cells[r2 + 1][c].hasType(ROOM)
                      || cells[r2 + 1][c].hasType(ENTRANCE))) {
                    cells[r2 + 1][c].addType(PERIMETER);
                }
            }
        }

        std::tuple<int, int, int, int> set_room(int _i, int _j, int height, int width) {
            if (height < 0) {
                if (_i < 0) {
                    height = vstd::rand(room_radix) + room_base;
                } else {
                    int a = n_i - room_base - _i;
                    a = a < 0 ? 0 : a;
                    auto r = (a < room_radix) ? a : room_radix;

                    height = vstd::rand(r) + room_base;
                }
            }
            if (width < 0) {
                if (_j < 0) {
                    width = vstd::rand(room_radix) + room_base;
                } else {
                    int a = n_j - room_base - _j;
                    a = a < 0 ? 0 : a;
                    auto r = (a < room_radix) ? a : room_radix;

                    width = vstd::rand(r) + room_base;
                }
            }

            return std::make_tuple(_i < 0 ? vstd::rand(n_i - height) : _i,
                                   _j < 0 ? vstd::rand(n_j - width) : _j,
                                   height,
                                   width);
        }

        std::tuple<std::map<int, int>, bool> sound_room(int r1, int c1, int r2, int c2) {
            std::map<int, int> hit;
            for (int r = r1; r <= r2; r++) {
                for (int c = c1; c <= c2; c++) {
                    if (cells[r][c].hasType(BLOCKED)) {
                        return std::make_tuple(hit, true);
                    }
                    if (cells[r][c].hasType(ROOM)) {
                        auto id = cells[r][c].getRoomId();
                        if (!vstd::ctn(hit, id)) {
                            hit[id] = 0;
                        }
                        hit[id] = hit[id] + 1;
                    }
                }
            }
            return std::make_tuple(hit, false);
        }

        void scatter_rooms() {
            for (int i = 0; i < alloc_rooms(); i++) {
                emplace_room();
            }
        }

        int alloc_rooms() {
            int dungeon_area = n_cols * n_rows;
            int room_area = options.room_max * options.room_max;
            return dungeon_area / room_area;
        }

        void open_room(Room &room, std::set<std::pair<int, int>> &connected) {
            auto list = door_sills(room);
            if (list.empty()) {
                return;
            }
            auto n_opens = alloc_opens(room);

            for (int i = 0; i < n_opens && !list.empty(); i++) {
                std::list<Sill> sills;
                auto it = list.begin();
                std::advance(it, vstd::rand(list.size() - 1));
                sills.splice(sills.begin(), list, it);
                auto sill = sills.front();
                auto door_r = sill.door_r;
                auto door_c = sill.door_c;
                auto door_cell = cells[door_r][door_c];

                if (door_cell.isDoorspace()) {
                    n_opens--;
                    continue;
                }

                auto out_id = sill.out_id;
                if (out_id) {
                    auto connect = std::make_pair(std::min(room.id, out_id), std::max(room.id, out_id));

                    if (vstd::ctn(connected, connect)) {
                        n_opens--;
                        continue;
                    }

                    connected.insert(connect);
                }
                auto open_r = sill.sill_r;
                auto open_c = sill.sill_c;
                auto open_dir = sill.dir;

                for (auto x = 0; x < 3; x++) {
                    auto r = open_r + (DI[open_dir] * x);
                    auto c = open_c + (DJ[open_dir] * x);

                    cells[r][c].removeType(PERIMETER);
                    cells[r][c].addType(ENTRANCE);
                }
                int door_type = generate_door_type();
                Door door;
                door.row = door_r;
                door.col = door_c;

                if (door_type == ARCH) {
                    cells[door_r][door_c].addType(ARCH);
                    cells[door_r][door_c].setLabel("a");
                    door.key = "arch";
                    door.type = "Archway";
                } else if (door_type == DOOR) {
                    cells[door_r][door_c].addType(DOOR);
                    cells[door_r][door_c].setLabel("o");
                    door.key = "open";
                    door.type = "Unlocked Door";
                } else if (door_type == LOCKED) {
                    cells[door_r][door_c].addType(LOCKED);
                    cells[door_r][door_c].setLabel("x");
                    door.key = "lock";
                    door.type = "Locked Door";
                } else if (door_type == TRAPPED) {
                    cells[door_r][door_c].addType(TRAPPED);
                    cells[door_r][door_c].setLabel("t");
                    door.key = "trap";
                    door.type = "Trapped Door";
                } else if (door_type == SECRET) {
                    cells[door_r][door_c].addType(SECRET);
                    cells[door_r][door_c].setLabel("s");
                    door.key = "secret";
                    door.type = "Secret Door";
                } else if (door_type == PORTC) {
                    cells[door_r][door_c].addType(PORTC);
                    cells[door_r][door_c].setLabel("p");
                    door.key = "portc";
                    door.type = "Portcullis";
                }

                door.out_id = out_id;
                if (out_id) {
                    room.door.insert({open_dir, door});
                }
            }
        }

        int generate_door_type() {
            auto i = int(vstd::rand(110));

            if (i < 15) {
                return ARCH;
            } else if (i < 60) {
                return DOOR;
            } else if (i < 75) {
                return LOCKED;
            } else if (i < 90) {
                return TRAPPED;
            } else if (i < 100) {
                return SECRET;
            } else {
                return PORTC;
            }
        }

        int alloc_opens(const Room &room) {
            auto room_h = ((room.south - room.north) / 2) + 1;
            auto room_w = ((room.east - room.west) / 2) + 1;
            auto flumph = sqrt(room_w * room_h);
            return (int) flumph + vstd::rand(flumph);
        }

        std::optional<Sill> check_sill(const Room &room, int sill_r, int sill_c, const std::string &dir) {
            auto door_r = sill_r + DI[dir];
            auto door_c = sill_c + DJ[dir];
            auto door_cell = cells[door_r][door_c];
            if (!(door_cell.hasType(PERIMETER))) {
                return {};
            }
            if (door_cell.isBlockedDoor()) {
                return {};
            }
            auto out_r = door_r + DI[dir];
            auto out_c = door_c + DJ[dir];
            auto out_cell = cells[out_r][out_c];
            if (out_cell.hasType(BLOCKED)) {
                return {};
            }
            auto out_id = -1;
            if (out_cell.hasType(ROOM)) {
                out_id = out_cell.getRoomId();
            }
            return std::make_optional<Sill>({sill_r, sill_c, dir, door_r, door_c, out_id});
        }

        std::list<Sill> door_sills(const Room &room) {
            std::list<Sill> sills;
            if (room.north >= 3) {
                for (int c = room.west; c <= room.east; c += 2) {
                    if (auto sill = check_sill(room, room.north, c, "north")) {
                        sills.push_back(sill.value());
                    }
                }
            }
            if (room.south <= n_rows - 3) {
                for (int c = room.west; c <= room.east; c += 2) {
                    if (auto sill = check_sill(room, room.south, c, "south")) {
                        sills.push_back(sill.value());
                    }
                }
            }
            if (room.west >= 3) {
                for (int r = room.north; r <= room.south; r += 2) {
                    if (auto sill = check_sill(room, r, room.west, "west")) {
                        sills.push_back(sill.value());
                    }
                }
            }
            if (room.east <= n_cols - 3) {
                for (int r = room.north; r <= room.south; r += 2) {
                    if (auto sill = check_sill(room, r, room.east, "east")) {
                        sills.push_back(sill.value());
                    }
                }
            }
            return sills;
        }

        void open_rooms() {
            std::set<std::pair<int, int>> connected;
            for (int i = 1; i <= n_rooms; i++) {
                open_room(rooms[i], connected);
            }
        }

        void label_rooms() {
            for (auto id = 1; id <= n_rooms; id++) {
                auto _room = rooms[id];
                auto label = vstd::str(_room.id);
                auto len = label.length();
                auto label_r = int((_room.north + _room.south) / 2);
                auto label_c = int((_room.west + _room.east - len) / 2) + 1;

                for (auto c = 0; c < len; c++) {
                    auto _char = label.substr(c, 1);
                    cells[label_r][label_c + c].setLabel(_char);
                }
            }
        }

        void corridors() {
            for (auto i = 1; i < n_i; i++) {
                auto r = (i * 2) + 1;
                for (auto j = 1; j < n_j; j++) {
                    auto c = (j * 2) + 1;

                    if (cells[r][c].hasType(CORRIDOR))continue;
                    tunnel(i, j);
                }
            }
        }

        void tunnel(int _i, int _j, const std::string &_last_dir = "") {
            std::queue<std::tuple<int, int, std::string>> args;
            args.push(std::make_tuple(_i, _j, _last_dir));
            while (!args.empty()) {
                auto arg = vstd::pop(args);
                auto dirs = tunnel_dirs(std::get<2>(arg));
                auto i = std::get<0>(arg);
                auto j = std::get<1>(arg);
                for (const auto &dir:dirs)
                    if (open_tunnel(i, j, dir)) {
                        auto next_i = i + DI[dir];
                        auto next_j = j + DJ[dir];

                        args.push(std::make_tuple(next_i, next_j, dir));
                    }
            }
        }

        std::deque<std::string> tunnel_dirs(const std::string &last_dir) {
            auto p = options.corridor_layout;
            std::deque<std::string> dirs;
            for (auto[key, value]:DJ) {
                dirs.push_back(key);//TODO: if(vstd::rand(1)push_back():else front
            }
            std::shuffle(dirs.begin(), dirs.end(), vstd::rng());

            if (!last_dir.empty() && p && vstd::rand(100) < p) {
                dirs.push_front(last_dir);
            }
            return dirs;
        }

        bool open_tunnel(int i, int j, const std::string &dir) {
            auto this_r = (i * 2) + 1;
            auto this_c = (j * 2) + 1;
            auto next_r = ((i + DI[dir]) * 2) + 1;
            auto next_c = ((j + DJ[dir]) * 2) + 1;
            auto mid_r = (this_r + next_r) / 2;
            auto mid_c = (this_c + next_c) / 2;

            if (sound_tunnel(mid_r, mid_c, next_r, next_c)) {
                return delve_tunnel(this_r, this_c, next_r, next_c);
            } else {
                return false;
            }
        }

        bool sound_tunnel(int mid_r, int mid_c, int next_r, int next_c) {

            if (next_r < 0 || next_r > n_rows) {
                return false;
            }
            if (next_c < 0 || next_c > n_cols) {
                return false;
            }
            auto r1 = std::min(mid_r, next_r);
            auto r2 = std::max(mid_r, next_r);
            auto c1 = std::min(mid_c, next_c);
            auto c2 = std::max(mid_c, next_c);

            for (auto r = r1; r <= r2; r++) {
                for (auto c = c1; c <= c2; c++) {
                    if (cells[r][c].isBlockedCorridor()) {
                        return false;
                    }
                }
            }

            return true;
        }

        bool delve_tunnel(int this_r, int this_c, int next_r, int next_c) {
            auto r1 = std::min(this_r, next_r);
            auto r2 = std::max(this_r, next_r);
            auto c1 = std::min(this_c, next_c);
            auto c2 = std::max(this_c, next_c);

            for (auto r = r1; r <= r2; r++) {
                for (auto c = c1; c <= c2; c++) {
                    cells[r][c].removeType(ENTRANCE);
                    cells[r][c].addType(CORRIDOR);
                }
            }
            return true;
        }

        void emplace_stairs() {
            auto n = options.add_stairs;
            if (n <= 0) {
                return;
            }
            auto list = stair_ends();

            if (list.empty()) {
                return;
            }

            for (int i = 0; i < n; i++) {
                auto it = list.begin();
                std::advance(it, vstd::rand(list.size()));
                Stairs stairs = *it;
                list.erase(it);

                auto r = stairs.row;
                auto c = stairs.col;
                auto type = (i < 2) ? i : vstd::rand(2);


                if (type == 0) {
                    cells[r][c].addType(STAIR_DN);
                    cells[r][c].setLabel("d");
                    stairs.key = "down";
                } else {
                    cells[r][c].addType(STAIR_UP);
                    cells[r][c].setLabel("u");
                    stairs.key = "up";
                }
                this->stairs.push_back(stairs);
            }
        }

        bool check_tunnel(int r, int c, std::map<std::string, std::vector<std::vector<int>>> check) {
            for (auto p:check["corridor"]) {
                if (!cells[r + p[0]][c + p[1]].hasType(CORRIDOR)) {
                    return false;
                }
            }
            for (auto p:check["walled"]) {
                if (cells[r + p[0]][c + p[1]].isOpenspace()) {
                    return false;
                }
            }
            return true;
        }

        std::list<Stairs> stair_ends() {
            std::list<Stairs> stairs;

            for (auto i = 0; i < n_i; i++) {
                auto r = (i * 2) + 1;
                for (auto j = 0; j < n_j; j++) {
                    auto c = (j * 2) + 1;

                    if (!cells[r][c].hasType(CORRIDOR) || cells[r][c].isStairs()) {
                        continue;
                    }
                    for (auto[dir, dir_value]:STAIR_END) {
                        if (check_tunnel(r, c, dir_value)) {
                            Stairs end;
                            end.row = r;
                            end.col = c;

                            auto n = dir_value["next"];
                            end.next_row = end.row + n[0][0];
                            end.next_col = end.col + n[0][1];

                            stairs.push_back(end);
                            break;
                        }
                    }
                }
            }
            return stairs;
        }

        void collapse(int r, int c) {
            if (!(cells[r][c].isOpenspace())) {
                return;
            }
            for (auto[key, value]:CLOSE_END)
                if (check_tunnel(r, c, value)) {
                    for (auto p:value["close"]) {
                        cells[r + p[0]][c + p[1]].clearTypes();
                    }
                    for (auto p:value["open"]) {
                        cells[r + p[0]][c + p[1]].addType(CORRIDOR);
                    }
                    for (auto p:value["recurse"]) {
                        collapse(r + p[0], c + p[1]);
                    }
                }
        }

        void collapse_tunnels(int p) {
            if (!p) {
                return;
            }
            auto all = p == 100;

            for (int i = 0; i < n_i; i++) {
                auto r = (i * 2) + 1;
                for (int j = 0; j < n_j; j++) {
                    auto c = (j * 2) + 1;

                    if (!(cells[r][c].isOpenspace())) {
                        continue;
                    }
                    if (cells[r][c].isStairs()) {
                        continue;
                    }
                    if (!(all || vstd::rand(100) < p)) {
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

            for (auto[room_index, room_data]:rooms) {
                std::set<std::string> dirs;
                for (auto[dir, _]:room_data.door) {
                    dirs.insert(dir);
                }
                for (const auto &dir:dirs) {
                    std::list<Door> shiny;
                    auto range = room_data.door.equal_range(dir);
                    for (auto it = range.first; it != range.second; it++) {
                        auto door = it->second;
                        auto door_r = door.row;
                        auto door_c = door.col;
                        auto door_cell = cells[door_r][door_c];
                        if (!(door_cell.isOpenspace())) {
                            continue;
                        }

                        if (vstd::ctn(fixed, std::make_pair(door_r, door_c))) {
                            shiny.push_back(door);
                        } else {
                            if (auto out_id = door.out_id) {
                                auto out_dir = OPPOSITE[dir];
                                rooms[out_id].door.insert(std::make_pair(out_dir, door));
                            }
                            shiny.push_back(door);
                            fixed.insert(std::make_pair(door_r, door_c));
                        }
                    }
                    if (!shiny.empty()) {
                        room_data.door.erase(dir);
                        for (const auto &shiny_door:shiny) {
                            room_data.door.insert(std::make_pair(dir, shiny_door));
                        }
                        doors.push_back(shiny);
                    } else {
                        room_data.door.erase(dir);
                    }
                }
            }
        }

        void empty_blocks() {
            for (auto r = 0; r <= n_rows; r++) {
                for (auto c = 0; c <= n_cols; c++) {
                    if (cells[r][c].hasType(BLOCKED)) {
                        cells[r][c].clearTypes();
                    }
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

public:
    static Dungeon create_dungeon(Options
                                  options) {
        Dungeon dungeon(std::move(options));

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
};

template<typename T>
std::map<std::string, std::vector<std::vector<int>>> rdg<T>::DUNGEON_LAYOUT = {
        {"Box",   {{1, 1, 1}, {1, 0, 1}, {1, 1, 1}}},
        {"Cross", {{0, 1, 0}, {1, 1, 1}, {0, 1, 0}}}
};
template<typename T>
std::map<std::string, int> rdg<T>::DI = {{"north", -1},
                                         {"south", 1},
                                         {"west",  0},
                                         {"east",  0}};
template<typename T>
std::map<std::string, int> rdg<T>::DJ = {{"north", 0},
                                         {"south", 0},
                                         {"west",  -1},
                                         {"east",  1}};

template<typename T>
std::map<std::string, std::map<std::string, std::vector<std::vector<int>>>> rdg<T>::STAIR_END = {
        {"north", {
                          {"walled", {{1,  -1}, {0,  -1}, {-1, -1}, {-1, 0},  {-1, 1},  {0, 1}, {1,  1}}},
                          {"corridor", {{0, 0}, {1,  0},  {2,  0}}},
                          {"stair", {{0, 0}}},
                          {"next", {{1,  0}}}
                  }},
        {"south", {
                          {"walled", {{-1, -1}, {0,  -1}, {1,  -1}, {1,  0},  {1,  1},  {0, 1}, {-1, 1}}},
                          {"corridor", {{0, 0}, {-1, 0},  {-2, 0}}},
                          {"stair", {{0, 0}}},
                          {"next", {{-1, 0}}}
                  }},
        {"west",  {
                          {"walled", {{-1, 1},  {-1, 0},  {-1, -1}, {0,  -1}, {1,  -1}, {1, 0}, {1,  1}}},
                          {"corridor", {{0, 0}, {0,  1},  {0,  2}}},
                          {"stair", {{0, 0}}},
                          {"next", {{0,  1}}}
                  }},
        {"east",  {
                          {"walled", {{-1, -1}, {-1, 0},  {-1, 1},  {0,  1},  {1,  1},  {1, 0}, {1,  -1}}},
                          {"corridor", {{0, 0}, {0,  -1}, {0,  -2}}},
                          {"stair", {{0, 0}}},
                          {"next", {{0,  -1}}}
                  }}
};
template<typename T>
std::map<std::string, std::map<std::string, std::vector<std::vector<int>>>> rdg<T>::CLOSE_END = {
        {"north", {
                          {"walled", {{0,  -1}, {1,  -1}, {1,  0},  {1,  1},  {0, 1}}},
                          {"close", {{0, 0}}},
                          {"recurse", {{-1, 0}}},
                  }},
        {"south", {
                          {"walled", {{0,  -1}, {-1, -1}, {-1, 0},  {-1, 1},  {0, 1}}},
                          {"close", {{0, 0}}},
                          {"recurse", {{1,  0}}},
                  }},
        {"west",  {
                          {"walled", {{-1, 0},  {-1, 1},  {0,  1},  {1,  1},  {1, 0}}},
                          {"close", {{0, 0}}},
                          {"recurse", {{0,  -1}}},
                  }},
        {"east",  {
                          {"walled", {{-1, 0},  {-1, -1}, {0,  -1}, {1,  -1}, {1, 0}}},
                          {"close", {{0, 0}}},
                          {"recurse", {{0,  1}}},
                  }}
};

template<typename T>
std::map<std::string, std::string> rdg<T>::OPPOSITE = {
        {"north", "south"},
        {"south", "north"},
        {"west",  "east"},
        {"east",  "west"}
};