#include <iostream>
#include <utility>
#include <map>
#include <vector>
#include <cmath>
#include <vstd.h>

std::map<std::string, std::vector<std::vector<int>>> DUNGEON_LAYOUT = {
        {"Box",   {{1, 1, 1}, {1, 0, 1}, {1, 1, 1}}},
        {"Cross", {{0, 1, 0}, {1, 1, 1}, {0, 1, 0}}}
};

std::map<std::string, int> DI = {{"north", -1},
                                 {"south", 1},
                                 {"west",  0},
                                 {"east",  0}};
std::map<std::string, int> DJ = {{"north", 0},
                                 {"south", 0},
                                 {"west",  -1},
                                 {"east",  1}};

std::map<std::string, int> CORRIDOR_LAYOUT = {
        {"Labyrinth", 0},
        {"Bent",      50},
        {"Straight",  100}
};

std::map<std::string, std::map<std::string, std::vector<std::vector<int>>>> STAIR_END = {
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


int NOTHING = 0x00000000;

int BLOCKED = 0x00000001;
int ROOM = 0x00000002;
int CORRIDOR = 0x00000004;
//                 0x00000008;
int PERIMETER = 0x00000010;
int ENTRANCE = 0x00000020;
int ROOM_ID = 0x0000FFC0;

int ARCH = 0x00010000;
int DOOR = 0x00020000;
int LOCKED = 0x00040000;
int TRAPPED = 0x00080000;
int SECRET = 0x00100000;
int PORTC = 0x00200000;
int STAIR_DN = 0x00400000;
int STAIR_UP = 0x00800000;

int LABEL = 0xFF000000;

int OPENSPACE = ROOM | CORRIDOR;
int DOORSPACE = ARCH | DOOR | LOCKED | TRAPPED | SECRET | PORTC;
int ESPACE = ENTRANCE | DOORSPACE | 0xFF000000;
int STAIRS = STAIR_DN | STAIR_UP;

int BLOCK_ROOM = BLOCKED | ROOM;
int BLOCK_CORR = BLOCKED | PERIMETER | CORRIDOR;
int BLOCK_DOOR = BLOCKED | DOORSPACE;

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
};

struct Options {
    const int n_rows = 39;  //must be an odd number
    const int n_cols = 39;  //must be an odd number
    const std::string dungeon_layout = "None";
    const int room_min = 3; //minimum room size
    const int room_max = 9; //maximum room size
    const std::string room_layout = "Scattered";  //Packed, Scattered
    const std::string corridor_layout = "Straight";
    const int remove_deadends = 50;//percentage
    const int add_stairs = 2; //number of stairs
    const std::string map_style = "Standard";
    const int cell_size = 18; //pixels
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
public:
    const Options options;
    std::vector<std::vector<int>> cell;
    std::map<int, Room> room;
private:
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
public:
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
            cell.emplace_back();
            for (int c = 0; c <= n_cols; c++) {
                cell[r].push_back(NOTHING);
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
                    cell[r][c] = BLOCKED;
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
                    cell[r][c] = BLOCKED;
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

                if (cell[r][c] & ROOM) {
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
                if (cell[r][c] & ENTRANCE) {
                    cell[r][c] = cell[r][c] & ~ESPACE;
                } else if (cell[r][c] & PERIMETER) {
                    cell[r][c] = cell[r][c] & ~PERIMETER;
                }
                cell[r][c] = cell[r][c] | (ROOM | (room_id << 6));
            }
        }

        int __height = (r2 - r1) + 1;
        int __width = (c2 - c1) + 1;
        Room _room = {room_id, r1, c1, r1, r2, c1, c2, __height, __width, __height * __width};
        room[room_id] = _room;

