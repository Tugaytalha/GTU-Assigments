#include <iostream>
using namespace std;

#ifndef Tetromino_T_H

#define Tetromino_T_H
enum class charimino : char { I, O, T, J, L, S, Z };  // strong enum class for Tetrominos

bool debug_cout(string sa ="");

bool debug_cout(int);

bool debug_cout(bool);

namespace Tetris {

string lower(string);  // make lowercase all character in given string

bool tReader(const std::istream &a); // to get input in while expression

bool tReader(const std::ostream &a);

charimino str_to_charmino(string chrmino);  // Convert char to charimino


class Tetromino
{
public:
    Tetromino(charimino tetro_type = charimino::I);
    Tetromino(const Tetromino&);
    ~Tetromino();
    //char *operator[](int);  // return editable Tetromino this is somewhat against the encapsulation principle 
    const char *operator[](const int) const;  // return tetromino piece for consts
    void print() const;
    void rotate(const string &l_or_r = "L");  //rotate the tetromino
    charimino type() const {return tet_type;}

private:
    void rotate_right();
    void delete_collumn();
    void delete_row();
    void move_vec(char **);
    charimino tet_type;
    char **tetro_vector;
        
};

class input  // class that holds the input function
{  
public:
   charimino take_input() const;
   int take_number() const;
   string take_direction() const;
};
};

#endif