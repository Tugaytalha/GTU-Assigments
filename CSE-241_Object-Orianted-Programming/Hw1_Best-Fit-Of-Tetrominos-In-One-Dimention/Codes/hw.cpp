#include <iostream>
#include <vector>
#include "Tetromino.h"
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
    input input_object;
    vector<charimino> chariminos;
    vector<Tetromino> tetrominos;
    

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

    //canFit test
    if(tetrominos[0].canFit(tetrominos[1], "right"))  cout << "Your first and second tetromino canFit";
    else cout << "Your first and second tetromino cannot Fit";

    Tetromino main_tet(chariminos[0]);
    for(int i = 1; i < so; ++i) 
        main_tet.testAndAdd(tetrominos[i]);
    cout << endl << "Your bestfit:";
    main_tet.print();
    
}

























