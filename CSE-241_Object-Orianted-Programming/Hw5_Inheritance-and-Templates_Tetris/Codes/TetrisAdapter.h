#include <iostream>
#include "AbstractTetris.h"
#include "Tetromino.h"  // For represent Tetriminos
#include "Location.h"  // Takes 2 integer to keep location in board 


#ifndef Adapter_T_H
#define Adapter_T_H

namespace Tetris
{
    template<typename T>
    class TetrisAdapter : virtual public AbstractTetris
    {
    public: // don't need big tree
        // make tetris board with user input wide and high
        TetrisAdapter(const int wide = 10, const int high = 20);

        // Destructor
        ~TetrisAdapter();

        // draw board to screen
        void Draw() const override;

        //  add a Tetromino to the board at the top row in the middle
        bool operator+=(const Tetromino &Tetro) override;

        // Reads the board from a file
        void readFromFile(const std::string &filename) override;

        // Writes the board to a file
        void writeToFile(const std::string &filename) const override;

        // animate drop of tetromino
        bool animate() override;

        // move tetromino horizantal in given number(negatives represent left)
        bool operator>>(int) override;

        // test tetromino movebility in given direction
        bool move_test(char) const override;

        // rotate last added tetromino
        bool Rotate(string) override;

        // Returns the last move made (first int for rotation, second for shift) (negative values represents left)
        std::pair<int, int> lastMove() const override;

        // Returns the number of step
        int numberOfMoves() const override {return step;}

       // Animates the tetromino dropping to the bottom of the board (read from file)
        bool fileanimate(int rotate_no, int move_no) override;
        
    private:
    
        // 3 of these takes 0 initially
        // move tetromino 1 right
        void move_left(int = 0) override;
        // move tetromino 1 left
        void move_right(int = 0) override;
        // move last added tetromino 1 down
        void operator||(int) override;

        // Rotate tetromino on the board
        bool Rotate_right(int index, int minx, int miny, int maxx, int maxy) override;

        T board_vec; // board T
    };
};

#include "TetrisAdapter.tpp"

#endif
