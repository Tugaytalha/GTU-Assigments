#include <iostream>
#include "Board.h"
#include <unistd.h>
using namespace std;

namespace Tetris {
#pragma region Board
// Calls other constructor that takes wide and high with 10 wide and 20 high
Board::Board() : Board(10, 20) {} 

// Makes tetris board
Board::Board(const int wide, const int high) {
    board_high = high;
    board_wide = wide;

    board_vec = new char*[board_high]; 
    for(int i = 0; i < board_high; ++i) {
        board_vec[i] = new char[board_wide]; 
        for (int j = 0; j < board_wide; ++j)
            board_vec[i][j] = ' ';
    }
    last_loc = new Location[4];
}

// draw board to screen
void Board::Draw() const {
    
    for (int i = -1; i <= board_high; ++i) {
        cout << "x ";
        for (int j = 0; j < board_wide; ++j) {
            if(-1 == i || board_high == i){
                cout << "x ";
            }
            else {
                cout << board_vec[i][j] << " ";
            }
        }
        cout << "x ";
        cout << endl;
    }
    cout << endl << endl << endl;
}

// add a tetrimino to middle top of the board
bool Board::operator+=(const Tetromino& Tetro) {
    int tet_wide = 4;
    bool wide_flag = true;
    for(int j = 3; j > -1 && wide_flag; --j) {
        for(int i = 0; i < 4 && wide_flag; ++i) {
            if(Tetro[i][j] != ' ') {
                wide_flag = false;
                ++tet_wide;
            }
        } 
        --tet_wide;
    }

    int ccounter, i = board_wide / 2 - tet_wide/2, j = 0, loc_counter = 0;
    bool flag = false, return_value = true;

    
    for(int vrow = 0; vrow < 4 && return_value; ++vrow) {
        ccounter = 0;
        if(flag) {
            ++j;
            i = board_wide / 2 - tet_wide/2;
            flag = false;
        }
        for(int c  = 0; c < 4 && return_value; ++c) {
            if(!flag ) {  // until current on tetromino letter
                if (Tetro[vrow][c] !=' '){
                    flag = true;
                    for (int k = ccounter; k > 0; --k) {
                        //last_tetromino[j][i] = ' ';
                        board_vec[j][i++] = ' ';
                    }
                    last_loc[loc_counter].moved = false;
                    last_loc[loc_counter].y = i;
                    last_loc[loc_counter++].x = j;
                    
                    board_vec[j][i] = Tetro[vrow][c];
                    ++i;
                }
            }
            else {
                if (' ' == board_vec[j][i]) {  
                    //last_tetromino[j][i] = Tetro[vrow][c];
                    if (Tetro[vrow][c] != ' '){
                        last_loc[loc_counter].moved = false;
                        last_loc[loc_counter].y = i;
                        last_loc[loc_counter++].x = j;
                        board_vec[j][i] = Tetro[vrow][c]; 
                        ++i;
                    }
                }
                else if(' ' != Tetro[vrow][c])
                    return_value = false;
            }
            ++ccounter;
        }
    }

    return return_value;
}

// move tetromino horizantal in given number(negatives represent left)
bool Board::operator>>(int number) {
    bool return_value;
    if(number < 0) {
        for(int i = 0; i > number &&(return_value = debug_cout(move_test('l'))) ; --i)
            move_left(0);
    }
    else {
        for(int i = 0; i < number && (return_value = move_test('r')); ++i)
            move_right(0);
    }
    
    return return_value;
}

// test tetromino movebility in given direction
bool Board::move_test(char LorRorD) const{
    bool can_move = true;

    if(LorRorD == 'l') { // can move to left
        for(int i = 0; i < 4 && can_move; ++i) {
            if(last_loc[i].y - 1 <= 0) can_move = false;
            else if(board_vec[last_loc[i].x][last_loc[i].y - 1] != ' '){
                can_move = false;
                for(int j = 0; j < 4 && !can_move; ++j) {
                    if((i != j && (last_loc[i]<last_loc[j]))) {
                        can_move = true;
                    }
                }
            }
        }
    }
    else if(LorRorD == 'r'){ // can move to right
        for(int i = 0; i < 4 && can_move; ++i) {
            if(last_loc[i].y + 1 >= board_wide) can_move = false;
            else if(board_vec[last_loc[i].x][last_loc[i].y + 1] != ' '){
                can_move = false;
                for(int j = 0; j < 4 && !can_move; ++j) {
                    if((i != j && (last_loc[i]>last_loc[j]))) {
                        can_move = true;
                    }
                }
            }
        }
    }
    else if (LorRorD == 'd'){  // can move to down
        for(int i = 0; i < 4 && can_move; ++i) {
            if(last_loc[i].x + 1 >= board_high) can_move = false;
            else if(board_vec[last_loc[i].x + 1][last_loc[i].y] != ' '){
                can_move = false;
                for(int j = 0; j < 4 && !can_move; ++j) {
                    if((i != j && (last_loc[i] >= last_loc[j]))) {
                        can_move = true;
                    }
                }
            }
        }
    }
    else if (LorRorD == '1'){  // can rotate to left
        for(int index = 0; index < 4 && can_move; ++index) {
            int i, j, diffx, diffy, maxx = INT32_MIN, maxy = INT32_MIN, minx = INT32_MAX, miny = INT32_MAX;
            for(int f = 0; f < 4; ++f) {
                if (minx > last_loc[f].x) minx = last_loc[f].x;
                else if (maxx < last_loc[f].x) maxx = last_loc[f].x;
                if (miny > last_loc[f].y) miny = last_loc[f].y;
                else if (maxy < last_loc[f].y) maxy = last_loc[f].y;
            }

            i = last_loc[index].x - minx;
            j = last_loc[index].y - miny;
            diffx = maxx - minx;
            diffy = maxy - miny;
            if(diffx - j + minx >= board_high || (i + miny) >= board_wide ||(i + miny) <= 0) can_move = false;
            else if(board_vec[diffx - j + minx][(i + miny)] != ' '){
                can_move = false;
                for(int index2 = 0; index2 < 4 && !can_move; ++index2) {
                    if(index != index2 && diffx - j + minx == last_loc[index2].x && i + miny == last_loc[index2].y) {
                        can_move = true;
                    }
                }
            }
        }
    }
    else if (LorRorD == '2'){  // can rotate to right
        for(int index = 0; index < 4 && can_move; ++index) {
            int i, j, diffx, diffy, maxx = INT32_MIN, maxy = INT32_MIN, minx = INT32_MAX, miny = INT32_MAX;
            for(int f = 0; f < 4; ++f) {
                if (minx > last_loc[f].x) minx = last_loc[f].x;
                if (maxx < last_loc[f].x) maxx = last_loc[f].x;
                if (miny > last_loc[f].y) miny = last_loc[f].y;
                if (maxy < last_loc[f].y) maxy = last_loc[f].y;
            }

            i = last_loc[index].x - minx;
            j = last_loc[index].y - miny;
            diffx = maxx - minx;
            diffy = maxy - miny;
            if(j + minx >= board_high || ((diffy - i) + miny) >= board_wide ||((diffy - i) + miny) < 0) can_move = false;
            else if(board_vec[j + minx][((diffy - i) + miny)] != ' '){
                can_move = false;
                for(int index2 = 0; index2 < 4 && !can_move; ++index2) {
                    if(j + minx == last_loc[index2].x && (diffy - i) + miny == last_loc[index2].y) {
                        can_move = true;
                    }
                }
            }
        }
    }

    return can_move;
}

// moves the tetromino down a notch
void Board::operator||(int index) {    
    bool inner_recursive = false;  
    if (index < 0) {inner_recursive = true; index*=-1;}

    if (false == last_loc[index].moved) {
        for(int j = 0; j < 4; ++j) {
            if(((last_loc[index] >= last_loc[j]))) {
                (*this)||-j;
            }
        }
        board_vec[last_loc[index].x + 1][last_loc[index].y] = board_vec[last_loc[index].x][last_loc[index].y];
        board_vec[last_loc[index].x][last_loc[index].y] = ' ';
        ++last_loc[index].x;
        last_loc[index].moved = true;
    }
    if (index != 3 && !inner_recursive)
        (*this)||(index + 1);
    else if(!inner_recursive) { // if this is last call reset all moved
        last_loc[0].moved =
        last_loc[1].moved =
        last_loc[2].moved =
        last_loc[3].moved = false;
    } 
}

//moves the tetromino right a notch
void Board::move_right(int index) {
    bool inner_recursive = false;  
    if (index < 0) {inner_recursive = true; index*=-1;}

    if (false == last_loc[index].moved) {
        for(int j = 0; j < 4; ++j) {  // if any other piece in the right move it first
            if(((last_loc[index] > last_loc[j]))) {  
                move_right(-j);
            }
        }
        board_vec[last_loc[index].x][last_loc[index].y + 1] = board_vec[last_loc[index].x][last_loc[index].y];
        board_vec[last_loc[index].x][last_loc[index].y] = ' ';
        ++last_loc[index].y;
        last_loc[index].moved = true;
    }
    if (index != 3 && !inner_recursive)
        move_right(index + 1);
    else if(!inner_recursive) { // if this is last call reset all moved
        last_loc[0].moved =
        last_loc[1].moved =
        last_loc[2].moved =
        last_loc[3].moved = false;
    } 
    
}

//moves the tetromino left a notch
void Board::move_left(int index) {
    bool inner_recursive = false;  
    if (index < 0) {inner_recursive = true; index*=-1;}

    if (false == last_loc[index].moved) {
        for(int j = 0; j < 4; ++j) {  // if any other piece in the right move it first
            if(((last_loc[index] < last_loc[j]))) {  
                move_left(-j);
            }
        }
        board_vec[last_loc[index].x][last_loc[index].y - 1] = board_vec[last_loc[index].x][last_loc[index].y];
        board_vec[last_loc[index].x][last_loc[index].y] = ' ';
        --last_loc[index].y;
        last_loc[index].moved = true;
    }
    if (index != 3 && !inner_recursive)
        move_left(index + 1);
    else if(!inner_recursive) { // if this is last call reset all moved
        last_loc[0].moved =
        last_loc[1].moved =
        last_loc[2].moved =
        last_loc[3].moved = false;
    } 

};

bool Board::animate() {
    string rotate_string, move_string;
    int rotate_no, move_no;
    bool valid = true;
    //step 1
    Draw();

    //step 2
    cout << "rotate Which direction and How many times? " << endl;
    cin >> rotate_string >> rotate_no;
    //step 3
    cout << "move Which direction and How many times? " << endl;
    cin >> move_string >> move_no;

    //step 4
    move_no *= (lower(move_string) == "l" || lower(move_string) == "left") ? -1 : 1;
    if (lower(rotate_string) == "l" || rotate_string == "left" ) {rotate_string = "r"; rotate_no *=3;}

    for(rotate_no %= 4; rotate_no > 0 && (valid = move_test((rotate_string == "l" ||rotate_string == "left") ? '1' : '2')); --rotate_no) {Rotate(rotate_string); }
    
    if (!valid)  return false;
    if(!((*this)>>move_no))  cout << "Couldn't as shift as want beacuse of impossible" << endl;


    while (move_test('d'))
    {
        //step 5
        Draw();
        //step 6
        usleep(500 * 1000);
        //step 7
        (*this)||0;
         
    }
    Draw();
    
    return true; 
}

bool Board::Rotate(string LorR) {
    if (lower(LorR) == "l" || lower(LorR) == "left") {
        Rotate_right(0,INT32_MAX, INT32_MAX, INT32_MIN, INT32_MIN);
        Rotate_right(0,INT32_MAX, INT32_MAX, INT32_MIN, INT32_MIN);
        return Rotate_right(0,INT32_MAX, INT32_MAX, INT32_MIN, INT32_MIN);
    }
    else {
        return Rotate_right(0,INT32_MAX, INT32_MAX, INT32_MIN, INT32_MIN);
    }


}


//recursive function that rotate last added tetromino on the board to right
bool Board::Rotate_right(int index, int minx, int miny, int maxx, int maxy) {
    bool inner_recursive = false, ok = true;  
    if (index < 0) {inner_recursive = true; index*=-1;}
    if (false == last_loc[index].moved) {
        int i, j, diffx, diffy;
        for(int f = 0; f < 4 && !last_loc[f].moved; ++f) {
            if (minx > last_loc[f].x) minx = last_loc[f].x;
            if (maxx < last_loc[f].x) maxx = last_loc[f].x;
            if (miny > last_loc[f].y) miny = last_loc[f].y;
            if (maxy < last_loc[f].y) maxy = last_loc[f].y;
        }

        i = last_loc[index].x - minx;
        j = last_loc[index].y - miny;
        diffx = maxx - minx;
        diffy = maxy - miny;
        
        if(j + minx >= board_high || ((diffy - i) + miny) >= board_wide ||((diffy - i) + miny) < 0)  {return debug_cout(false);}
        else if(j + minx == last_loc[index].x && (diffy - i) + miny == last_loc[index].y) {last_loc[index].moved = true; ok = false;}
        else if(board_vec[j + minx][(diffy - i) + miny] != ' ') {
            ok = false;
            for(int index2 = 0; index2 < 4; ++index2) {
                if(index != index2 && j + minx == last_loc[index2].x && (diffy - i) + miny == last_loc[index2].y) {
                    if (index2 == 0) {
                        last_loc[0].moved = true;
                        last_loc[1].moved = true;
                        last_loc[2].moved = true;
                        last_loc[3].moved = true;
                        ok = false;
                        }
                    else    {
                    Rotate_right(-index2, minx, miny, maxx, maxy);
                    ok = true;
                    }
                }
            }
        }

        if(ok) {
            if(last_loc[index].moved == false){
            board_vec[j + minx][(diffy - i) + miny] = board_vec[i + minx][j + miny];
            board_vec[i + minx][j + miny] = ' ';
            last_loc[index].x = j + minx;
            last_loc[index].y = (diffy - i) + miny;
            last_loc[index].moved = true;
            }
        }   
    }

    if (index != 3 && !inner_recursive)
        Rotate_right(index + 1, minx, miny, maxx, maxy);
    else if(!inner_recursive) { // if this is last call reset all moved
        last_loc[0].moved =
        last_loc[1].moved =
        last_loc[2].moved =
        last_loc[3].moved = false;
    } 
    return true;
}

Board::~Board() {
    if(board_vec != nullptr) {
        for(int i = 0; i < board_high;++i) {
            if (board_vec[i] != nullptr) delete []board_vec[i];
        }
        delete []board_vec;
    }
    if (last_loc != nullptr) delete []last_loc;
}



#pragma endregion

#pragma region Location
// is other in 1 under
bool Location::operator>=(const Location& o) const{
    return ((x == (o.x - 1)) && (y == o.y));
}

// is other in 1 above
bool Location::operator<=(const Location&o) const{
    return ((x == o.x + 1) && (y == o.y));
}

// is other in 1 right
bool Location::operator>(const Location&o) const{
    return ((x == o.x) && (y == o.y - 1));
}  

// is other in 1 left
bool Location::operator<(const Location&o) const{
    return ((x == o.x) && (y == o.y + 1));
}  


#pragma endregion

};