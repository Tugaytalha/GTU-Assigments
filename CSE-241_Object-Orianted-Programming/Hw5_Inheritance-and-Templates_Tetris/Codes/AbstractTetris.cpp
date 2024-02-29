#include "AbstractTetris.h"
namespace Tetris {

// Makes tetris board
AbstractTetris::AbstractTetris(const int wide, const int high) {
    board_high = high;
    board_wide = wide;
    last_loc = new Location[4];
    noRotate = -9994;
    noShift = -9994;
}

// Destruct last_loc
AbstractTetris::~AbstractTetris() {
    cout << "base des" << endl << endl; 
    if (last_loc != nullptr) {
        delete[] last_loc;
    }
}
    
std::pair<int, int> AbstractTetris::lastMove() const {
    return pair<int, int>(noRotate, noShift);
}

const char AbstractTetris::last_tetro() const {
    char tt;
     switch (tetro)
        { // Fill vector of tetromino according to strong enum class's member
        case charimino::I:
            tt = 'I';
            break;
        case charimino::O:
            tt = 'O';
            break;
        case charimino::T:
            tt = 'T';
            break;
        case charimino::J:
            tt = 'J';
            break;
        case charimino::L:
            tt = 'L';
            break;
        case charimino::S:
            tt = 'S';
            break;
        case charimino::Z:
            tt = 'Z';
            break;
        }
    
    return tt;
}


}