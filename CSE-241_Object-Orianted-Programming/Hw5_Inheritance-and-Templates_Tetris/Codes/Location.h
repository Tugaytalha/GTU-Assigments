
#ifndef TetLoc
#define TetLoc

namespace Tetris
{
    // keeps a location
    class Location
    { // this class don't need big tree
    public:
        int x;
        int y;
        bool moved;                              // piece moved or not
        bool operator>=(const Location &) const; // is other in 1 under
        bool operator<=(const Location &) const; // is other in 1 above
        bool operator>(const Location &) const;  // is other in 1 right
        bool operator<(const Location &) const;  // is other in 1 left
    };
}

#endif