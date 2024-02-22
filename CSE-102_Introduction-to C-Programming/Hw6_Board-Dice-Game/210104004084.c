#include <stdio.h>
#include <time.h>
#define ROW 15
#define COL 30

int abs(int a);

int rand();

void srand(unsigned int seed);

int dice();

/* prepare the game area for play */
void make_game_area(char area[ROW+1][COL+1]);

/* printing game area */
void printMap (char map[ROW+1][COL+1], int r, int c);

/* decide to who is start */
int startGame();

int play_game(char game_area[ROW+1][COL+1], int starter);

/* function for Player 1's move */
int p1_plays(char game_area[ROW+1][COL+1], int p1x, int p1y, int dicem);

/* function for Player 1's move */
int p2_plays(char game_area[ROW+1][COL+1], int p2x, int p2y, int dicem);

/* this function checking penalty and put player to game_area */ 
void put_player(char game_area[ROW+1][COL+1], int px, int py, int player_num);

void print_penalty(int player_num);


int main () {
    int i, j, ff = 1, notexlt = 1, starter, winner, t = 1, c1 = 0, c2 = 0;
    char game_area[ROW+1][COL+1], c;
    printf("\033[0m"); // reset terminal color

    while (notexlt) {

        make_game_area(game_area);
        printMap(game_area, 15, 30);

        printf("\nPlayer 1: %d",c1);
        printf("\nPlayer 2: %d\n",c2);
        printf("\n\nTo start to game, players should dice and decide who is going to start first according to it...\n");
        starter = startGame();
        winner = play_game(game_area, starter);

        printf("\033[0;3%dm",winner+2); // Set the text to the color player's color
        printf("\n\n\n\n *** PLAYER %d WON THE GAME ***\n", winner);
        if (winner == 1) c1++;
        else c2++;
        printf("\033[0m"); // reset color
        printMap(game_area, 15, 30);

        t = 1;
        while (t) { // Ask to user want play again or no
            printf(
                "\n\nPress 1 and enter for Play again\n"
                "Press 0 and enter for Exit.\n"
                "------------------------------\n"
                );
            printf("\nYOUR CHOICE: ");
            if(scanf("%d",&notexlt) != 1) {
                printf("ERROR, please enter a valid entry."); // if input isn't valid(char etc.) will print this
                ff = 0;
            }		
            while((c = getchar()) != '\n' && c != EOF); // waiting to entry like fflush, beacuse if entry a char program will try to read that all time  
            if ((notexlt>1 || notexlt<0) && ff) printf("Wrong code, please enter again.");
            else if (ff) t = 0;
            else ff = 1;
        }
    }
}


/* prepare the game area for play */
void make_game_area(char area[ROW+1][COL+1]) {
    int i,j;
    /* temporary location arrays for a more organized creating */
    int pL[2][2] = {{1,1},{3,3}}, fL[2][2] = {{2,1},{4,3}}, penalties[9][2] = {{1,14},{7,28},{13,14},{3,10},{3,18},{5,26},{10,26},{11,10},{11,28}};
    /* making game area */
    for (i = 0; i < ROW; i++) {
        for (j = 0; j < COL; j++) {
            if (i == 0 || i == ROW-1 || ((i == 2 || i == ROW-3) && j<28 && j>1) || ((i == 4 || i == ROW-5) && j<26 && j>3))
                area[i][j] = '.';
            else if (j == 0 || j == COL-1 || ((j == 2 || j == COL-3) && i<13 && i>1) || ((j == 4 || j == COL-5) && i<11 && i>3))
                area[i][j] = ':';
            else
                area[i][j] = ' ';
        }
    }
    for (i = 0; i < 9; i++) {
        area[penalties[i][0]][penalties[i][1]] = 'X'; //Penalty points
    }
    area[fL[0][0]][fL[0][1]] = area[fL[1][0]][fL[1][1]] = '_'; //Finish lines
    area[pL[0][0]][pL[0][1]] = '1'; //player 1
    area[pL[1][0]][pL[1][1]] = '2'; //player 2
}

