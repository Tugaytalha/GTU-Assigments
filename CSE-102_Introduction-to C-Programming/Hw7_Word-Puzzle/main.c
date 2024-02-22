#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>


int randnum(int min, int max);

int is_overlap(char puzzle[][16], int sx, int sy, int lenght_of_word, int direction);

void add_to_puzzle(char puzzle[][16], char word[6],int sx, int sy, int direction);

void print_puzzle(char puzzle[][16]);

void fill_puzzle(char puzzle[][16]);

int is_exist_in_puzzle(char puzzle[][16], int sx, int sy, char word[6]);

int check_direction(char puzzle[][16], int sx, int sy, char word[6], int direction);

int play_game(char puzzle[][16], char words[][6]);

int wrong(int left);

void delete_word(char puzzle[][16], int low, int sx, int sy, int direction, char words[][6], int ir);


int main() {
    int i, j, k, a = 1, c = 1, b = 0, d, index, st[2], final_point;
    int low;  // lenght of word
    char puzzle[15][16], words[7][6], indexes[8], ch;
    FILE *word_file;

    word_file = fopen("wordlist.txt","r");
    if (word_file==NULL) printf("File couldn't open\n\n");
    
    for (i = 0; i < 15; i++) for (j = 0; j<16; j++) puzzle[i][j] = 0;
    
    srand(time(NULL));
    
    for (i=0; i<7; i++) {
        fseek(word_file, 0, SEEK_SET);
        index = randnum(1,50);
        ch = index + 48;
        if(strchr(indexes, ch)) {
            i--;
            continue;
        }
        indexes[i] = ch;
        
        for (j = 0; j < index; j++) fscanf(word_file, "%s", words[i]);

        low = strlen(words[i]);
        a = 1;
        
        while(a) {
            //1. L to R   2. R to L   3. U to D   4. D to U   5. LD to RU   6. LU to RD   7. RD to LU   8. RU to LD 
            d = randnum(1,8); // decide to word's direction 
            st[0] = randnum(1,15); // decide to word's start index
            st[1] = randnum(1,15); // decide to word's start index
            c = 1;
            //Validiate of word's location limit control
            if ((d == 1 || (b = 1 && (d == 5 || d == 6 ))) && (st[0] + low <= 16 || (c = 0 && 0))) {
                if (b == 0) a = 0;
            }
            if ((b = 0) || ((d == 2 || (b = 1 && (d == 7 || d == 8 ))) &&  (st[0] - low >= 0 || (c = 0 && 0)))) {
                if (b == 0) a = 0;
            }
            if ((d == 3 || d == 6 || d == 8) && st[1] + low <= 16) {
                if (c == 1) a = 0;
            }
            if ((d == 4 || d == 5 || d == 7) && st[1] - low >= 0) {
                if (c == 1) a = 0;
            }
            b = 0;

            if (a || is_overlap(puzzle, st[0], st[1], low, d))  // overlap with other words?
                a = 1;
        }
        printf("Word: %5s    Start Coordinate: [%2d, %2d]\n", words[i], st[0], st[1]);

        add_to_puzzle(puzzle, words[i], st[0], st[1], d);  // add word to puzzle
    }
    fill_puzzle(puzzle);
    //print_puzzle(puzzle);
    final_point = play_game(puzzle,words);
    if (final_point == 14)
        printf("\nCongratulations! You found all of words and you won the game!!!\n\n");
    else if (final_point<0) 
        printf("\nYou got %d points and quitted the game\n\n", -final_point);
    else 
        printf("\nYou have no left and got %d points, game over :/...\n\n", final_point);

    return 0;
}



int randnum(int min, int max) {
    return ((rand() % (int) (((max) + 1) - (min))) + (min));
}

