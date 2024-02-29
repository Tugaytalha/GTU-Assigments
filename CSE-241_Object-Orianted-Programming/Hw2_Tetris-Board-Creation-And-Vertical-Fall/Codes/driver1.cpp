#include <iostream>
#include <vector>
#include "Tetrominos.cpp"
#include "Tetris.cpp"
#define NDEBUG
using namespace std;


void debug_cout() {  // function that using for debug
    static int counter = 1;
    #ifndef NDEBUG
    cout << counter << endl;
    ++counter;
    #endif
}


int main()
{
    int wide, high;
    input input_object;
    vector<charimino> chariminos;
    vector<Tetromino> tetrominos;
    
    cout << endl << "Please enter board wide:";
    cin >> wide;
    cout << endl << "Please enter board high:";
    cin >> high;

    Tetris tt(wide, high);

    chariminos = input_object.take_input();  // taking tetrominos from user
    cout << endl << "Your Tetrominos:";
    size_t so = chariminos.size();  
    for(int i = 0; i < so; ++i) {  // saving tetrominos 
        tetrominos.push_back(chariminos[i]);
        tetrominos[i].print();
    }
    
    


    //rotates
    cout << endl << "Your first tetromino rotated to the left:";
    tetrominos[0].rotate("left");
    tetrominos[0].print();
    cout << endl << "Your first tetromino rotated to the left again:";
    tetrominos[0].rotate("left");
    tetrominos[0].print();
    cout << endl << "Your first tetromino rotated to the right:";
    tetrominos[0].rotate("right");
    tetrominos[0].print();

    cout << "Your empty board:" << endl;
    tt.Draw();
    cout << "Your board with first tetromino added:" << endl;
    tt.Add(tetrominos[0]);
    tt.Draw();

    
}

