        for (int r = r1 - 1; r <= r2 + 1; r++) {
            if (!(cell[r][c1 - 1] & (ROOM | ENTRANCE))) {
                cell[r][c1 - 1] = cell[r][c1 - 1] | PERIMETER;
            }
            if (!(cell[r][c2 + 1] & (ROOM | ENTRANCE))) {
                cell[r][c2 + 1] = cell[r][c2 + 1] | PERIMETER;
            }
        }
        for (int c = c1 - 1; c <= c2 + 1; c++) {
            if (!(cell[r1 - 1][c] & (ROOM | ENTRANCE))) {
                cell[r1 - 1][c] = cell[r1 - 1][c] | PERIMETER;
            }
            if (!(cell[r2 + 1][c] & (ROOM | ENTRANCE))) {
                cell[r2 + 1][c] = cell[r2 + 1][c] | PERIMETER;
            }
        }
    }

    std::tuple<int, int, int, int> set_room(int _i, int _j, int _height, int _width) {
        int height, width;
        if (_height < 0) {
            if (_i < 0) {
                height = vstd::rand(room_radix) + room_base;
            } else {
                int a = n_i - room_base - _i;
                a = a < 0 ? 0 : a;
                auto r = (a < room_radix) ? a : room_radix;

                height = vstd::rand(r) + room_base;
            }
        }
        if (_width < 0) {
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
                if (cell[r][c] & BLOCKED) {
                    return std::make_tuple(hit, true);
                }
                if (cell[r][c] & ROOM) {
                    auto id = (cell[r][c] & ROOM_ID) >> 6;
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
            std::advance(it, vstd::rand(list.size()));
            sills.splice(sills.begin(), list, it);
            auto sill = sills.front();
            auto door_r = sill.door_r;
            auto door_c = sill.door_c;
            auto door_cell = cell[door_r][door_c];

            if (door_cell & DOORSPACE) {
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

                cell[r][c] = cell[r][c] & ~PERIMETER;
                cell[r][c] = cell[r][c] | ENTRANCE;
            }
            int door_type = generate_door_type();
            Door door;
            door.row = door_r;
            door.col = door_c;

            if (door_type == ARCH) {
                cell[door_r][door_c] = cell[door_r][door_c] | ARCH;
                cell[door_r][door_c] = cell[door_r][door_c] | ('a' << 24);
                door.key = "arch";
                door.type = "Archway";
            } else if (door_type == DOOR) {
                cell[door_r][door_c] = cell[door_r][door_c] | DOOR;
                cell[door_r][door_c] = cell[door_r][door_c] | ('o' << 24);
                door.key = "open";
                door.type = "Unlocked Door";
            } else if (door_type == LOCKED) {
                cell[door_r][door_c] = cell[door_r][door_c] | LOCKED;
                cell[door_r][door_c] = cell[door_r][door_c] | ('x' << 24);
                door.key = "lock";
                door.type = "Locked Door";
            } else if (door_type == TRAPPED) {
                cell[door_r][door_c] = cell[door_r][door_c] | TRAPPED;
                cell[door_r][door_c] = cell[door_r][door_c] | ('t' << 24);
                door.key = "trap";
                door.type = "Trapped Door";
            } else if (door_type == SECRET) {
                cell[door_r][door_c] = cell[door_r][door_c] | TRAPPED;
                cell[door_r][door_c] = cell[door_r][door_c] | ('s' << 24);
                door.key = "secret";
                door.type = "Secret Door";
            } else if (door_type == PORTC) {
                cell[door_r][door_c] = cell[door_r][door_c] | TRAPPED;
                cell[door_r][door_c] = cell[door_r][door_c] | ('p' << 24);
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

    int alloc_opens(Room room) {
        auto room_h = ((room.south - room.north) / 2) + 1;
        auto room_w = ((room.east - room.west) / 2) + 1;
        auto flumph = sqrt(room_w * room_h);
        return (int) flumph + vstd::rand(flumph);
    }

    std::optional<Sill> check_sill(Room room, int sill_r, int sill_c, std::string dir) {
        auto door_r = sill_r + DI[dir];
        auto door_c = sill_c + DJ[dir];
        auto door_cell = cell[door_r][door_c];
        if (!(door_cell & PERIMETER)) {
            return {};
        }
        if (door_cell & BLOCK_DOOR) {
            return {};
        }
        auto out_r = door_r + DI[dir];
        auto out_c = door_c + DJ[dir];
        auto out_cell = cell[out_r][out_c];
        if (out_cell & BLOCKED) {
            return {};
        }
        auto out_id = -1;
        if (out_cell & ROOM) {
            out_id = (out_cell & ROOM_ID) >> 6;
        }
        return std::make_optional<Sill>({sill_r, sill_c, dir, door_r, door_c, out_id});
    }

    std::list<Sill> door_sills(Room room) {
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
            open_room(room[i], connected);
        }
    }

    void label_rooms() {
        for (auto id = 1; id <= n_rooms; id++) {
            auto _room = room[id];
            auto label = vstd::str(_room.id);
            auto len = label.length();
            auto label_r = int((_room.north + _room.south) / 2);
            auto label_c = int((_room.west + _room.east - len) / 2) + 1;

            for (auto c = 0; c < len; c++) {
                auto _char = label.substr(c, 1)[0];
                cell[label_r][label_c + c] = cell[label_r][label_c + c] | (_char << 24);
            }
        }
    }

    void corridors() {
        for (auto i = 1; i < n_i; i++) {
            auto r = (i * 2) + 1;
            for (auto j = 1; j < n_j; j++) {
                auto c = (j * 2) + 1;

                if (cell[r][c] & CORRIDOR)continue;
                tunnel(i, j);
            }
        }
    }

    void tunnel(int i, int j, std::string last_dir = "") {
        auto dirs = tunnel_dirs(last_dir);

        for (auto dir:dirs)
            if (open_tunnel(i, j, dir)) {
                auto next_i = i + DI[dir];
                auto next_j = j + DJ[dir];

                tunnel(next_i, next_j, dir);
            }
    }

    std::deque<std::string> tunnel_dirs(std::string last_dir) {
        auto p = CORRIDOR_LAYOUT[options.corridor_layout];
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

    bool open_tunnel(int i, int j, std::string dir) {
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
                if (cell[r][c] & BLOCK_CORR) {
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
                cell[r][c] = cell[r][c] & ~ENTRANCE;
                cell[r][c] = cell[r][c] | CORRIDOR;
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
        return $dungeon
        unless(@list);
        my $cell = $dungeon->{ 'cell' };

        my $i;
        for ($i = 0; $i < $n; $i++) {
            my
            $stair = splice(@list, int(rand(@list)),1);
            last unless($stair);
            my $r = $stair->{ 'row' };
            my $c = $stair->{ 'col' };
            my $type = ($i < 2) ? $i : int(rand(2));

            if ($type == 0) {
                $cell->[$r][$c] |= $STAIR_DN;
                $cell->[$r][$c] |= (ord('d') << 24);
                $stair->
                { 'key' } = 'down';
            } else {
                $cell->[$r][$c] |= $STAIR_UP;
                $cell->[$r][$c] |= (ord('u') << 24);
                $stair->
                { 'key' } = 'up';
            }
            push(@{
                $dungeon->
                { 'stair' }
            },$stair);
        }
        return $dungeon;
    }

    bool check_tunnel(int r, int c, std::map<std::string, std::vector<std::vector<int>>> check) {
        for (auto p:check["corridor"]) {
            if (cell[r + p[0]][c + p[1]] != CORRIDOR) {
                return false;
            }
        }
        for (auto p:check["walled"]) {
            if (cell[r + p[0]][c + p[1]] & OPENSPACE) {
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

                if (cell[r][c] != CORRIDOR || cell[r][c] & STAIRS) {
                    continue;
                }
                for (auto[dir,dir_value]:STAIR_END) {
                    if (check_tunnel(r, c, dir_value)) {
                        Stairs end;
                        end.row = r;
                        end.col = c;

                        auto n=dir_value["next"];
                        end.next_row=end.row+n[0][0];
                        end.next_col=end.col+n[0][1];

                       stairs.push_back(end);
                       break;
                    }
                }
            }
        }
        return stairs;
    }

};

Dungeon create_dungeon(Options
                       options) {
    Dungeon dungeon(options);

    dungeon.init_cells();

    dungeon.emplace_rooms();

    dungeon.open_rooms();

    dungeon.label_rooms();

    dungeon.corridors();

    if (dungeon.options.add_stairs) {
        dungeon.emplace_stairs();
    }
//    dungeon.clean_dungeon();

    return dungeon;
}

int main() {
    auto dungeon = create_dungeon(Options());
    for (const auto &row:dungeon.cell) {
        for (auto col:row) {
            if (auto label = char((col & LABEL) >> 24)) {
                std::cout << label;
            } else if (col & ROOM) {
                std::cout << "X";
            } else if (col & CORRIDOR) {
                std::cout << "x";
            } else if (col & DOORSPACE) {
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
