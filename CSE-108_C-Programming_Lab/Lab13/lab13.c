#include <stdio.h>
#include <stdlib.h>
#include <string.h>


typedef struct node
{
  char bookname[30];
  char author[30];
  int year;
  struct node *next;
}node;

node *read_file();

node *insert_node(node *root, char *bookname, char *author, int year);

void print_nodes(node *root);

node *delete_node(node *root, char *bookname);

int main() {
  int cas;
  char bookname[30];
  node *new, *head;
  head = read_file();
  print_nodes(head);
  printf("Wanted delete? " );
  scanf("%s", bookname);
  head = delete_node(head, bookname);
  print_nodes(head);

  return 0;
}

node *read_file() {
  node * head;
  char name[30], author[30];
  FILE *f;
  int year;
  head = NULL;
  printf("Reaading the source.txt file...\n");
  f = fopen("source.txt", "r");
  while(fscanf(f, "%s %s %d", name, author, &year) == 3) {
    head = insert_node(head, name, author, year);
  }
  fclose(f);
  return head;
}

node *insert_node(node *root, char *bookname, char *author, int year) {  // inserting node to linked list
  node *cp, *new, *temp;
  if (root == NULL) {  // if there is no node create root
    root = (node *) malloc(sizeof(node));
    strcpy(root->bookname, bookname);
    strcpy(root->author, author);
    root->year = year;
    root->next = NULL;
  }
  else {  // if linked list is exist
    cp = root;
    while(cp != NULL && year > cp->year  ) {  // contunie until find true place
      temp = cp;
      cp = cp->next;
    }

    // insert part
    if (cp == root) {
      new = (node *) malloc(sizeof(node));
      new->next = root;
      strcpy(new->bookname, bookname);
      strcpy(new->author, author);
      new->year = year;
      root = new;
    }
    else if (cp == NULL) {
      new = (node *) malloc(sizeof(node));
      strcpy(new->bookname, bookname);
      strcpy(new->author, author);
      new->year = year;
      new->next = NULL;
      temp->next = new;
    }
    else {
      new = (node *) malloc(sizeof(node));
      strcpy(new->bookname, bookname);
      strcpy(new->author, author);
      new->year = year;
      new->next = cp;
      temp->next = new;
    }
  }
  return root;
}

//printing I thing clear
void print_nodes(node *root) {
  printf("Printing the linked list\n" );
  while (root != NULL) {  // until end of the list
    printf("%15s %15s %8d\n", root->bookname, root->author, root->year);
    root = root->next;
  }
}


node *delete_node(node *root, char *bookname) {
  node *temp, *cp;
  int found = 0;
  cp = root;

  while (cp != NULL && !found) {  // until found or end of the list
    if (!strcmp(cp->bookname, bookname)) {  // if find
      if (cp == root) {
        root = root->next;
        free(cp);
      }
      else if (cp->next == NULL) {
        temp->next = NULL;
        free(cp);
      }
      else {
        temp->next = cp->next;
        free(cp);
      }
      found = 1;
    }
    temp = cp;
    cp = cp->next;
  }
  return root;
}