// add word to puzzle
int is_overlap(char puzzle[][16], int sx, int sy, int low, int direction) {
    int b = 1, t = 0, i;
            
    switch (direction)
    {
    case 1:
        for (i = sx-1; i<sx-1+low; i++) {    
            if (puzzle[sy-1][i] != 0) {   // search for other words 
                t = 1;
            }
        }
        break;

    case 2:
        for (i = sx-1; i>sx-1-low; i--) {    
            if (puzzle[sy-1][i] != 0) {  // search for other words
                t = 1;
            }
        }
        break;

    case 3:
        for (i = sy-1; i<sy-1+low; i++) {    
            if (puzzle[i][sx-1] != 0) {  // search for other words
                t = 1;
            }
        }
        break;

    case 4:
        for (i = sy-1; i>sy-1-low; i--) {    
            if (puzzle[i][sx-1] != 0) {  // search for other words
                t = 1;
            }
        }
        break;

    case 5:
        for(i = 0; i<low; i++) {
            if (puzzle[sy-1-i][sx-1+i] != 0) {  // search for other words
                t = 1;
            }
        }
        break;

    case 6:
        for(i = 0; i<low; i++) {
            if (puzzle[sy-1+i][sx-1+i] != 0) {  // search for other words
                t = 1;
            }
        }
        break;

    case 7:
        for(i = 0; i<low; i++) {
            if (puzzle[sy-1-i][sx-1-i] != 0) {  // search for other words
                t = 1;
            }
        }
        break;

    case 8:
        for(i = 0; i<low; i++) {
            if (puzzle[sy-1+i][sx-1-i] != 0) {  // search for other words
                t = 1;
            }
        }
        break;
    }
    return t;
}


void print_puzzle(char puzzle[][16]) {
    int i, j;
    for (i = 0; i<15; i++) {
        for (j = 0; j<15; j++) {
            if (puzzle[i][j] == 'X') {
                printf("\033[0;31m"); //Set the text to the color red
                printf("%c ",puzzle[i][j]);
                printf("\033[0m"); //Resets the text to default color
            }               
            else if (puzzle[i][j])
                printf("%c ",puzzle[i][j]);
            else 
                printf("  ");
        }   
        printf("\n");
    }
}

void add_to_puzzle(char puzzle[][16], char word[6],int sx, int sy, int direction) {
    int b = 1, i;
    size_t low = strlen(word);
    for (i = 0; i<low; i++) {  // contunie until lenght of word
        switch (direction)
        {  // place the letter
        case 1:
            puzzle[sy-1][sx-1+i] = word[i];
            break;

        case 2:
            puzzle[sy-1][sx-1-i] = word[i];
            break;

        case 3:
            puzzle[sy-1+i][sx-1] = word[i];
            break;

        case 4:
            puzzle[sy-1-i][sx-1] = word[i];
            break;

        case 5:
            puzzle[sy-1-i][sx-1+i] = word[i];
            break;

        case 6:
            puzzle[sy-1+i][sx-1+i] = word[i];
            break;

        case 7:
            puzzle[sy-1-i][sx-1-i] = word[i];
            break;

        case 8:
            puzzle[sy-1+i][sx-1-i] = word[i];
            break;
        }
    }
}        
    
void fill_puzzle(char puzzle[][16]) {
    int i, j;
    for (i = 0; i<15; i++) {
        for (j = 0; j<15; j++) {
            if (!puzzle[i][j]) 
                puzzle[i][j] = randnum('a','z');  // take random char to puzzle's empaty part
        }   
    }
}

int is_exist_in_puzzle(char puzzle[][16], int sx, int sy, char word[6]) {
    int i, j, direction = 0, a = 0;
    char c;

    for (i = 0; i<15; i++) {
        for (j = 0; j<15; j++) {
            if(puzzle[i][j] == word[0]) {
                if(puzzle[i-1][j-1] == word[1]) {
                    a = check_direction(puzzle, sx, sy, word, 7);
                    if (a)  // if word exist 
                        direction = a; 
                }
                if(puzzle[i-1][j] == word[1]) {
                    a = check_direction(puzzle, sx, sy, word, 4);
                    if (a)  // if word exist 
                        direction = a; 
                }
                if(puzzle[i-1][j+1] == word[1]) {
                    a = check_direction(puzzle, sx, sy, word, 5);
                    if (a)  // if word exist 
                        direction = a; 
                }
                if(puzzle[i][j-1] == word[1]) {
                    a = check_direction(puzzle, sx, sy, word, 2);
                    if (a)  // if word exist 
                        direction = a; 
                }
                if(puzzle[i][j+1] == word[1]) {
                    a = check_direction(puzzle, sx, sy, word, 1);
                    if (a)  // if word exist 
                        direction = a; 
                }
                if(puzzle[i+1][j-1] == word[1]) {
                    a = check_direction(puzzle, sx, sy, word, 8);
                    if (a)  // if word exist 
                        direction = a; 
                }
                if(puzzle[i+1][j] == word[1]) {
                    a = check_direction(puzzle, sx, sy, word, 3);
                    if (a)  // if word exist 
                        direction = a; 
                }
                if(puzzle[i+1][j+1] == word[1]) {
                    a = check_direction(puzzle, sx, sy, word, 6);
                    if (a)  // if word exist 
                        direction = a; 
                }
            }
        }   
    }
    return direction;
}

