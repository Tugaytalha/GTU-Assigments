#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h> // For open(), read(), write()
#include <ctype.h>

int strcicmp(char const *a, char const *b)
{
    for (;; a++, b++) {
        int d = tolower((unsigned char)*a) - tolower((unsigned char)*b);
        if (d != 0 || !*a)
            return d;
    }
}


// Function to add a student grade to the file
void addStudentGrade(char *filename, char *name, char *grade) {
    int pid = fork();
    if (pid == -1) {
        perror("Error forking");
        exit(1);
    }
    if (pid == 0)
    {// Open the file in append mode (create if it doesn't exist)
        int fd = open(fileName, O_WRONLY | O_APPEND | O_CREAT, 0644);
        if (fd == -1) {
          perror("Error opening file");
          exit(1);
        }

        // Prepare the data to write
        char data[1024];
        sprintf(data, "%s, %s\n", name, grade);

        // Write the data to the file
        if (write(fd, data, strlen(data)) == -1) {
          perror("Error writing to file");
          exit(1);
        }

        // Close the file
        close(fd);
        _exit(EXIT_SUCCESS);
    }
    else
    {
        waitpid(pid, NULL, 0);
    }
}

// Function to search for a student's grade by name
void searchStudentGrade(char *name) {
    int pid = fork();
    if (pid == -1) {
        perror("Error forking");
        exit(1);
    }
    if (pid == 0)
    {
        // Open the file in read mode
        int fd = open("grades.txt", O_RDONLY);
        if (fd == -1) {
          perror("Error opening file");
          exit(1);
        }

        // Read the file line by line
        char line[1024];
        int bytesRead;
        int flags[5];
        while (((bytesRead = read(fd, line, sizeof(line))) > 0) && flags[0] == 0) {
            // Find the end of the line (replace newline with null terminator)
            //for (int i = 0; i < bytesRead; i++) {
            //  if (line[i] == '\n') {
            //    line[i] = '\0';
            //    flags[0] = 1;
            //    break;
            //  }
            //}
            line[bytesRead] = '\0';
            // Split the line into the student's name and grade
            char *studentName = strtok(line, ", ");
            char *studentGrade = strtok(NULL, "\n");
            do
            {

                // Check if the student's name matches the search name
                if (strcicmp(studentName, name) == 0) {
                  // Print the student's name and grade
                  printf("%s, %s\n", studentName, studentGrade);
                  flags[0] = 1;
                }

                studentName = strtok(NULL, ", ");
                studentGrade = strtok(NULL, "\n");

                if (studentName == NULL)
                {
                    flags[0] = 1;
                }
            } while (flags[0] == 0);
        }

      // Close the file
      close(fd);
      _exit(EXIT_SUCCESS);
    }
    else
    {
        waitpid(pid, NULL, 0);
    }
}



//// Function to sort the student grades in the file
//void sortStudentGrades(char *fileName, int sortType) {
//  // Open the file in read mode
//  FILE *file = fopen(fileName, "r");
//  if (file == NULL) {
//    perror("Error opening file");
//    exit(1);
//  }
//
//  // Read the file line by line and store the student grades in an array
//  char lines[1024][1024];
//  int lineCount = 0;
//  while (fgets(lines[lineCount], sizeof(lines[lineCount]), file)) {
//    lineCount++;
//  }
//
//  // Close the file
//  fclose(file);
//
//  // Sort the student grades in the array
//  for (int i = 0; i < lineCount; i++) {
//    for (int j = i + 1; j < lineCount; j++) {
//      if (sortType == 0) {
//        // Sort by name in ascending order
//        if (strcicmp(lines[i], lines[j]) > 0) {
//          char temp[1024];
//          strcpy(temp, lines[i]);
//          strcpy(lines[i], lines[j]);
//          strcpy(lines[j], temp);
//        }
//      } else if (sortType == 1) {
//        // Sort by name in descending order
//        if (strcicmp(lines[i], lines[j]) < 0) {
//          char temp[1024];
//          strcpy(temp, lines[i]);
//          strcpy(lines[i], lines[j]);
//          strcpy(lines[j], temp);
//        }
//      } else if (sortType == 2) {
//        // Sort by grade in ascending order
//        char *grade1 = strtok(lines[i], ", ");
//        strtok(NULL, "\n");
//        char *grade2 = strtok(lines[j], ", ");
//        strtok(NULL, "\n");
//        if (strcicmp(grade1, grade2) > 0) {
//          char temp[1024];
//          strcpy(temp, lines[i]);
//          strcpy(lines[i], lines[j]);
//          strcpy(lines[j], temp);
//        }
//      } else if (sortType == 3) {
//        // Sort by grade in descending order
//        char *grade1 = strtok(lines[i], ", ");
//        strtok(NULL, "\n");
//        char *grade2 = strtok(lines[j], ", ");
//        strtok(NULL, "\n");
//        if (strcicmp(grade1, grade2) < 0) {
//          char temp[1024];
//          strcpy(temp, lines[i]);
//          strcpy(lines[i], lines[j]);
//          strcpy(lines[j], temp);
//        }
//      }
//    }
//  }

