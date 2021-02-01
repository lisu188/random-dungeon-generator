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

struct Room {
    Room(const int id, const int row, const int col, const int north, const int south, const int west, const int east,
         const int height, const int width) : id(id), row(row), col(col), north(north), south(south),
                                              west(west), east(east), height(height), width(width),
                                              area(height * width) {}

    const int id;
    const int row;
    const int col;
    const int north;
    const int south;
    const int west;
    const int east;
    const int height;
    const int width;
    const int area;
};

struct Options {
    const int n_rows = 39;  //must be an odd number
    const int n_cols = 39;  //must be an odd number
    const std::string dungeon_layout = "None";
    const int room_min = 3; //minimum room size
    const int room_max = 9; //maximum room size
    const std::string room_layout = "Packed";  //Packed, Scattered
    const std::string corridor_layout = "Bent";
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
    explicit Dungeon(Options _options) :
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

        room.try_emplace(room_id, room_id, r1, c1, r1, r2, c1, c2, ((r2 - r1) + 1), ((c2 - c1) + 1));

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

    void open_room(Room room) {
        auto list = door_sills(room);
        if (list.empty()) {
            return;
        }
        auto n_opens = alloc_opens(room);

        for (int i=0;i<n_opens;i++){
            my $sill = splice(@list,int(rand(@list)),1);
            last unless ($sill);
            my $door_r = $sill->{'door_r'};
            my $door_c = $sill->{'door_c'};
            my $door_cell = $cell->[$door_r][$door_c];
            redo if ($door_cell & $DOORSPACE);

            my $out_id; if ($out_id = $sill->{'out_id'}) {
                my $connect = join(',',(sort($room->{'id'},$out_id)));
                redo if ($dungeon->{'connect'}{$connect}++);
            }
            my $open_r = $sill->{'sill_r'};
            my $open_c = $sill->{'sill_c'};
            my $open_dir = $sill->{'dir'};

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
# open door

                    my $x; for ($x = 0; $x < 3; $x++) {
                my $r = $open_r + ($di->{$open_dir} * $x);
                my $c = $open_c + ($dj->{$open_dir} * $x);

                $cell->[$r][$c] &= ~ $PERIMETER;
                $cell->[$r][$c] |= $ENTRANCE;
            }
            my $door_type = &door_type();
            my $door = { 'row' => $door_r, 'col' => $door_c };

            if ($door_type == $ARCH) {
                $cell->[$door_r][$door_c] |= $ARCH;
                $door->{'key'} = 'arch'; $door->{'type'} = 'Archway';
            } elsif ($door_type == $DOOR) {
                $cell->[$door_r][$door_c] |= $DOOR;
                $cell->[$door_r][$door_c] |= (ord('o') << 24);
                $door->{'key'} = 'open'; $door->{'type'} = 'Unlocked Door';
            } elsif ($door_type == $LOCKED) {
                $cell->[$door_r][$door_c] |= $LOCKED;
                $cell->[$door_r][$door_c] |= (ord('x') << 24);
                $door->{'key'} = 'lock'; $door->{'type'} = 'Locked Door';
            } elsif ($door_type == $TRAPPED) {
                $cell->[$door_r][$door_c] |= $TRAPPED;
                $cell->[$door_r][$door_c] |= (ord('t') << 24);
                $door->{'key'} = 'trap'; $door->{'type'} = 'Trapped Door';
            } elsif ($door_type == $SECRET) {
                $cell->[$door_r][$door_c] |= $SECRET;
                $cell->[$door_r][$door_c] |= (ord('s') << 24);
                $door->{'key'} = 'secret'; $door->{'type'} = 'Secret Door';
            } elsif ($door_type == $PORTC) {
                $cell->[$door_r][$door_c] |= $PORTC;
                $cell->[$door_r][$door_c] |= (ord('#') << 24);
                $door->{'key'} = 'portc'; $door->{'type'} = 'Portcullis';
            }
            $door->{'out_id'} = $out_id if ($out_id);
            push(@{ $room->{'door'}{$open_dir} },$door) if ($door);
        }
    }

    int alloc_opens(Room room) {
        auto room_h = ((room.south - room.north) / 2) + 1;
        auto room_w = ((room.east - room.west) / 2) + 1;
        auto flumph = sqrt(room_w * room_h);
        return (int)flumph + vstd::rand(flumph);
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
        std::shuffle(sills.begin(), sills.end(), vstd::rng());
        return sills;
    }

    void open_rooms() {
        for (int i = 1; i <= n_rooms; i++) {
            open_room(room[i]);
        }
    }
};

Dungeon create_dungeon(Options
                       options) {
    Dungeon dungeon(options);

    dungeon.init_cells();

    dungeon.emplace_rooms();

    dungeon.open_rooms();
//
//    dungeon.label_rooms();
//
//    dungeon.corridors();
//
//    if (dungeon.options.add_stairs) {
//        dungeon.emplace_stairs();
//
//    }
//    dungeon.clean_dungeon();

    return dungeon;
}

int main() {
    auto dungeon = create_dungeon(Options());
    for (const auto &row:dungeon.cell) {
        for (auto col:row) {
            if (col & ROOM) {
                std::cout << "X";
            } else {
                std::cout << " ";
            }
            std::cout << " ";
        }
        std::cout << std::endl;
    }
    return 0;
}
