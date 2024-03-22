#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

// Function to create a new student grade
void addStudentGrade(char *name, char *grade) {
  // Open the grades file in append mode
  FILE *fp = fopen("grades.txt", "a");
  if (fp == NULL) {
    perror("Error opening file");
    exit(1);
  }

  // Write the student's name and grade to the file
  fprintf(fp, "%s, %s\n", name, grade);

  // Close the file
  fclose(fp);
}

// Function to search for a student's grade
void searchStudent(char *name) {
  // Open the grades file in read mode
  FILE *fp = fopen("grades.txt", "r");
  if (fp == NULL) {
    perror("Error opening file");
    exit(1);
  }

  // Read the file line by line
  char line[256];
  while (fgets(line, sizeof(line), fp)) {
    // Remove the newline character from the line
    line[strlen(line) - 1] = '\0';

    // Split the line into the student's name and grade
    char *name = strtok(line, ", ");
    char *grade = strtok(NULL, ", ");

    // If the student's name matches the search term, print the student's name and grade
    if (strcmp(name, name) == 0) {
      printf("%s, %s\n", name, grade);
    }
  }

  // Close the file
  fclose(fp);
}

// Function to sort the student grades
void sortStudentGrades(char *filename) {
  // Open the grades file in read mode
  FILE *fp = fopen("grades.txt", "r");
  if (fp == NULL) {
    perror("Error opening file");
    exit(1);
  }

  // Read the file line by line and store the student grades in an array
  char lines[100][256];
  int i = 0;
  while (fgets(lines[i], sizeof(lines[i]), fp)) {
    // Remove the newline character from the line
    lines[i][strlen(lines[i]) - 1] = '\0';

    i++;
  }

  // Close the file
  fclose(fp);

  // Sort the student grades using the quicksort algorithm
  qsort(lines, i, sizeof(lines[0]), strcmp);

  // Open the grades file in write mode
  fp = fopen("grades.txt", "w");
  if (fp == NULL) {
    perror("Error opening file");
    exit(1);
  }

  // Write the sorted student grades to the file
  for (int j = 0; j < i; j++) {
    fprintf(fp, "%s\n", lines[j]);
  }

  // Close the file
  fclose(fp);
}

// Function to display the student grades
void displayStudentGrades(char *filename) {
  // Open the grades file in read mode
  FILE *fp = fopen("grades.txt", "r");
  if (fp == NULL) {
    perror("Error opening file");
    exit(1);
  }

  // Read the file line by line and print the student grades
  char line[256];
  while (fgets(line, sizeof(line), fp)) {
    // Remove the newline character from the line
    line[strlen(line) - 1] = '\0';

    // Print the student's name and grade
    printf("%s\n", line);
  }

  // Close the file
  fclose(fp);
}

// Function to display the usage of the program
void usage() {
  printf("Usage: gtuStudentGrades [command] [arguments]\n");
  printf("\nCommands:\n");
  printf("  addStudentGrade <name> <grade>: Add a new student grade\n");
  printf("  searchStudent <name>: Search for a student's grade\n");
  printf("  sortStudentGrades: Sort the student grades\n");
  printf("  displayStudentGrades: Display the student grades\n");
  printf("  usage: Display the usage of the program\n");
}

// Main function
int main(int argc, char *argv[]) {
  // Check if the user entered a command
  if (argc < 2) {
    usage();
    return 1;
  }

  // Get the command from the user
  char *command = argv[1];

  // Execute the command
  if (strcmp(command, "addStudentGrade") == 0) {
    // Check if the user entered the student's name and grade
    if (argc < 4) {
      usage();
      return 1;
    }

    // Get the student's name and grade from the user
    char *name = argv[2];
    char *grade = argv[3];

    pid_t pid = fork();
    if(pid == 0)
    {
        // Child process
        addStudentGrade(name, grade);
        exit(0);
    }
    else if(pid > 0)
    {
        // Parent process
        wait(NULL);
		printf("Student grade added successfully.\n");
    }
    else
    {
        // Error occurred
        perror("Error creating child process");
        exit(1);
    }
  } else if (strcmp(command, "searchStudent") == 0) {
    // Check if the user entered the student's name
    if (argc < 3) {
      usage();
      return 1;
    }

    // Get the student's name from the user
    char *name = argv[2];

    pid_t pid = fork();
    if(pid == 0)
    {
        // Child process
        searchStudent(name);
        exit(0);
    }
    else if(pid > 0)
    {
        // Parent process
        wait(NULL);
    }
    else
    {
        // Error occurred
        perror("Error creating child process");
        exit(1);
    }
  } else if (strcmp(command, "sortStudentGrades") == 0) {
    sortStudentGrades("grades.txt");
    printf("Student grades sorted successfully.\n");
  } else if (strcmp(command, "displayStudentGrades") == 0) {
    displayStudentGrades("grades.txt");
  } else if (strcmp(command, "usage") == 0) {
    usage();
  } else {
    printf("Invalid command.\n");
    usage();
  }

  return 0;
}