//  // Open the file in write mode
//  file = fopen(fileName, "w");
//  if (file == NULL) {
//    perror("Error opening file");
//    exit(1);
//  }
//
//  // Write the sorted student grades to the file
//  for (int i = 0; i < lineCount; i++) {
//    fprintf(file, "%s", lines[i]);
//  }
//
//  // Close the file
//  fclose(file);
//}

//// Function to display all student grades in the file
//void showAllStudentGrades(char *fileName) {
//  // Open the file in read mode
//  FILE *file = fopen(fileName, "r");
//  if (file == NULL) {
//    perror("Error opening file");
//    exit(1);
//  }
//
//  // Read the file line by line and print the student grades
//  char line[1024];
//  while (fgets(line, sizeof(line), file)) {
//    printf("%s", line);
//  }
//
//  // Close the file
//  fclose(file);
//}

//// Function to display the first 5 student grades in the file
//void listFirst5StudentGrades(char *fileName) {
//  // Open the file in read mode
//  FILE *file = fopen(fileName, "r");
//  if (file == NULL) {
//    perror("Error opening file");
//    exit(1);
//  }
//
//  // Read the file line by line and print the first 5 student grades
//  char line[1024];
//  int count = 0;
//  while (fgets(line, sizeof(line), file) && count < 5) {
//    printf("%s", line);
//    count++;
//  }
//
//  // Close the file
//  fclose(file);
//}


// Function to sort the student grades in the file
void sortStudentGrades(char *fileName, int sortType) {
    int pid = fork();
    if (pid == -1) {
        perror("Error forking");
        exit(1);
    }
    if (pid == 0)
    {
        // Open the file in read mode
        int fd = open(fileName, O_RDONLY);
        if (fd == -1) {
            perror("Error opening file");
            exit(1);
        }

        // Read the file line by line and store the student grades in an array
        char lines[1024][1024];
        int lineCount = 0;
        char buffer[1024];
        int bytesRead;
        while ((bytesRead = read(fd, buffer, sizeof(buffer))) > 0) {
            // Split the buffer into lines and store them in the array
            char *line = strtok(buffer, "\n");
            while (line != NULL) {
                strcpy(lines[lineCount], line);
                lineCount++;
                line = strtok(NULL, "\n");
            }
        }

        // Close the file
        close(fd);

        // Sort the student grades in the array
        for (int i = 0; i < lineCount; i++) {
            for (int j = i + 1; j < lineCount; j++) {
                if (sortType == 0) {
                  // Sort by name in ascending order
                  if (strcicmp(lines[i], lines[j]) > 0) {
                    char temp[1024];
                    strcpy(temp, lines[i]);
                    strcpy(lines[i], lines[j]);
                    strcpy(lines[j], temp);
                  }
                } else if (sortType == 1) {
                  // Sort by name in descending order
                  if (strcicmp(lines[i], lines[j]) < 0) {
                    char temp[1024];
                    strcpy(temp, lines[i]);
                    strcpy(lines[i], lines[j]);
                    strcpy(lines[j], temp);
                  }
                } else if (sortType == 2) {
                  // Sort by grade in ascending order
                  char *grade1 = strtok(lines[i], ", ");
                  strtok(NULL, "\n");
                  char *grade2 = strtok(lines[j], ", ");
                  strtok(NULL, "\n");
                    printf("grade1: %s, grade2: %s\n", grade1, grade2);
                  if (strcicmp(grade1, grade2) > 0) {
                    char temp[1024];
                    strcpy(temp, lines[i]);
                    strcpy(lines[i], lines[j]);
                    strcpy(lines[j], temp);
                  }
                } else if (sortType == 3) {
                  // Sort by grade in descending order
                  strtok(lines[i], ", ");
                  char *grade1 = strtok(NULL, "\n");
                  strtok(lines[j], ", ");
                  char *grade2 = strtok(NULL, "\n");
                    printf("grade1: %s, grade2: %s\n", grade1, grade2);
                  if (strcicmp(grade1, grade2) < 0) {
                    char temp[1024];
                    strcpy(temp, lines[i]);
                    strcpy(lines[i], lines[j]);
                    strcpy(lines[j], temp);
                  }
                }
                else
                {
                    printf("Invalid sort type\n");
                    _exit(1);
                }
            }
        }

        // Print the sorted student grades
        for (int i = 0; i < lineCount; i++) {
            printf("%s\n", lines[i]);
        }
    }
    else
    {
        wait(NULL);
    }
}


// Function to display all student grades in the file
void showAllStudentGrades(char *fileName) {
    // Open the file in read mode
    int fd = open(fileName, O_RDONLY);
    if (fd == -1) {
        perror("Error opening file");
        exit(1);
    }

    // Read the file line by line and print the student grades
    char buffer[1024];
    int bytesRead;
    while ((bytesRead = read(fd, buffer, sizeof(buffer))) > 0) {
        write(STDOUT_FILENO, buffer, bytesRead);
    }

    // Close the file
    close(fd);
}

