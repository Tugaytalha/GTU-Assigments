#include <iostream>
#include "Tetromino.h"
#include "Board.h"
#include <time.h>
#define NDEBUG
using namespace std;

/* in this case we can use also "using namespace Tetris;" 
 * instead of writing Tetris:: again and again 
 */
int main() {
    // Providing a seed value
	srand((unsigned) time(NULL));

    Tetris::Tetromino a(charimino::S);  
    cout << "Tetromino:" << endl;
    a.print();
    cout << "Tetromino rotated left:" << endl;
    a.rotate();
    a.print();
    cout << "Tetromino rotated left x2:" << endl;
    a.rotate();
    a.print();

    Tetris::Board sa(10,20);
    string temp;
    cout << "Empty board:" << endl;
    sa.Draw();

    cout << "Press enter";
    getline(cin, temp);
    cout << "Tetromino added:" << endl;
    sa += a;
    sa.Draw();
    cout << "Tetromino rotated right:" << endl;
    sa.Rotate("r");
    sa.Draw();

    cout << "Press enter for animate";
    getline(cin, temp);
    sa.animate();
    
    cout << "Press enter for fast random play";
    getline(cin, temp);
    Tetris::Board as(30,30);
    charimino arr[7] = {charimino::I, charimino::O, charimino::T, charimino::J, charimino::L, charimino::S, charimino::Z};
    for (size_t i = 0; i < 120; i++)
    {
        Tetris::Tetromino t(arr[rand() % 7]);
        as += t;
        int tt = rand() % 4;
        for(i = 0; i < tt; ++i)
            t.rotate();
        while(as.move_test('d')){
            as||0;
            as >> rand() % 15 - 7;
            if(i % 12 == 0)as.Draw();
        }
    }
    as.Draw();
    
    

    return 0;
}