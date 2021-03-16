#include <iostream>
#include <utility>
#include <map>
#include <vector>
#include <cmath>
#include <vstd.h>

namespace rdg {
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

    class Cell {
        friend class Dungeon;

        std::set<CellType> types;
        int room_id;
        std::string label;
    public:
        bool isBlockedRoom();

        bool isBlockedCorridor();

        bool isBlockedDoor();

        bool hasLabel();

        std::string getLabel();

        bool isEspace();

        bool hasType(CellType type);

        bool isOpenspace();

        bool isDoorspace();

        bool isStairs();

        int getRoomId();

    private:
        void setType(CellType type);

        void setLabel(std::string label);

        void setRoomId(int room_id);

        void addType(CellType type);

        void removeType(CellType type);

        void clearTypes();

        void clearLabel();

        void clearEspace();
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
        const int n_rows = 39;  //must be an odd number
        const int n_cols = 39;  //must be an odd number
        const std::string dungeon_layout = "None";
        const int room_min = 3; //minimum rooms size
        const int room_max = 9; //maximum rooms size
        const std::string room_layout = "Scattered";  //Packed, Scattered
        const std::string corridor_layout = "Straight";
        const int remove_deadends = 100;//percentage
        const int add_stairs = 2; //number of stairs
        const std::string map_style = "Standard";
        const int cell_size = 18; //pixels
    };

    class Dungeon {
    public:
        static Dungeon create_dungeon(Options options);

        std::vector<std::vector<Cell>> getCells() {
            return cells;
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
                         _options);

        void init_cells();

        void mask_cells(std::vector<std::vector<int>> mask);

        void round_mask();

        void emplace_rooms();

        void pack_rooms();

        void emplace_room(int _i = -1, int _j = -1, int _height = -1, int _width = -1);

        std::tuple<int, int, int, int> set_room(int _i, int _j, int _height, int _width);

        std::tuple<std::map<int, int>, bool> sound_room(int r1, int c1, int r2, int c2);

        void scatter_rooms();

        int alloc_rooms();

        void open_room(Room &room, std::set<std::pair<int, int>> &connected);

        int generate_door_type();

        int alloc_opens(const Room &room);

        std::optional<Sill> check_sill(const Room &room, int sill_r, int sill_c, const std::string &dir);

        std::list<Sill> door_sills(const Room &room);

        void open_rooms();

        void label_rooms();

        void corridors();

        void tunnel(int i, int j, const std::string &last_dir = "");

        std::deque<std::string> tunnel_dirs(const std::string &last_dir);

        bool open_tunnel(int i, int j, const std::string &dir);

        bool sound_tunnel(int mid_r, int mid_c, int next_r, int next_c);

        bool delve_tunnel(int this_r, int this_c, int next_r, int next_c);

        void emplace_stairs();

        bool check_tunnel(int r, int c, std::map<std::string, std::vector<std::vector<int>>> check);

        std::list<Stairs> stair_ends();

        void collapse(int r, int c);

        void collapse_tunnels(int p);

        void remove_deadends();

        void fix_doors();

        void empty_blocks();

        void clean_dungeon();
    };


}