int check_direction(char puzzle[][16], int sx, int sy, char word[6], int direction) {
    int i, j, low;
    low = strlen(word);
    for (i = 2; i<low; i++) {  // contunie until lenght of word
        switch (direction)
        {  // check the word
        case 1:
            if(puzzle[sy-1][sx-1+i] != word[i])
                direction = 0;
            break;

        case 2:
            if(puzzle[sy-1][sx-1-i] != word[i])
                direction = 0;
            break;

        case 3:
            if(puzzle[sy-1+i][sx-1] != word[i])
                direction = 0;
            break;

        case 4:
            if(puzzle[sy-1-i][sx-1] != word[i])
                direction = 0;
            break;

        case 5:
            if(puzzle[sy-1-i][sx-1+i] != word[i])
                direction = 0;
            break;

        case 6:
            if(puzzle[sy-1+i][sx-1+i] != word[i])
                direction = 0;
            break;

        case 7:
            if(puzzle[sy-1-i][sx-1-i] != word[i])
                direction = 0;
            break;

        case 8:
            if(puzzle[sy-1+i][sx-1-i] != word[i])
                direction = 0;
            break;
        }
    }
    return direction;
}

int play_game(char puzzle[][16], char words[][6]) {
    int i, startx = 0, t = 0, starty = 0, found = 0, point = 0, left = 3, direction, ir, low;
    char commands[3][6];

    while (left && !found && point != 14) {
        print_puzzle(puzzle);
        printf("\nEnter coordinates and word:");
        scanf("%s",commands[0]);
        if (!strcmp(commands[0],":q")) {  //quit command
            printf("Quitting...\n");
            return -point;
            }
        scanf("%s %s", commands[1],commands[2]);
        
        printf("%s %s %s\n",commands[0],commands[1],commands[2]);
        
        t = 0;
        for (i = 0; i < 7; i++) {
            if(!strcmp(commands[2],words[i])) {
                t = 1;
                ir = i;
                break;
            }
        }

        if (t)
        {   
            startx = 0;
            starty = 0; 
            for (i = 0; commands[0][i] != '\0'; i++)
                startx = startx * 10 + commands[0][i] - '0';
            for (i = 0; commands[1][i] != '\0'; i++)
                starty = starty * 10 + commands[1][i] - '0';
            
            direction = is_exist_in_puzzle(puzzle, startx, starty, commands[2]);
            if (direction){
                point += 2;
                printf("Founded! YOu got 2 points. Your total points: %d\n\n",point);
                low = strlen(commands[2]);
                delete_word(puzzle, low, startx, starty, direction, words, ir);
            }
            else 
                left = wrong(left);
        }
        else 
            left = wrong(left);
        
    }
    return point;
}

int wrong(int left) {
    left--; // dexrement lefts
    if (left) {  // if user still have lefts print lefts
        printf("Wrong choice! You have only %d lefts.\n\n",left);
    }
    return left; 
}

void delete_word(char puzzle[][16], int low, int sx, int sy, int direction, char words[][6], int ir) {
    int i;

    for (i = 0; i < 6; i++)
        words[ir][i] = 0; // clean the word in words
    
    for (i = 0; i<low; i++) {  // contunie until lenght of word
        switch (direction)
        {  // place the Xs
        case 1:
            puzzle[sy-1][sx-1+i] = 'X';
            break;

        case 2:
            puzzle[sy-1][sx-1-i] = 'X';
            break;

        case 3:
            puzzle[sy-1+i][sx-1] = 'X';
            break;

        case 4:
            puzzle[sy-1-i][sx-1] = 'X';
            break;

        case 5:
            puzzle[sy-1-i][sx-1+i] = 'X';
            break;

        case 6:
            puzzle[sy-1+i][sx-1+i] = 'X';
            break;

        case 7:
            puzzle[sy-1-i][sx-1-i] = 'X';
            break;

        case 8:
            puzzle[sy-1+i][sx-1-i] = 'X';
            break;
        }
    }
}

