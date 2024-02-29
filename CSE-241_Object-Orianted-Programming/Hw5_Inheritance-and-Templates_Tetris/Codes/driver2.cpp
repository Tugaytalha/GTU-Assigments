#include "Tetris.h"
 

/* in this case we can use also "using namespace Tetris;" 
 * instead of writing Tetris:: again and again 
 */ 
int main() {
    Tetris::input input_object;
    int board_type, adap_type = 1;

    cout << "[1] TetrisVector \n[2] TetrisArray1D \n[3] TetrisAdapter \nBoard Type: ";
    while (((board_type = input_object.take_number()) > 3 || board_type < 0) || ((board_type == 3) && (Tetris::tReader(cout << "\n[1] Deque \n[2] Vector \nBoard Type: ")) && (adap_type = input_object.take_number()) > 3 || adap_type < 0) ) cout << "Please enter number between 1-3: ";
    board_type += adap_type - 1;

    play_tetris(board_type);

    return 0;
}