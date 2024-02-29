#include <iostream>
#include <vector>
#include "Tetromino.h"
#define NDEBUG
using namespace std;


Tetromino::Tetromino(charimino tetro_type)
{
    switch (tetro_type)
        {  // Fill vector of tetromino according to strong enum class's member
        case charimino::I :
            tetro_vector = I;
            break;
        case charimino::O :
            tetro_vector = O;
            break;
        case charimino::T :
            tetro_vector = T;
            break;
        case charimino::J :
            tetro_vector = J;
            break;
        case charimino::L :
            tetro_vector = L;
            break;
        case charimino::S :
            tetro_vector = S;
            break;
        case charimino::Z :
            tetro_vector = Z;
            break;
    }
}
    


vector<charimino> input::take_input() {
    bool valid = false;
    int num_of_trmn = 0;
    string input_string;
    vector<charimino> chariminos;

    while (!valid) // take num_of_tetrominos until it's a natural number
    {
        num_of_trmn = 0;
        cout << "How many tetrominos?" << endl;
        cin >> input_string;
        valid = true;
        for (auto c : input_string)
        {
            if (!((c - '0') >= 0 && (c - '0') <= 9))
            {
                num_of_trmn = 0;
                break;
            }
            else
                num_of_trmn = num_of_trmn * 10 + (c - '0');
        }
        if (num_of_trmn == 0)
        {
            valid = false;
            cout << "Please enter a valid number." << endl;
        }
    }

    chariminos.resize(num_of_trmn);
    cout << "What are the types?" << endl;
    for (int i = 0; i < num_of_trmn; ++i)
    {
        valid = false;
        while (!valid) // take tetromino types and save an char vector
        {
            cout << i + 1 << ".";
            cin >> input_string;
            valid = true;
            if (!((input_string == "I" || input_string == "i" || input_string == "ı") || (input_string == "O" || input_string == "o") || (input_string == "T" || input_string == "t") || (input_string == "J" || input_string == "j") || (input_string == "L" || input_string == "l") || (input_string == "S" || input_string == "s") || (input_string == "Z" || input_string == "z")))
            {
                valid = false;
                cout << "Please enter a valid tetromino ('I', 'O', 'T', 'J', 'L', 'S', 'Z' )." << endl;
            }
            else {
                switch(input_string[0]) {
                    
                    case 'O':
                    case 'o':
                        chariminos[i] = charimino::O;
                        break;
                    case 'T':
                    case 't':
                        chariminos[i] = charimino::T;
                        break;
                    case 'J':
                    case 'j':
                        chariminos[i] = charimino::J;
                        break;
                    case 'L':
                    case 'l':
                        chariminos[i] = charimino::L;
                        break;
                    case 'S':
                    case 's':
                        chariminos[i] = charimino::S;
                        break;
                    case 'Z':
                    case 'z':
                        chariminos[i] = charimino::Z;
                        break;
                    default:
                        chariminos[i] = charimino::I;
                        break;
                }
            }
        }
    }
    return chariminos;
}

void Tetromino::print() const {
    for(auto i: tetro_vector ) {
        for (auto j : i) {
            cout << " " << j;
        }
        cout << endl;
    }
}

void Tetromino::rotate(const string &l_or_r) {
    if (l_or_r == "left" || l_or_r == "Left" || l_or_r == "l" || l_or_r == "L") {
        rotate_right();
        rotate_right();
        rotate_right();
    }
    else if (l_or_r == "right" || l_or_r == "Right" ||l_or_r == "r" || l_or_r == "R")
    {
        rotate_right();
    }
}

void Tetromino::rotate_right() {
    vector<vector<char>> copy_tet;
    int tetro_size = tetro_vector.size();
    // copy_tet.resize(tetro_size);
    // cout << copy_tet << endl; 
    // for(auto k : copy_tet) {
    //     k.resize(tetro_vector[0].size()+1);
    //     cout << k.size();
    // }
    copy_tet = {{'\0', '\0', '\0', '\0'}, {'\0', '\0', '\0', '\0'}, {'\0', '\0', '\0', '\0'}, {'\0', '\0', '\0', '\0'}};  // for allocation - some errors occurred in resize function
    for (int i = 0; i < tetro_size; ++i) {
        for (int j = 0; j < tetro_size; ++j) { 
            copy_tet[j][tetro_size - i -1] = tetro_vector[i][j];
        }    
    }   
    bool delete_line = true;
    for (int i = 0; i < tetro_size && delete_line; ++i) {
        delete_line = true;
        for (int j = 0; j < tetro_size; ++j) { 
            if(copy_tet[j][i] != ' ')
                delete_line = false;
        }    

        if(delete_line){
            delete_collumn(copy_tet);  // delete empty collumn and 	lean tetromino against left
            i = -1;
        }

    }   
    delete_line = true;
    for (int i = tetro_size - 1; i > 0 && delete_line; --i) {

        delete_line = true;
        for (int j = 0; j < tetro_size; ++j) { 
            if(copy_tet[i][j] != ' ')
                delete_line = false;
        }    
        if(delete_line) {
            delete_row(copy_tet);  // delete empty row and 	lean tetromino against bottom
            i = tetro_size;
        }
    }   
    tetro_vector = copy_tet;       
}

