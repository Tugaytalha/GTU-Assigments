#include "Tetris.h"

using namespace std;

bool test_tetris(int board_type) {
    // Providing a seed value
	srand((unsigned) time(NULL));

    Tetris::input input_object;
    Tetris::AbstractTetris *game_board;
    int wide,high;
    bool valid = true;
    string cstr;
    std::fstream saveFile;
    saveFile.open("Tetrissave", std::ios_base::out);
    saveFile.close();
    

    wide = 10;
    high = 20;
    switch(board_type)
    {
    case 1:
        game_board = new Tetris::TetrisVector;
        break;
    
    case 2:
        game_board = new Tetris::TetrisArray1D;
        break;
    
    case 3:
        game_board = new Tetris::TetrisAdapter<deque<deque <char> > >;
        break;
    
    case 4:
        game_board = new Tetris::TetrisAdapter<vector<vector <char> > >;
        break;
    
    default:
        break;
    }
    
    try
    {
        game_board->lastMove();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    
    cout << "Press enter 1-2";
    getline(cin, cstr);
    getline(cin, cstr);
    // std::fstream saveFile;
    // saveFile.open("Tetrissave", std::ios_base::out);
    // saveFile.close();


    for(int i = 0; i < 5; ++i) {
        Tetris::Tetromino Tet(Tetris::str_to_charmino("R"));
        valid = ((*game_board) += Tet);
        (*game_board).Rotate("r");
        usleep(1000 * 1000);
        game_board->fileanimate(rand()%4, rand()%10-5);  // uses operator +=
        game_board->writeToFile("Tetrissave");
    }
    (*game_board).Draw();

    cout << "Last Move First: " <<  game_board->lastMove().first << " " <<"Last Move Second: " << game_board->lastMove().second << endl;

    cout << "Press enter 1-2";
    getline(cin, cstr);
    getline(cin, cstr);
 
    cout << "reset board and read from another file" << endl;
    usleep(2000 * 1000);
    delete game_board;
   switch(board_type)
    {
    case 1:
        game_board = new Tetris::TetrisVector;
        break;
    
    case 2:
        game_board = new Tetris::TetrisArray1D;
        break;
    
    case 3:
        game_board = new Tetris::TetrisAdapter<deque<deque <char> > >;
        break;
    
    case 4:
        game_board = new Tetris::TetrisAdapter<vector<vector <char> > >;
        break;
    
    default:
        break;
    }

    game_board->readFromFile("Saveback");
    game_board->Draw();

    cout << "Back to the first game" << endl;
        cout << "Press enter 1-2";
    getline(cin, cstr);
    getline(cin, cstr);
 
     delete game_board;
   switch(board_type)
    {
    case 1:
        game_board = new Tetris::TetrisVector;
        break;
    
    case 2:
        game_board = new Tetris::TetrisArray1D;
        break;
    
    case 3:
        game_board = new Tetris::TetrisAdapter<deque<deque <char> > >;
        break;
    
    case 4:
        game_board = new Tetris::TetrisAdapter<vector<vector <char> > >;
        break;
    
    default:
        break;
    }   

    game_board->readFromFile("Tetrissave");
    game_board->Draw(); 

    cout << "number of moves : " << game_board->numberOfMoves() << endl << endl;  

    cout << "Press enter 1-2";
    getline(cin, cstr);
    getline(cin, cstr);
    return true;
}

bool play_tetris(int board_type) {
    // Providing a seed value
	srand((unsigned) time(NULL));

    Tetris::input input_object;
    Tetris::AbstractTetris *game_board;
    int wide,high;
    bool valid = true;
    string cstr;

    
    cout << "Enter board wide: ";
    while((wide = input_object.take_number()) < 10) cout << "Enter a number greater than 10: ";
    cout << "Enter board high: ";
    while((high = input_object.take_number()) < 10) cout << "Enter a number greater than 10: ";
    switch (board_type)
    {
    case 1:
        game_board = new Tetris::TetrisVector;
        break;
    
    case 2:
        game_board = new Tetris::TetrisArray1D;
        break;
    
    case 3:
        game_board = new Tetris::TetrisAdapter<deque<deque <char> > >;
        break;
    
    case 4:
        game_board = new Tetris::TetrisAdapter<vector<vector <char> > >;
        break;
    
    default:
        break;
    }
    
    cout << "Do you wanna continue the last game? [Y/n]: ";
    cin >> cstr;
    if (Tetris::lower(cstr) == "y") game_board->readFromFile("Tetrissave");

    std::fstream saveFile;
    saveFile.open("Tetrissave", std::ios_base::out);
    saveFile.close();


    while(valid) {
        Tetris::Tetromino Tet(input_object.take_input());
        valid = ((*game_board) += Tet);
        (*game_board).Rotate("r");
        if (valid) (*game_board).animate();
    }
    (*game_board).Draw();
    system("clear");
    cout << " \033[15B" << "\033[40C" << "Game over!!!" << " \033[10B"; 

    return false;
}