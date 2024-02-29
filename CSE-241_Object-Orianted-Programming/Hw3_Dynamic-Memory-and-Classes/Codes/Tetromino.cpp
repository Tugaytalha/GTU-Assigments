#include <iostream>
#include "Tetromino.h"
#define NDEBUG
using namespace std;


bool debug_cout(string sa) {  // function that using for debug
    static int counter = 1;
    #ifndef NDEBUG
    cout << sa << "\033[1;31m" << counter << "\033[0m\n" << endl;
    ++counter;
    #endif
    return true;
}

bool debug_cout(int sa) {  // function that using for debug
    #ifndef NDEBUG
    cout << sa;
    debug_cout();
    #endif
    return true;
}

bool debug_cout(bool sa) {  // function that using for debug
    #ifndef NDEBUG
    if(sa) cout << "true";
    else cout << "false";
    debug_cout();
    #endif
    return sa;
}


namespace Tetris {

string lower(string stri) {
    for(int i = 0; stri[i] != '\0'; ++i) {
        if(stri[i] > 64 && stri[i] < 91) stri[i]+=32;
    }
    return stri;
}

charimino input::take_input() const{
    bool valid = false;
    int num_of_trmn = 0;
    string input_string;
    charimino chariminos;

    cout << "What are the type?" << endl;

    while (!valid) // take tetromino types and save an char vector
    {
        cin >> input_string;
        valid = true;
        if (!((input_string == "I" || input_string == "i" || input_string == "ı") || (input_string == "O" || input_string == "o") || (input_string == "T" || input_string == "t") || (input_string == "J" || input_string == "j") || (input_string == "L" || input_string == "l") || (input_string == "S" || input_string == "s") || (input_string == "Z" || input_string == "z") || (input_string == "R" || input_string == "r")))
        {
            valid = false;
            cout << "Please enter a valid tetromino ('I', 'O', 'T', 'J', 'L', 'S', 'Z', 'R' )." << endl;
        }
        else {
            switch(input_string[0]) {
                
                case 'O':
                case 'o':
                    chariminos = charimino::O;
                    break;
                case 'T':
                case 't':
                    chariminos = charimino::T;
                    break;
                case 'J':
                case 'j':
                    chariminos = charimino::J;
                    break;
                case 'L':
                case 'l':
                    chariminos = charimino::L;
                    break;
                case 'S':
                case 's':
                    chariminos = charimino::S;
                    break;
                case 'Z':
                case 'z':
                    chariminos = charimino::Z;
                    break;
                case 'R':
                case 'r': {
                    charimino arr[7] = {charimino::I, charimino::O, charimino::T, charimino::J, charimino::L, charimino::S, charimino::Z};
                    chariminos = arr[rand() % 7];
                    break;
                }
                default:
                    chariminos = charimino::I;
                    break;
            }
        }
    }
    
    return chariminos;
}

int input::take_number() const {
    bool valid = false;
    int num_of_trmn = 0;
    string input_string;

    while (!valid) // take num_of_tetrominos until it's a natural number
    {
        num_of_trmn = 0;
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
    
    return num_of_trmn;
}

string input::take_direction() const {
    bool valid = false;
    string input_string;

    while (!valid) // take num_of_tetrominos until it's a natural number
    {
        cin >> input_string;
        valid = true;
        input_string = lower(input_string);
        if(input_string != "left" && input_string != "l"  && input_string != "right" && input_string != "r") {
            valid = false;
            cout << "Enter valid direction: ";
        }
    }
    
    return input_string;
}

#pragma region Tetromino
// classic constructor (default = I)
Tetromino::Tetromino(charimino tetro_type) {
    tetro_vector = new char*[4];
    for (int i = 0; i < 4; ++i) {
        tetro_vector[i] = new char[4];
        for (int j = 0; j < 4; ++j) {
            tetro_vector[i][j] = ' ';
        }
    }
    
    switch (tetro_type)
        {  // Fill vector of tetromino according to strong enum class's member
        case charimino::I :
            tetro_vector[3][0] = 'I';
            tetro_vector[3][1] = 'I';
            tetro_vector[3][2] = 'I';
            tetro_vector[3][3] = 'I';
            break;
        case charimino::O :
            tetro_vector[3][0] = 'O';
            tetro_vector[3][1] = 'O';
            tetro_vector[2][0] = 'O';
            tetro_vector[2][1] = 'O';
            break;
        case charimino::T :
            tetro_vector[3][1] = 'T';
            tetro_vector[2][0] = 'T';
            tetro_vector[2][1] = 'T';
            tetro_vector[2][2] = 'T';
            break;
        case charimino::J :
            tetro_vector[3][0] = 'J';
            tetro_vector[3][1] = 'J';
            tetro_vector[2][1] = 'J';
            tetro_vector[1][1] = 'J';
            break;
        case charimino::L :
            tetro_vector[3][1] = 'L';
            tetro_vector[3][0] = 'L';
            tetro_vector[2][0] = 'L';
            tetro_vector[1][0] = 'L';
            break;
        case charimino::S :
            tetro_vector[3][0] = 'S';
            tetro_vector[3][1] = 'S';
            tetro_vector[2][1] = 'S';
            tetro_vector[2][2] = 'S';
            break;
        case charimino::Z :
            tetro_vector[3][2] = 'Z';
            tetro_vector[3][1] = 'Z';
            tetro_vector[2][1] = 'Z';
            tetro_vector[2][0] = 'Z';
            break;
    }
}

Tetromino::~Tetromino() {
    if(tetro_vector != nullptr) {
        for(int i = 0; i < 4; ++i) {
            if(tetro_vector[i] != nullptr) delete[] tetro_vector[i];
        }
        delete[] tetro_vector;
    }
}

// coppy constructor
Tetromino::Tetromino(const Tetromino&o) {
    for (int i = 0; i < 4; ++i) {
        tetro_vector[i] = new char[4];
        for (int j = 0; j < 4; ++j) {
            tetro_vector[i][j] = o.tetro_vector[i][j];
        }
    }
}

// index operator overloading
// char *Tetromino::operator[](int i) {
//     return tetro_vector[i];
// }

// index operator overloading for consts
const char *Tetromino::operator[](const int i) const{
    return tetro_vector[i];
}

// print tetromino on the screen
void Tetromino::print() const {
    for(int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            cout << " " << tetro_vector[i][j];
        }
        cout << endl;
    }
    cout << endl << endl;
}

//rotate the tetromino
void Tetromino::rotate(const string &l_or_r) {
    if (lower(l_or_r) == "left" || lower(l_or_r) == "l") {
        rotate_right();
        rotate_right();
        rotate_right();
    }
    else if (lower(l_or_r) == "right" || lower(l_or_r) == "r")
    {
        rotate_right();
    }
}  

void Tetromino::rotate_right() {
    char ** copy_tet;
    copy_tet = new char*[4];
    copy_tet[0] = new char[4];
    copy_tet[1] = new char[4];
    copy_tet[2] = new char[4];
    copy_tet[3] = new char[4];
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) { 
            copy_tet[j][4 - i -1] = tetro_vector[i][j];
        }    
    }  
    move_vec(copy_tet);
    bool delete_line = true;
    for (int i = 0; i < 4 && delete_line; ++i) {
        delete_line = true;
        for (int j = 0; j < 4; ++j) { 
            if(tetro_vector[j][i] != ' ')
                delete_line = false;
        }    

        if(delete_line){
            delete_collumn();  // delete empty collumn and lean tetromino against left
            i = -1;
        }

    }   
    delete_line = true;
    for (int i = 4 - 1; i > 0 && delete_line; --i) {

        delete_line = true;
        for (int j = 0; j < 4; ++j) { 
            if(tetro_vector[i][j] != ' ')
                delete_line = false;
        }    
        if(delete_line) {
            delete_row();  // delete empty row and 	lean tetromino against bottom
            i = 4;
        }
    }       
};

//delete empty collumn from tetromino array
void Tetromino::delete_collumn() {
    char **temp;
    
    temp = new char*[4];
    temp[0] = new char[4];
    temp[1] = new char[4];
    temp[2] = new char[4];
    temp[3] = new char[4];

    for(int i = 0; i < 4; ++i) {
        for(int j = 4-1; j > 0; --j) {
            temp[i][j-1] = tetro_vector[i][j];
        }
        temp[i][4] = ' ';
    }
    move_vec(temp);
}

//delete empty row from tetromino array
void Tetromino::delete_row() {
    char *temp;
    temp = tetro_vector[3];

    for(int i = 4 -1; i > 0 ; --i) {
        tetro_vector[i] = tetro_vector[i-1];
    }
    tetro_vector[0] = temp;
}

//  delete tetromino vector for new allocation
void Tetromino::move_vec(char **o) {
    for(int i = 0; i < 4; ++i){
        for(int j = 0; j < 4; ++j){ 
            tetro_vector[i][j] = o[i][j]; 
        }
    }
    

    delete []o[0];
    delete []o[1];
    delete []o[2];
    delete []o[3];
    delete []o;
}


#pragma endregion
};





























