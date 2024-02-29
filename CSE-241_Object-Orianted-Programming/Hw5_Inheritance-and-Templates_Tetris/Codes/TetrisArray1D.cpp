#include <fstream>
#include "TetrisArray1D.h"
#include <unistd.h>



namespace Tetris {
    
#pragma region TetrisArray1D

TetrisArray1D::TetrisArray1D(int wid, int hig) :  AbstractTetris(wid, hig) {
    board_vec = new char[wide()*high()];
    for (int i = 0; i < high(); ++i) {
        for (int j = 0; j < wide(); ++j) {
            (*this)[i][j] = ' ';
        }
    }
}

char *TetrisArray1D::operator[](int i) {
    return (board_vec + (i*wide()));
}

const char *TetrisArray1D::operator[](const int i) const{
    return (board_vec + (i*wide()));
}

// draw board to screen
void TetrisArray1D::Draw() const {
    for (int i = -1; i <= high(); ++i) {
        cout << "x ";
        for (int j = 0; j < wide(); ++j) {
            if(-1 == i || high() == i){
                cout << "x ";
            }
            else {
                cout << (*this)[i][j] << " ";
            }
        }
        cout << "x ";
        cout << endl;
    }
    cout << endl << endl << endl;
}

// add a tetrimino to middle top of the board
bool TetrisArray1D::operator+=(const Tetromino& Tetro) {
    int tet_wide = 4;
    bool wide_flag = true;
    tetro = Tetro.type();

    for(int j = 3; j > -1 && wide_flag; --j) {
        for(int i = 0; i < 4 && wide_flag; ++i) {
            if(Tetro[i][j] != ' ') {
                wide_flag = false;
                ++tet_wide;
            }
        } 
        --tet_wide;
    }

    int ccounter, i = wide() / 2 - tet_wide/2, j = 0, loc_counter = 0;
    bool flag = false, return_value = true;

    
    for(int vrow = 0; vrow < 4 && return_value; ++vrow) {
        ccounter = 0;
        if(flag) {
            ++j;
            i = wide() / 2 - tet_wide/2;
            flag = false;
        }
        for(int c  = 0; c < 4 && return_value; ++c) {
            if(!flag ) {  // until current on tetromino letter
                if (Tetro[vrow][c] !=' '){
                    flag = true;
                    for (int k = ccounter; k > 0; --k) {
                        //last_tetromino[j][i] = ' ';
                        (*this)[j][i++] = ' ';
                    }
                    last_loc[loc_counter].moved = false;
                    last_loc[loc_counter].y = i;
                    last_loc[loc_counter++].x = j;
                    
                    (*this)[j][i] = Tetro[vrow][c];
                    ++i;
                }
            }
            else {
                if (' ' == (*this)[j][i]) {  
                    //last_tetromino[j][i] = Tetro[vrow][c];
                    if (Tetro[vrow][c] != ' '){
                        last_loc[loc_counter].moved = false;
                        last_loc[loc_counter].y = i;
                        last_loc[loc_counter++].x = j;
                        (*this)[j][i] = Tetro[vrow][c]; 
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
bool TetrisArray1D::operator>>(int number) {
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
bool TetrisArray1D::move_test(char LorRorD) const{
    bool can_move = true;

    if(LorRorD == 'l') { // can move to left
        for(int i = 0; i < 4 && can_move; ++i) {
            if(last_loc[i].y - 1 <= 0) can_move = false;
            else if((*this)[last_loc[i].x][last_loc[i].y - 1] != ' '){
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
            if(last_loc[i].y + 1 >= wide()) can_move = false;
            else if((*this)[last_loc[i].x][last_loc[i].y + 1] != ' '){
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
            if(last_loc[i].x + 1 >= high()) can_move = false;
            else if((*this)[last_loc[i].x + 1][last_loc[i].y] != ' '){
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
            if(diffx - j + minx >= high() || (i + miny) >= wide() ||(i + miny) <= 0) can_move = false;
            else if((*this)[diffx - j + minx][(i + miny)] != ' '){
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
            if(j + minx >= high() || ((diffy - i) + miny) >= wide() ||((diffy - i) + miny) < 0) can_move = false;
            else if((*this)[j + minx][((diffy - i) + miny)] != ' '){
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
void TetrisArray1D::operator||(int index) {    
    bool inner_recursive = false;  
    if (index < 0) {inner_recursive = true; index*=-1;}

    if (false == last_loc[index].moved) {
        for(int j = 0; j < 4; ++j) {
            if(((last_loc[index] >= last_loc[j]))) {
                (*this)||-j;
            }
        }
        (*this)[last_loc[index].x + 1][last_loc[index].y] = (*this)[last_loc[index].x][last_loc[index].y];
        (*this)[last_loc[index].x][last_loc[index].y] = ' ';
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
void TetrisArray1D::move_right(int index) {
    bool inner_recursive = false;  
    if (index < 0) {inner_recursive = true; index*=-1;}

    if (false == last_loc[index].moved) {
        for(int j = 0; j < 4; ++j) {  // if any other piece in the right move it first
            if(((last_loc[index] > last_loc[j]))) {  
                move_right(-j);
            }
        }
        (*this)[last_loc[index].x][last_loc[index].y + 1] = (*this)[last_loc[index].x][last_loc[index].y];
        (*this)[last_loc[index].x][last_loc[index].y] = ' ';
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
void TetrisArray1D::move_left(int index) {
    bool inner_recursive = false;  
    if (index < 0) {inner_recursive = true; index*=-1;}

    if (false == last_loc[index].moved) {
        for(int j = 0; j < 4; ++j) {  // if any other piece in the right move it first
            if(((last_loc[index] < last_loc[j]))) {  
                move_left(-j);
            }
        }
        (*this)[last_loc[index].x][last_loc[index].y - 1] = (*this)[last_loc[index].x][last_loc[index].y];
        (*this)[last_loc[index].x][last_loc[index].y] = ' ';
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

// ask user and animate tetromino dropping
bool TetrisArray1D::animate() {
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
    noShift = move_no;

    for(rotate_no %= 4; rotate_no > 0 && (valid = move_test((rotate_string == "l" ||rotate_string == "left") ? '1' : '2')); --rotate_no) {Rotate(rotate_string); }
    noRotate = rotate_no;
    
    if (!valid)  return false;
    if(!((*this)>>move_no))  cout << "Couldn't as shift as want beacuse of impossible" << endl;
    usleep(500 * 1000);


    while (move_test('d'))
    {
        //step 5
        Draw();
        //step 6
        usleep(500 * 1000);
        //step 7
        (*this)||0;
    }
    ++step;
    try
    {
        this->writeToFile("Tetrissave");  // file extension intentionally left blank
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        usleep(1000 * 1000);
    }
    
    Draw();
    
    return true; 
}

// animate tetromino dropping 
bool TetrisArray1D::fileanimate(int rotate_no, int move_no) {
    bool valid = true;
    Draw();
    
    if (!valid)  return false;
    if(!((*this)>>move_no))  cout << "Couldn't as shift as want beacuse of impossible" << endl;

    for(rotate_no %= 4; rotate_no > 0 && (valid = move_test('2')); --rotate_no) {Rotate("r"); }
noShift = move_no;
noRotate = rotate_no;
    while (move_test('d'))
    {
        Draw();
        (*this)||0;
    }
    Draw();
    
    return true; 
}

bool TetrisArray1D::Rotate(string LorR) {
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
bool TetrisArray1D::Rotate_right(int index, int minx, int miny, int maxx, int maxy) {
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
        
        if(j + minx >= high() || ((diffy - i) + miny) >= wide() ||((diffy - i) + miny) < 0)  {return debug_cout(false);}
        else if(j + minx == last_loc[index].x && (diffy - i) + miny == last_loc[index].y) {last_loc[index].moved = true; ok = false;}
        else if((*this)[j + minx][(diffy - i) + miny] != ' ') {
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
            (*this)[j + minx][(diffy - i) + miny] = (*this)[i + minx][j + miny];
            (*this)[i + minx][j + miny] = ' ';
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

TetrisArray1D::~TetrisArray1D() {
    delete[] board_vec;
    // and last_loc will be deleted by AbstractTetris's destructor (auto call)
}

void TetrisArray1D::writeToFile(const std::string& filename) const{
    std::fstream saveFile;
    saveFile.open(filename, std::ios_base::app);
    std::pair<int, int> rot_shift;
    try
    {
        rot_shift = this->lastMove();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        usleep(1000 * 1000);
        return;
    }
    
    if(saveFile.is_open() == false) {
        throw std::ios_base::failure("File can't open\n");
    }

    saveFile << this->numberOfMoves() << " " << this->last_tetro() << " " << rot_shift.first << " " << rot_shift.second << std::endl;

    saveFile.close();

}

void TetrisArray1D::readFromFile(const std::string &filename) {
    int nRot, nMove, ccc = 0;
    string tetType;
    std::fstream saveFile;
    saveFile.open(filename, std::ios_base::in);

    if(saveFile.is_open() == false) {
        throw std::ios_base::failure("File can't open\n");
    }

    while (tReader(saveFile >> step >> tetType >> nRot >> nMove) && !saveFile.eof() && ccc < 10)
    {
        Tetromino Tet(tetro = str_to_charmino(tetType));  // create a tetromino (assigment operator (=) returns assigned value)
        (*this) += Tet;
        this->fileanimate(nRot, nMove);
        ccc++;
    }

    saveFile.close();
}

std::pair<int, int> TetrisArray1D::lastMove() const {
    if (noRotate == -9994) throw std::invalid_argument("There is no last move"); 
    return pair<int, int>(noRotate, noShift);
}


#pragma endregion

};
