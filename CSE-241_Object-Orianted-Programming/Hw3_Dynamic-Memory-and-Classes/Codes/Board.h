#include <iostream>
#include "Tetromino.h"

using namespace std;

#ifndef Board_T_H
#define Board_T_H

namespace Tetris {
// keeps a location
class Location { // this class don't need big tree
public:
    int x;
    int y;
    bool moved;  //piece moved or not
    bool operator>=(const Location&) const;  // is other in 1 under
    bool operator<=(const Location&) const;  // is other in 1 above
    bool operator>(const Location&) const;   // is other in 1 right
    bool operator<(const Location&) const;  // is other in 1 left
};

class Board {
public:  // don't need big tree
    Board();  // make tetris board with 10 wide 20 high
    Board(const int wide, const int high);  // make tetris board with user input wide and high
    ~Board();
    void Draw() const;  // draw board to screen
    bool operator+=(const Tetromino& Tetro);  //  add a Tetromino to the board at the top row in the middle
    bool animate();  // animate drop of tetromino
    bool operator>>(int);  // move tetromino horizantal in given number(negatives represent left)
    void move_left(int);  // move tetromino 1 right
    void move_right(int);  // move tetromino 1 left
    void operator||(int);  // move tetromino 1 down
    bool move_test(char) const;  // test tetromino movebility in given direction
    bool Rotate(string);  // rotate last added tetromino

private:
    bool Rotate_left(int index, int minx, int miny, int maxx, int maxy);
    bool Rotate_right(int index, int minx, int miny, int maxx, int maxy);
    char **board_vec;  // board array
    Location *last_loc;  // last added tetromino's location
    int board_wide, board_high;
};
};


#endif

