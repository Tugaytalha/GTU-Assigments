#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define STACK_BLOCK_SIZE 10

typedef struct { int * array; int currentsize; int maxsize} stack;

int push(stack * s, int d); /* the stack array will grow STACK_BLOCK_SIZE entries at a time */

int pop(stack * s); /* the stack array will shrink STACK_BLOCK_SIZE entries at a time */

stack * init_return(); /* initializes an empty stack */

int init(stack * s); /* returns 1 if initialization is successful */

int destinationCount (int array[], int numberOfElements);

stack * extend_stack(stack *s);

int findmin(stack *s); // find smallest disk

int main (void)
{ 
    stack *stacks;
    int numberOfDisks, i, left_or_right, moveCount = 0, min_or_no = 0, temp;
    int from, to, disk;
    int top_of_rod[4];

    stacks = init_return();
    if (!init(stacks)) {
        printf("Error in init! Terminating...\n");
        return -99;
    }      
 
    printf ("\nNumber of disks = ");
    scanf ("%d", &numberOfDisks);

    // all disks start from rod '1'
    for (i = numberOfDisks; i >= 1; i--) {
        push(stacks+1, i);
    }
    
    
    
    

    // find direction to move smallest disk
    if (!(numberOfDisks & 1))
        left_or_right = 1;
    else
        left_or_right = -1;    
    
    printf("\nTower of Hanoi Puzzle Solution\n"
           "------------------------------\n");
        
    do {
        
        moveCount++;
    
        if (!min_or_no) {
            // move smallest disk
            
            from = findmin(stacks);
            
            to = from + left_or_right;
            if ( to > 3 )
                to -= 3;
            if ( to < 1 )
                to += 3;
            
            disk = 1;

        }
        else {
            // move other disk (1 valid move only)

            // store disks at the top of the rods for a more understandable operation (if there is no, number of disks+1)
            for ( i = 3; i >= 1; --i ) {
                if(stacks[i].currentsize-1 >= 0)
                    top_of_rod[i] =  stacks[i].array[stacks[i].currentsize-1];      
                else 
                    top_of_rod[i] = numberOfDisks + 1;      
            }

            // we need rods without smallest disk
            from = findmin(stacks) - 1;
            to = findmin(stacks) + 1;
            if ( to > 3 )
                to -= 3;
            else if ( to < 1 )
                to += 3;      
            else if ( from > 3 )
                from -= 3;
            else if ( from < 1 )
                from += 3;  

            // Only smaller can move otherwise change those
            if (top_of_rod[to]< top_of_rod[from]) {
                temp = from;
                from = to;
                to = temp;            
            }
            disk = top_of_rod[from]; 
        }        

        // pop & push disk
        push(stacks + to, pop(stacks + from));

        // additional info: Number of moves required is 2^(number of disks) - 1
        printf ("%3d: Move the disk %2d from '%d' to '%d'\n", moveCount, disk, from, to);
                
        // reverse min_or_no
        min_or_no = ! min_or_no;
        
    }
    while ( stacks[3].currentsize != numberOfDisks );
    free(stacks);
    return 0;
}

int init(stack * s) {
    int i, t = 1;
    for(i = 1; i < 4; ++i) {
        if (s[i].maxsize != STACK_BLOCK_SIZE || s[i].currentsize != 0 )
            t = 0;
    }
    return t;
}

int push(stack * s, int d)  {
    if (s->currentsize >= s->maxsize)
        extend_stack(s);
    s->array[s->currentsize] = d;
    s->currentsize++;

    return 1;
}

int pop(stack * s){
    int rtn;
    s->currentsize--;
    rtn = s->array[s->currentsize];
    s->array[s->currentsize] = 0;

    return rtn;
}

stack * init_return() {
    stack *new;
    int i, j;
    // allocating stack
    new = (stack *) malloc (4 * sizeof(stack));  // element 0 will not be used
    for (i = 1; i <= 3; i++) {
        //allocating array in stack
        new[i].array = (int *) malloc(STACK_BLOCK_SIZE * sizeof(int));
        for (j = 0; j < STACK_BLOCK_SIZE; j++) 
            new[i].array[j] = 0;
        new[i].maxsize = STACK_BLOCK_SIZE;
        new[i].currentsize = 0;
    }
    return new;
}


stack * extend_stack(stack *s) {
    s->array = (int *) realloc(s->array, (s->maxsize + STACK_BLOCK_SIZE) * sizeof(int));
    s->maxsize += STACK_BLOCK_SIZE;

    return s;
}

int findmin(stack *s) {
    int i;
    for (i = 1; i <= 3; i++) {
        if (s[i].currentsize-1 >= 0 && s[i].array[s[i].currentsize-1] == 1)
            return i;
    }    
}