/* printing game area */
void printMap (char map[ROW+1][COL+1], int r, int c)  { // r = number of rows, c number of collumns
    int i,j;
    char mc;
    for (i = 0; i<r; i++) {
        for (j = 0; j<c; j++) {
            mc = map[i][j];
            if (mc == 'X') {
                printf("\033[0;31m"); // Set the text to the color red
                printf("%c", mc);
                printf("\033[0m"); // reset color
            }
            else if (mc == '_') {
                printf("\033[0;32m"); // Set the text to the color green
                printf("%c", mc);
                printf("\033[0m"); // reset color
            }
            else if (mc == '1') {
                printf("\033[5;33m"); // Set the text to the color yellow-brown dimming
                printf("%c", mc);
                printf("\033[0m"); // reset color
            }
            else if (mc == '2') {
                printf("\033[5;34m"); // Set the text to the color dimming blue
                printf("%c", mc);
                printf("\033[0m"); // reset color
            }
            else {
                printf("%c",mc);
            }
            
        }   
        printf("\n");     
    }
}

int dice() { // function to dice 
    srand(time(NULL));
    return rand()%6 + 1;
}

/* decide to who is start */
int startGame() {
    int starter, i, j, flag = 1;
    while (flag) 
    {
        printf("\033[0;3%dm",3); // Set the text to the color yellow
        printf("PLAYER 1... press ENTER to dice");
        getchar();
        i = dice();
        printf("DICE: %d\n",i);
        printf("\033[0;34m"); // Set the text to the color blue
        printf("PLAYER 2... press ENTER to dice");
        getchar();
        j = dice();
        printf("DICE: %d\n",j);
        printf("\033[0m"); // reset color
        if (i == j) {
            printf("Same dice value... Please try again.\n");
        }
        else if (i>j) {
            starter = 1;
            flag = 0;
        }
        else {
            starter = 2;
            flag = 0;
        }
    }
    printf("\n\n *** PLAYER %d will start game... ***\n",starter);
    return starter;
}

/* game */
int play_game(char game_area[ROW+1][COL+1], int starter) {
    int i, j, p2 = 2, dice1, dice2, p1 = starter, finish = 0, p1x, p1y, p2x, p2y, direction1, direction2, w1 = 0, w2 = 0, winner;
    char mc;
    if (starter == 2) 
        p2 = 1;
        
    while (!finish) 
    {
        printMap(game_area, 15, 30);
        /* find player locations */
        for (i = 0; i<15; i++) {
            for (j = 0; j<30; j++) {
                mc = game_area[i][j];
                if (mc == p1+'1'-1) {
                    p1x = j;
                    p1y = i;
                    game_area[i][j] = ' ';
                }
                else if (mc == p2+'1'-1) {
                    p2x = j;
                    p2y = i;
                    game_area[i][j] = ' ';
                }
            }  
        }

        
        printf("\033[0;3%dm",p1+2); // Set the text to the color p1's color
        printf("PLAYER %d... press ENTER to dice",p1);
        getchar();
        dice1 = dice();
        printf("DICE: %d\n", dice1);
        if (p1 == 1) {
            w1 = p1_plays(game_area, p1x, p1y, dice1);
        }
        else {
            w1 = p2_plays(game_area, p1x, p1y, dice1);
        }
        if (w1 == 1) {
            put_player(game_area, p2x, p2y, p2);
            winner = p1;
            break;
        }

        printf("\033[0;3%dm",p2+2); // Set the text to the color p2's color
        printf("PLAYER %d... press ENTER to dice",p2);
        getchar();
        dice2 = dice();
        printf("DICE: %d\n",dice2);
        printf("\033[0m"); // reset color
        if (p1 == 1) {
            w2 = p2_plays(game_area, p2x, p2y, dice2);
        }
        else {
            w2 = p1_plays(game_area, p2x, p2y, dice2);
        }
        if (w2 == 1) {
            winner = p2;
            break;
        }
        
        
    }
    return winner;
}

