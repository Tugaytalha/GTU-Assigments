#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h> // For open(), read(), write()
#include <ctype.h>

// Function to append a log entry to the log file
// This function will be called by only child processes (except forking error case but if we can't create a child process,
// we can't write to log file with child) that's why we are not forking again in this function
int appendtolog(char *logentry)
{
    int fd = open("operations.log", O_WRONLY | O_APPEND | O_CREAT, 0644);
    if (fd == -1)
    {
        perror("Error opening log file");
        return -1;
    }

    if (write(fd, logentry, strlen(logentry)) == -1)
    {
        perror("Error writing to log file");
        return -1;
    }

    close(fd);
    return 0;
}

int strcicmp(char const *a, char const *b)
{
    for (;; a++, b++) {
        int d = tolower((unsigned char)*a) - tolower((unsigned char)*b);
        if (d != 0 || !*a)
            return d;
    }
}

// Function to split a certain format string to grade
char * gradetok(char *str)
{
    static char *p;
    if (str)
    {
        p = str;
    }
    if (*p == '\0')
    {
        return NULL;
    }
    char *ret = NULL;
    while (*p != '\0' && *p != ',')
    {
        p++;
    }
    if (*p == ',' && *(++p) == ' ')
    {
        ret = p;
        while (*p != '\0' && *p != '\n')
        {
            p++;
        }
        if (*p == '\n')
        {
            *p = '\0';
        }
    }
    return ret;
}


// Function to add a student grade to the file
void addStudentGrade(char *fileName, char *name, char *grade) {
    int pid = fork();
    if (pid == -1) {
        perror("Error forking");
        appendtolog("Error forking in addStudentGrade\n");
        exit(1);
    }
    if (pid == 0)
    {// Open the file in append mode (create if it doesn't exist)
        int fd = open(fileName, O_WRONLY | O_APPEND | O_CREAT, 0644);
        if (fd == -1) {
          perror("Error opening file");
          char errstr[1024];
            sprintf(errstr, "Error opening file in addStudentGrade %s %s %s\n", name, grade, fileName);
          appendtolog(errstr);
          exit(1);
        }

        // Prepare the data to write
        char data[10240];
        if (strlen(grade) > 2)
        {
            printf("Invalid grade\n");
            char errstr[1024];
            sprintf(errstr, "Invalid grade in addStudentGrade %s %s %s\n", name, grade, fileName);
            appendtolog(errstr);
            _exit(1);
        }
        sprintf(data, "%s, %s\n", name, grade);

        // Write the data to the file
        if (write(fd, data, strlen(data)) == -1) {
            perror("Error writing to file");
            char errstr[1024];
            sprintf(errstr, "Error writing to file in addStudentGrade %s %s %s\n", name, grade, fileName);
            appendtolog(errstr);
            exit(1);
        }

        // Close the file
        close(fd);
        char logstr[1024];
        sprintf(logstr, "Added student grade %s %s %s\n", name, grade, fileName);
        appendtolog(logstr);
        _exit(EXIT_SUCCESS);
    }
    else
    {
        waitpid(pid, NULL, 0);
    }
}