void Tetromino::delete_collumn(vector<vector<char>> &vect) {
    vector<vector<char>> temp;
    temp = {{'\0', '\0', '\0', '\0'}, {'\0', '\0', '\0', '\0'}, {'\0', '\0', '\0', '\0'}, {'\0', '\0', '\0', '\0'}};  // for allocation - some errors occurred in resize function
    int vec2_size = vect.size();
    int vec_size = vect[0].size();

    for(int i = 0; i < vec2_size; ++i) {
        for(int j = vec_size -1; j > 0; --j) {
            temp[i][j-1] = vect[i][j];
        }
        temp[i][vec_size] = ' ';
    }
    vect = temp;
}

void Tetromino::delete_row(vector<vector<char>> &vect) {
    vector<char> temp;
    temp = {' ', ' ', ' ', ' '};  // for allocation - some errors occurred in resize function
    int vec2_size = vect.size();

    for(int i = vec2_size -1; i > 0 ; --i) {
        vect[i] = vect[i-1];
    }
    vect[0] = temp;
}

bool Tetromino::canFit(Tetromino &other, string l_or_r) {
    bool fit = false;
    vector<int> holes;
    
    if (l_or_r == "left" || l_or_r == "Left" || l_or_r == "l" || l_or_r == "L") {
        for(int i = 0; (i < 4) && !(fit); ++i) {
            for(int j = 0; (j < 4) && !(fit); ++j) {
                holes = hole_counter(other.tetro_vector, tetro_vector);
                fit = (holes[0] == holes[1]);
                rotate("right");
            }   
            other.rotate("right");
        }
    }
    else if (l_or_r == "right" || l_or_r == "Right" ||l_or_r == "r" || l_or_r == "R")
    {
        for(int i = 0; (i < 4) && !(fit); ++i) {
            for(int j = 0; (j < 4) && !(fit); ++j) {
                holes = hole_counter(tetro_vector, other.tetro_vector);
                fit = (holes[0] == holes[1]);
                other.rotate("right");
            }   
            rotate("right");
        }
    }
    other.rotate("left");
    rotate("left");
    return fit;
}

vector<int> Tetromino::hole_counter(const vector<vector<char>> &vec1, const vector<vector<char>> &vec2) {  // count hole between tetrominos in bottom rows
    vector<int> holes =  {-5, -5};
    bool in_tetro = false;

    for(auto i : vec1[2]) {
        if(in_tetro && i == ' ') {
            holes[0] = 1;
            in_tetro = false;
        }
        else if (i == ' ') 
            ++holes[0];
        else if (!(in_tetro) && i != ' ')
            in_tetro = true;
   }
    for(auto i : vec2[2]) {
        if(in_tetro && i == ' ') {
            holes[0] = 1;
            in_tetro = false;
        }
        else if (i == ' ') 
            ++holes[0];
        else if (!(in_tetro) && i != ' ')
            break;
    }

    in_tetro = false;
    for(auto i : vec1[3]) {
        if(in_tetro && i == ' ') {
            holes[1] = 1;
            in_tetro = false;
        }
        else if (i == ' ') 
            ++holes[1];
        else if (!(in_tetro) && i != ' ')
            in_tetro = true;
   }
    for(auto i : vec2[3]) {
        if(in_tetro && i == ' ') {
            holes[1] = 1;
            in_tetro = false;
        }
        else if (i == ' ') 
            ++holes[1];
        else if (!(in_tetro) && i != ' ')
            break;
        
   }
   return holes;
}

Tetromino::Tetromino()
{
    tetro_vector = I;
}

bool Tetromino::testAndAdd(Tetromino &other) {
    bool fit = false;
    vector<int> holes;
    size_t so = tetro_vector.size();

    for(int j = 0; (j < 4) && !(fit); ++j) {
        holes = hole_counter(tetro_vector, other.tetro_vector);
        fit = (holes[0] == holes[1]);
        other.rotate("right");
    }   
    other.rotate("left");
    for (int i = 0; i < so; ++i) {
        tetro_vector[i].insert(tetro_vector[i].end(), other.tetro_vector[i].begin(), other.tetro_vector[i].end()); 
    }
    return fit;
}