// Function to display the first 5 student grades in the file
void listFirst5StudentGrades(char *fileName) {
    // Open the file in read mode
    int fd = open(fileName, O_RDONLY);
    if (fd == -1) {
        perror("Error opening file");
        exit(1);
    }

    // Read the file line by line and print the first 5 student grades
    char buffer[1024];
    int bytesRead;
    int count = 0;
    while ((bytesRead = read(fd, buffer, sizeof(buffer))) > 0 && count < 5) {
        // Split the buffer into lines and print them
        char *line = strtok(buffer, "\n");
        while (line != NULL && count < 5) {
            printf("%s\n", line);
            count++;
            line = strtok(NULL, "\n");
        }
    }

    // Close the file
    close(fd);
}

// Function to display the student grades between the given range in the file
void listSomeStudentGrades(char *fileName, int numEntries, int pageNumber) {
    int pid = fork();
    if (pid == -1) {
        perror("Error forking");
        exit(1);
    }
    if (pid == 0)
    {
    // Open the file in read mode
    int fd = open(fileName, O_RDONLY);
    if (fd == -1) {
        perror("Error opening file");
        exit(1);
    }

    // Read the file line by line and print the student grades between the given range
    char buffer[1024];
    int bytesRead;
    int count = 0;
    int start = (pageNumber - 1) * numEntries;
    int end = start + numEntries;
    while ((bytesRead = read(fd, buffer, sizeof(buffer))) > 0 && count < end) {
        // Split the buffer into lines and print them if within the range
        char *line = strtok(buffer, "\n");
        while (line != NULL && count < end) {
            if (count >= start) {
                printf("%s\n", line);
            }
            count++;
            line = strtok(NULL, "\n");
        }
    }

    // Close the file
    close(fd);
    }
    else
    {
        waitpid(pid, NULL, 0);
    }
}

// Function to display the usage of the program
void usage() {
  printf("Usage:\n");
  printf("gtuStudentGrades add <name> <grade>\n");
  printf("gtuStudentGrades search <name>\n");
  printf("gtuStudentGrades sort <file> <type>\n");
  printf("gtuStudentGrades showAll <file>\n");
  printf("gtuStudentGrades listFirst5 <file>\n");
  printf("gtuStudentGrades listSome <numEntries> <pageNumber> <file>\n");
}

int main() {
    while (1) {
      // Get the command from the user
      char command[1024];
      printf("Enter a command (type 'help' for help): ");
      gets(command);

      // Parse the command
      char *args[1024];
      int argc = 0;

      // DFA to parse the command the way that both accepts spaces and double quotes as seperators
        int state = 0;
        for (int i = 0; i < strlen(command); i++) {
            if (isalnum(command[i]) && state == 0) {
                args[argc] = &command[i];
                argc++;
                state = 1;
            } else if (command[i] == ' ' && state == 1) {
                command[i] = '\0';
                state = 0;
            } else if (command[i] == '\"' && state == 0) {
                args[argc] = &command[i + 1];
                argc++;
                state = 2;
            } else if (command[i] == '\"' && state == 2) {
                command[i] = '\0';
                state = 0;
            }
        }

      // Execute the command
      if (strcicmp(args[0], "addStudentGrade") == 0) {
        if (argc != 3) {
          printf("Invalid number of arguments for 'add' command\n");
          usage();
        } else {
          addStudentGrade(args[1], args[2]);
        }
      } else if (strcicmp(args[0], "searchStudent") == 0) {
        if (argc != 2) {
          printf("Invalid number of arguments for 'search' command\n");
          usage();
        } else {
            searchStudentGrade(args[1]);
        }
        } else if (strcicmp(args[0], "sort") == 0) {
        if (argc != 3) {
          printf("Invalid number of arguments for 'sort' command\n");
          usage();
        } else {
          sortStudentGrades(args[1], atoi(args[2]));
        }
        } else if (strcicmp(args[0], "showAll") == 0) {
            if (argc != 2) {
              printf("Invalid number of arguments for 'showAll' command\n");
              usage();
            } else {
              showAllStudentGrades(args[1]);
            }
        } else if (strcicmp(args[0], "listFirst5") == 0) {
            if (argc != 2) {
              printf("Invalid number of arguments for 'listFirst5' command\n");
              usage();
            } else {
              listFirst5StudentGrades(args[1]);
            }
        } else if (strcicmp(args[0], "listSome") == 0) {
            if (argc != 4) {
              printf("Invalid number of arguments for 'listSome' command\n");
              usage();
            } else {
              listSomeStudentGrades(args[3], atoi(args[1]), atoi(args[2]));
            }
        } else if (strcicmp(args[0], "help") == 0) {
            usage();
        } else {
            printf("Invalid command\n");
            usage();
        }
        free(token);
    }

}