#include <iostream>
#include <vector>
#define NDEBUG
using namespace std;


class Tetris {
public:
   Tetris();  // make tetris board with 10 wide 20 high
   Tetris(int wide, int high);  // make tetris board with user input wide and high
   void Draw() const;
   bool Add(const Tetromino& Tetro);  //  add a Tetromino to the board at the top row in the middle
   void animate();  // animate drop of tetromino
   
private:
   vector<vector<char>> board;  
   vector<vector<char>> last_tetromino;  
   int board_wide, board_high;
};