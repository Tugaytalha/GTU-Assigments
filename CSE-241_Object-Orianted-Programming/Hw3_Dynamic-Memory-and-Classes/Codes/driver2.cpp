#include <iostream>
#include "Tetromino.h"
#include "Board.h"
#include <time.h>
using namespace std;

/* in this case we can use also "using namespace Tetris;" 
 * instead of writing Tetris:: again and again 
 */
int main() {
    // Providing a seed value
	srand((unsigned) time(NULL));

    Tetris::input input_object;
    int wide,high;
    bool valid = true;

    cout << "Enter board wide: ";
    while((wide = input_object.take_number()) < 10) cout << "Enter a number greater than 10: ";
    cout << "Enter board high: ";
    while((high = input_object.take_number()) < 10) cout << "Enter a number greater than 10: ";
    Tetris::Board game_board(wide, high);

    while(valid) {
        Tetris::Tetromino Tet(input_object.take_input());
        valid = (game_board += Tet);
        game_board.Rotate("r");
        if (valid) game_board.animate();
    }
    game_board.Draw();
    system("clear");
    cout << " \033[15B" << "\033[40C" << "Game over!!!" << " \033[10B"; 

    return 0;
}