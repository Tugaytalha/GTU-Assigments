#include <iostream>
#include <vector>
#include "Tetris.h"
#define NDEBUG
using namespace std;



Tetris::Tetris() : Tetris(10, 20) {}  // Calls other constructor that takes wide and high with 10 wide and 20 high

// Makes tetris board
Tetris::Tetris(const int wide, const int high) {
    vector<char> temp;
    temp.resize(wide, ' ');
    board.resize(high, temp); 
    board_high = high;
    board_wide = wide;
}

void Tetris::Draw() const {
    int Bsize, Rsize;

    Bsize = board.size();
    Rsize = board[0].size();
    for (int i = -1; i <= Bsize; ++i) {
        cout << "x ";
        for (int j = 0; j < Rsize; ++j) {
            if(-1 == i || Bsize == i){
                cout << "x ";
            }
            else {
                cout << board[i][j] << " ";
            }
        }
        cout << "x ";
        cout << endl;
    }
}

bool Tetris::Add(const Tetromino& Tetro) {
    vector<vector<char>> tetro_vec = Tetro.get_tetro();
    int ccounter, i = board_wide / 2 - 1, j = 0;
    bool flag = false;
    vector<char> temp;
    temp.resize(board_wide, ' ');
    last_tetromino.resize(board_high, temp); 

    for(auto vrow : tetro_vec) {
        ccounter = 0;
        if(flag) {
            ++j;
            i = board_wide / 2 - 1;
        }
        for(auto c : vrow) {
            if(!flag ) {  // until current on tetromino lettera
                if (c !=' '){
                    flag = true;
                    for (int k = ccounter; k > 0; --k) {
                        last_tetromino[j][i] = ' ';
                        board[j][i++] = ' ';
                    }
                    last_tetromino[j][i] = c;
                    board[j][i] = c;
                    ++i;
                }
            }
            else {
                if (' ' == board[j][i]) {  
                    last_tetromino[j][i] = c;
                    board[j][i] = c; 
                    ++i;
                }
                else if(' ' != c)
                    return false;
            }
            ++ccounter;
        }
    }
    return true;
}