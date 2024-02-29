#include <iostream>
#include "Tetromino.h"  // For represent Tetriminos
#include "Location.h"  // Takes 2 integer to keep location in board 

#ifndef abstet
#define abstet

namespace Tetris
{

    class AbstractTetris
    {
    public:
        // Constructor that creates a board with given size
        AbstractTetris(const int width = 10, const int height = 20);

        // Virtual destructor
        virtual ~AbstractTetris();

        // Draws the board on the screen, starting from the top if specified
        virtual void Draw() const = 0;

        // Reads the board from a file
        virtual void readFromFile(const std::string &filename) = 0;

        // Writes the board to a file
        virtual void writeToFile(const std::string &filename) const = 0;

        // Adds a tetromino to the board at the top middle position
        virtual bool operator+=(const Tetromino& Tetro) = 0;

        // move tetromino horizantal in given number(negatives represent left)
        virtual bool operator>>(int) = 0;

        // Animates the tetromino dropping to the bottom of the board
        virtual bool animate() = 0;

        // Returns the last move made (first int for rotation, second for shift) (negative values represents left)
        virtual std::pair<int, int> lastMove() const;

        // Returns the number of moves made
        virtual int numberOfMoves() const = 0;
    
        // test tetromino movebility in given direction
        virtual bool move_test(char) const = 0;

        // rotate last added tetromino  
        virtual bool Rotate(string) = 0;  


        // virtual  operator[](const int i);   // unclear return value
        // virtual operator[](const int i) const;

        // Getter of board wide and high
        const int wide() const{return board_wide;}; 
        const int high() const{return board_high;}; 

        virtual const char last_tetro() const;

        // Animates the tetromino dropping to the bottom of the board (read from file)
        virtual bool fileanimate(int rotate_no, int move_no) = 0;
        
    protected:
        int board_wide, board_high, step;
        Location *last_loc;
        charimino tetro;  // keeps last tetromino
        int noRotate, noShift;

        // 3 of these takes 0 initially   
        // move tetromino 1 right
        virtual void move_left(int = 0) = 0;  
        // move tetromino 1 left
        virtual void move_right(int = 0) = 0;  
        // move last added tetromino 1 down
        virtual void operator||(int) = 0;  

        // Rotate tetromino on the board
        virtual bool Rotate_right(int index, int minx, int miny, int maxx, int maxy) = 0;

    };

} // namespace tetris

#endif