// Function to search for a student's grade by name
void searchStudentGrade(char *fileName, char *name) {
    int pid = fork();
    if (pid == -1) {
        perror("Error forking");
        appendtolog("Error forking in searchStudentGrade\n");
        exit(1);
    }
    if (pid == 0)
    {
        // Open the file in read mode
        int fd = open(fileName, O_RDONLY);
        if (fd == -1) {
            perror("Error opening file");
            char errstr[1024];
            sprintf(errstr, "Error opening file in searchStudentGrade %s %s\n", name, fileName);
            appendtolog(errstr);
            exit(1);
        }

        // Read the file line by line
        char line[20480];
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
            char *studentName = strtok(line, ",");
            char *studentGrade = strtok(NULL, "\n");
            do
            {
                // Check if the student's name matches the search name
                if (strcicmp(studentName, name) == 0) {
                  // Print the student's name and grade
                  printf("%s, %s\n", studentName, studentGrade);
                  flags[0] = 1;
                  flags[2] = 1;
                }

                studentName = strtok(NULL, ",");
                studentGrade = strtok(NULL, "\n");

                if (studentName == NULL)
                {
                    flags[0] = 1;
                    flags[1] = 1;
                }
            } while (flags[0] == 0);
        }
        if (flags[0] == 0) {
            printf("Student not found\n");
            char errstr[1024];
            sprintf(errstr, "Student not found in searchStudentGrade %s %s\n", name, fileName);
            appendtolog(errstr);
        } else if (flags[2] == 1){
            char logstr[1024];
            sprintf(logstr, "Searched student grade %s %s\n", name, fileName);
            appendtolog(logstr);
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


// Function to sort the student grades in the file
void sortStudentGrades(char *fileName, int sortType) {
    int pid = fork();
    if (pid == -1) {
        perror("Error forking");
        appendtolog("Error forking in sortStudentGrades\n");
        exit(1);
    }
    if (pid == 0)
    {
        // Open the file in read mode
        int fd = open(fileName, O_RDONLY);
        if (fd == -1) {
            perror("Error opening file");
            char errstr[1024];
            sprintf(errstr, "Error opening file in sortStudentGrades %s %d\n", fileName, sortType);
            appendtolog(errstr);
            exit(1);
        }

        // Read the file line by line and store the student grades in an array
        char lines[1024][1024];
        int lineCount = 0;
        char buffer[10240];
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
        for (int i = 0; i < lineCount - 1; i++) {
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
                  char *grade1 = gradetok(lines[i]);
                  char *grade2 = gradetok(lines[j]);
                  if (strcicmp(grade1, grade2) > 0) {
                    char temp[1024];
                    strcpy(temp, lines[i]);
                    strcpy(lines[i], lines[j]);
                    strcpy(lines[j], temp);
                  }
                } else if (sortType == 3) {
                  // Sort by grade in descending order
                  char *grade1 = gradetok(lines[i]);
                  char *grade2 = gradetok(lines[j]);

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
                    char errstr[1024];
                    sprintf(errstr, "Invalid sort type in sortStudentGrades %s %d\n", fileName, sortType);
                    appendtolog(errstr);
                    _exit(1);
                }
            }
        }

        // Print the sorted student grades
        for (int i = 0; i < lineCount; i++) {
            printf("%s\n", lines[i]);
        }
        char logstr[1024];
        sprintf(logstr, "Sorted student grades %s %d\n", fileName, sortType);
        appendtolog(logstr);
        _exit(EXIT_SUCCESS);
    }
    else
    {
        wait(NULL);
    }
}


// Function to display all student grades in the file
void showAllStudentGrades(char *fileName) {
    int pid = fork();
    if (pid == -1) {
        perror("Error forking");
        appendtolog("Error forking in showAllStudentGrades\n");
        exit(1);
    }
        if (pid == 0)
        {
        // Open the file in read mode
        int fd = open(fileName, O_RDONLY);
        if (fd == -1) {
            perror("Error opening file");
            char errstr[1024];
            sprintf(errstr, "Error opening file in showAllStudentGrades %s\n", fileName);
            appendtolog(errstr);
            exit(1);
        }

        // Read the file line by line and print the student grades
        char buffer[10240];
        int bytesRead;
        while ((bytesRead = read(fd, buffer, sizeof(buffer))) > 0) {
            write(STDOUT_FILENO, buffer, bytesRead);
        }

        // Close the file
        close(fd);
        char logstr[1024];
        sprintf(logstr, "Showed all student grades %s\n", fileName);
        appendtolog(logstr);
        _exit(EXIT_SUCCESS);
    }
    else
    {
        waitpid(pid, NULL, 0);
    }
}


