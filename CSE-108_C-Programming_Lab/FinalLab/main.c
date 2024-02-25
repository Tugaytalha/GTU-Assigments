#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define NUMOFITEMS 2
#define FILENAME "Lab11_data.bin"
//Variable
char * prt_info;

//Struct
typedef struct item
{
	int price;
	char name[20];
	int ID;
}item_t;

//Struct
typedef struct
{
	char mail[20];
	int phone;
}contactInfo_t;

//Struct
typedef struct
{
	int price;
	char name[20];
	int ID;
	char mail[20];
	int phone;
}itemWithCont_t;



item_t *getItems(item_titems,int size);

node *insert_node(node *root, char *bookname, char *author, int year);

void printItem(item_t *root);

node *delete_node(node *root, char *bookname);

int writeItems(char filename[], item_t *items, int size);

int main()
{
	item_t * items;
	getItems(items,0);


	return 0;


}


item_t *getItems(item_t *items, int size) {

  for(i = size; i < size + NUMOFITEMS; i++) {
		printf("ID => ", i+1);
		printf("Name => " );
		scanf("%s", items[i].name);
		printf("price => " );
		scanf("%d", &items[i].price);
	}
	for(i = size; i < size + NUMOFITEMS; i++) {
		printf("Printing...\n" );
		printItem(items[i]);
	}
	if(writeItems(FILENAME, items, size+2) == size+2) printf("Succesfully wrote data to file\n");
  return items;
}


int writeItems(char filename[], item_t *items, int size) {
	FILE *f;
	f = fopen(filename, "rb");

	for (i = 0)
	return size;
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
void printItem(item_t *root) {
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