/* function for Player 1's move */
int p1_plays(char game_area[ROW+1][COL+1], int p1x, int p1y, int dicem) {
    int i, j, tempdice, win = 0;
    if (p1x < 28 && p1y == 1) { // Top side
        if((p1x+dicem)<=28)
            p1x += dicem;
        else {
            tempdice = p1x+dicem-28;
            p1x = 28;
            p1y += tempdice;
        }
    }
    else if (p1x > 1 && p1y == 13) { // Bottom side
        if((p1x - dicem)>=1)
            p1x -= dicem;
        else {
            tempdice = abs(p1x-dicem)+1;
            p1x = 1;
            p1y -= tempdice;
        }
    }
    else if (p1x == 1) { // Left side
        if((p1y - dicem)>2)
            p1y -= dicem;
        else if ((p1y - dicem) == 2)  {
            p1y -= dicem;
            win = 1;
        }
        else {
            tempdice = abs(p1y-dicem)+1;
            p1y = 1;
            p1x += tempdice;
            win = 1;
        }
    }
    else if (p1x == 28) { // Right side
        if((p1y+dicem)<=13)
            p1y += dicem;
        else {
            tempdice = p1y+dicem-13;
            p1y = 13;
            p1x -= tempdice;
        }
    }
    put_player(game_area, p1x, p1y, 1);
    return win;
}

/* function for Player 2's move */
int p2_plays(char game_area[ROW+1][COL+1], int p2x, int p2y, int dicem) {
    int i, j, tempdice, win = 0;
    if (p2x < 26 && p2y == 3) { // Top side
        if((p2x+dicem)<=26)
            p2x += dicem;
        else {
            tempdice = p2x+dicem-26;
            p2x = 26;
            p2y += tempdice;
        }
    }
    else if (p2x > 3 && p2y == 11) { // Bottom side
        if((p2x - dicem)>=3)
            p2x -= dicem;
        else {
            tempdice = abs(p2x-dicem-2)+1;
            p2x = 3;
            p2y -= tempdice;
        }
    }
    else if (p2x == 3) { // Left side
        if((p2y - dicem)>4)
            p2y -= dicem;
        else if ((p2y - dicem) == 4)  {
            p2y -= dicem;
            win = 1;
        }
        else {
            tempdice = abs(p2y-dicem-2)+1;
            p2y = 3;
            p2x += tempdice;
            win = 1;
        }
    }
    else if (p2x == 26) { // Right side
        if((p2y+dicem)<=11)
            p2y += dicem;
        else {
            tempdice = p2y+dicem-11;
            p2y = 11;
            p2x -= tempdice;
        }
    }
    put_player(game_area, p2x, p2y, 2);
    return win;
}

/* this function checking penalty and put player to game_area */ 
void put_player(char game_area[ROW+1][COL+1], int px, int py, int player_num) {
    int i, j, put = 0;

    for (i = 0; i<15 && !(put); i++) {
        for (j = 0; j<30 && !(put); j++) {
            if(i == py && j == px) {
                if (game_area[i][j] == 'X') {
                    if (j > 25) { // Penalty conditions
                        print_penalty(player_num);
                        i -= 2;
                    }
                    else if(i<5){
                        print_penalty(player_num);
                        j -= 2;
                    }
                    else {
                        print_penalty(player_num);
                        j += 2;
                    }

                }
                game_area[i][j] = player_num+'1'-1; //Player putting
                put = 1;  // putting complated
            }
        }
    }
}

void print_penalty(int player_num) {
    printf("\033[0;3%dm",player_num+2); // Set the text to the color player's color
    printf("Penalty for player %d...\n",player_num);
    printf("\033[0m"); // reset color
}
