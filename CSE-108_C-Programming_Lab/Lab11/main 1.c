#include <stdio.h>
#include <stdlib.h>
#include <string.h>


typedef struct {
  int ID;
  char title[50];
  char author[50];
  char subject[100];
  int year;

} Book;

Book *Books;
int empty = 2, maxsize = 2;

void incrase_books_size();

void add_new_book();

int get_by_title();

int get_by_author();

int get_by_subject();

void get_sort_year();

void print_all();

int main() {
  int i, j, t, cas;
  char c;

  Books = (Book *) malloc(2 * sizeof(Book));
  t = 1;
  while (t) {
		printf(
			   "\n\nMENU\n"
			   "1. Add New Book\n"
			   "2. List Books\n"
  			 "3. EXIT.\n"
         "------------------------------\n"
			   );
		printf("\nYOUR CHOICE: ");
    if(scanf("%d",&cas) != 1 || (cas < 1 || cas > 3)) {
			printf("ERROR, please enter a valid entry."); // if input isn't valid(char etc.) will print this
			continue;
		}
		while((c = getchar()) != '\n' && c != EOF); // waiting to entry like fflush, beacuse if entry a char program will try to read that all time
		switch(cas) { // chose apllier
      case 1:
        add_new_book();

        break;
      case 2:
        while (t) {
          printf(
               "\n\nSUBMENU\n"
               "1. Get by title\n"
               "2. Get by Author\n"
               "3. Get by subject  \n"
               "4. Sorted list by year (DESC) \n"
               "5. List all books.\n"
               "6. EXIT nSUBMENU\n"
               "------------------------------\n"
               );
          printf("\nYOUR CHOICE: ");
          if(scanf("%d",&cas) != 1) {
            printf("ERROR, please enter a valid entry."); // if input isn't valid(char etc.) will print this
            continue;
          }
          while((c = getchar()) != '\n' && c != EOF); // waiting to entry like fflush, beacuse if entry a char program will try to read that all time
          switch(cas) { // chose apllier
            case 1:
              print_book_by_id(get_by_title());  // get by sleeted title

              break;
            case 2:
              print_book_by_id(get_by_author()); // get by sleeted author

              break;
            case 3:
              print_book_by_id(get_by_subject()); // get by sleeted Subject
              break;
            case 4:
                get_sort_year();
              break;
            case 5:
               print_all();

              break;
            case 6:
              break;

          }

        break;
      case 3:
        return 0;

      }
    }
  }

  return 0;
}

void add_new_book() {
    int yearb;
    char  titleb[50], authorb[50], subjectb[100];

    if (empty == 0) incrase_books_size();
    printf("Title => " );
    scanf("%[^\n]%*c", titleb);
    printf("Author => " );
    scanf("%[^\n]%*c", authorb);
    printf("Subject => " );
    scanf("%[^\n]%*c", subjectb);
    printf("Year => " );
    scanf("%d", &yearb);

    Books[maxsize-empty].ID = maxsize - empty + 1;
    Books[maxsize-empty].year = yearb;
    strcpy(Books[maxsize-empty].title, titleb);
    strcpy(Books[maxsize-empty].subject, subjectb);
    strcpy(Books[maxsize-empty].author, authorb);
    empty--;
}

void incrase_books_size() {
  Book *abc;
  int i;

  // add 2 to global variables
  maxsize += 2;
  empty += 2;

  //allocat new memory
  abc = (Book *) calloc(maxsize, sizeof(Book));

  //coppying new memory
  for (i = 0; i < maxsize; i++) {
    abc[i].ID = Books[i].ID;
    strcpy(abc[i].title, Books[i].title);
    strcpy(abc[i].author, Books[i].author);
    strcpy(abc[i].subject, Books[i].subject);
  }

  // freeing and new assigment
  free(Books);
  Books = abc;
}


int get_by_title() {
  char titleb[50];
  int i;

  printf("Title => " );
  scanf("%[^\n]%*c", titleb);

  for (i = 0; i < maxsize; i++) {
    if(!strcmp(titleb,Books[i].title)) {
      printf("Book found!\n" );
      return Books[i].ID;
    }
  }
}

int get_by_author() {
  char authorb[50];
  int i;

  printf("Author => " );
  scanf("%[^\n]%*c", authorb);

  for (i = 0; i < maxsize; i++) {
    if(!strcmp(authorb, Books[i].author)) {
      printf("Book found!\n" );
      return Books[i].ID;
    }
  }
}

int get_by_subject() {
  char subjectb[100];
  int i;

  printf("Subject => " );
  scanf("%[^\n]%*c", subjectb);

  for (i = 0; i < maxsize; i++) {
    if(!strcmp(subjectb, Books[i].subject)) {
      printf("Book found!\n" );
      return Books[i].ID;
    }
  }
}

void print_book_by_id(int im) {
  int i;
  for (i = 0; i < maxsize; i++){
    if(Books[i].ID == im) {
      printf("Title => %s\n", Books[i].title);
      printf("Author => %s\n", Books[i].author);
      printf("Subject => %s\n", Books[i].subject);
      printf("Title => %s\n", Books[i].title);
      printf("Year => %d\n", Books[i].year);
      return ;
    }
    }
}

void get_sort_year() {
  int i, j;
  Book temp[1];

  for (i = 0; i<maxsize; i++) {
    for (j = i+1; j < maxsize; j++) {
      if (Books[i].year > Books[i].year) {
        temp[0] = Books[i];
        Books[i]  = Books[j];
        Books[j] = temp[0];
      }
    }
  }
  print_all();
}

void print_all() {
  int i;
  printf("list of books\n");
  printf("**************************\n");
  for (i = 0; i < maxsize-empty; i++) {
    printf("%d. Book;\n",i+1);
    printf("Title => %s\n", Books[i].title);
    printf("Author => %s\n", Books[i].author);
    printf("Subject => %s\n", Books[i].subject);
    printf("Title => %s\n", Books[i].title);
    printf("Year => %d\n", Books[i].year);
    printf("**************************\n");
  }
}
