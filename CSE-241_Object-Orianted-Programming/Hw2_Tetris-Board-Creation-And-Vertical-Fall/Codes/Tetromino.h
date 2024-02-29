#include <iostream>
#include <vector>
#define NDEBUG
using namespace std;

enum class charimino : char { I, O, T, J, L, S, Z };  // strong enum class for Tetrominos

class input  // class that holds the input function
{  
public:
    vector<charimino> take_input();
};


class Tetromino
{
public:
    Tetromino(charimino tetro_type);
    Tetromino();
    void print() const;
    void rotate(const string &l_or_r);  //rotate the tetromino
    bool canFit(Tetromino &, string);  // check if any perfect fit between 2 tetrominos
    bool testAndAdd(Tetromino &other);  // check if any perfect
    vector<vector<char>> get_tetro() const;  // return tetromino vector

private:
    vector<int> hole_counter(const vector<vector<char>> &, const vector<vector<char>> &);
    void rotate_right();
    void delete_collumn(vector<vector<char>> &vect);
    void delete_row(vector<vector<char>> &vect);
    vector<vector<char>> tetro_vector;
        
};


static vector<vector<char>> I = {
        {' ', ' ', ' ', ' '},
        {' ', ' ', ' ', ' '},
        {' ', ' ', ' ', ' '},
        {'I', 'I', 'I', 'I'}
        };
     static vector<vector<char>> O = {
        {' ', ' ', ' ', ' '},
        {' ', ' ', ' ', ' '},
        {'O', 'O', ' ', ' '},
        {'O', 'O', ' ', ' '}
        };
     static vector<vector<char>> T = {
        {' ', ' ', ' ', ' '},
        {' ', ' ', ' ', ' '},
        {'T', 'T', 'T', ' '},
        {' ', 'T', ' ', ' '}
        };
     static vector<vector<char>> J = {
        {' ', ' ', ' ', ' '},
        {' ', 'J', ' ', ' '},
        {' ', 'J', ' ', ' '},
        {'J', 'J', ' ', ' '}
        };
     static vector<vector<char>> L = {
        {' ', ' ', ' ', ' '},
        {'L', ' ', ' ', ' '},
        {'L', ' ', ' ', ' '},
        {'L', 'L', ' ', ' '}
        };
     static vector<vector<char>> S = {
        {' ', ' ', ' ', ' '},
        {' ', ' ', ' ', ' '},
        {' ', 'S', 'S', ' '},
        {'S', 'S', ' ', ' '}
        };
     static vector<vector<char>> Z = {
        {' ', ' ', ' ', ' '},
        {' ', ' ', ' ', ' '},
        {'Z', 'Z', ' ', ' '},
        {' ', 'Z', 'Z', ' '}
        };