// Function to display the first 5 student grades in the file
void listFirst5StudentGrades(char *fileName) {
    int pid = fork();
    if (pid == -1) {
        perror("Error forking");
        appendtolog("Error forking in listFirst5StudentGrades\n");
        exit(1);
    }
    if (pid == 0)
    {
        // Open the file in read mode
        int fd = open(fileName, O_RDONLY);
        if (fd == -1) {
            perror("Error opening file");
            char errstr[1024];
            sprintf(errstr, "Error opening file in listFirst5StudentGrades %s\n", fileName);
            appendtolog(errstr);
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
        char logstr[1024];
        sprintf(logstr, "Listed first 5 student grades %s\n", fileName);
        appendtolog(logstr);
        _exit(EXIT_SUCCESS);
    }
    else
    {
        waitpid(pid, NULL, 0);
    }
}


// Function to display the student grades between the given range in the file
void listSomeStudentGrades(char *fileName, int numEntries, int pageNumber) {
    int pid = fork();
    if (pid == -1) {
        perror("Error forking");
        appendtolog("Error forking in listSomeStudentGrades\n");
        exit(1);
    }
    if (pid == 0)
    {
        // Open the file in read mode
        int fd = open(fileName, O_RDONLY);
        if (fd == -1) {
            perror("Error opening file");
            char errstr[1024];
            sprintf(errstr, "Error opening file in listSomeStudentGrades %s %d %d\n", fileName, numEntries, pageNumber);
            appendtolog(errstr);
            exit(1);
        }

        // Read the file line by line and print the student grades between the given range
        char buffer[10240];
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
        if (count < start) {
            printf("No student grades in the given range\n");
            char errstr[1024];
            sprintf(errstr, "No student grades in the given range %s %d %d\n", fileName, numEntries, pageNumber);
            appendtolog(errstr);
        } else {
            char logstr[1024];
            sprintf(logstr, "Listed student grades in the given range %s %d %d\n", fileName, numEntries, pageNumber);
            appendtolog(logstr);
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


// Function to create empty file
void gtuStudentGrades(char *fileName) {
    int pid = fork();
    if (pid == -1) {
        perror("Error forking");
        appendtolog("Error forking in gtuStudentGrades\n");
        exit(1);
    }
    if (pid == 0)
    {
        // Create the file in write mode (if exists, empty it)
        int fd = open(fileName, O_WRONLY | O_TRUNC | O_CREAT, 0644);
        if (fd == -1) {
            perror("Error opening file");
            char errstr[1024];
            sprintf(errstr, "Error opening file in gtuStudentGrades %s\n", fileName);
            appendtolog(errstr);
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


// Function to display the usage of the program
void usage() {
    printf("Usage:\n");
    printf("gtuStudentGrades <file>  // Creates a file with the given name\n");
    printf("addStudentGrade <name> <grade> <file>  // Add the student to the given file with given grade \n");
    printf("searchStudent <name> <file> // Search the student in the given file\n");
    printf("sortAll <file> <type>  // Sort the students in the given file by the given type. Types:\n\t0: Sort by name in ascending order\n\t1: Sort by name in descending order\n\t2: Sort by grade in ascending order\n\t3: Sort by grade in descending order\n");
    printf("showAll <file>  // Show all student grades in the given file\n");
    printf("listGrades <file>  // List the first 5 student grades in the given file\n");
    printf("listSome <numEntries> <pageNumber> <file>  // List the student grades between the given range in the given file\n");
}


int main() {
    while (1) {
      // Get the command from the user
      char command[10240];
      printf("Enter a command (type 'gtuStudentGrades' for help and 'exit' to exit): ");
      gets(command);

      // Parse the command
      char *args[1024];
      int argc = 0;

      // DFA to parse the command the way that both accepts spaces and double quotes as seperators
        int state = 0;
        int i;
        int len = strlen(command);
        for (i = 0; i < len; i++) {
            if (isalnum(command[i]) && state == 0) {
                args[argc] = &command[i];
                argc++;
                state = 1;
            } else if (command[i] == ' ' && state == 1) {
                command[i] = '\0';
                state = 0;
            } else if (command[i] == '\"' && (state == 0 || state == 1)) {
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
        if (argc != 4) {
          printf("Invalid number of arguments for 'addStudentGrade' command\n");
          usage();
        } else {
          addStudentGrade(args[3], args[1], args[2]);
        }
      } else if (strcicmp(args[0], "searchStudent") == 0) {
        if (argc != 3) {
          printf("Invalid number of arguments for 'searchStudent' command\n");
          usage();
        } else {
            searchStudentGrade(args[2], args[1]);
        }
        } else if (strcicmp(args[0], "sortAll") == 0) {
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
        } else if (strcicmp(args[0], "listGrades") == 0) {
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
        } else if (strcicmp(args[0], "gtuStudentGrades") == 0) {
            if (argc == 2) {
              gtuStudentGrades(args[1]);
            } else if (argc == 1) {
              usage();
            } else {
              printf("Invalid number of arguments for 'gtuStudentGrades' command\n");
              usage();
            }
        } else if (strcicmp(args[0], "exit") == 0) {
            appendtolog("Exiting\n");
            break;
        }
        else {
            printf("Invalid command\n");
            usage();
        }
    }

    return 0;
}