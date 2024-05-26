#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <semaphore.h>
#include <sys/types.h>

#define BUFFER_SIZE 10
#define PATH_MAX 4096

// Structure to hold source and destination file paths
typedef struct {
    char sourcePath[PATH_MAX];
    char destPath[PATH_MAX];
} FilePair;

// Structure to hold source and destination directory paths just like FilePair
typedef struct {
    char sourceDir[PATH_MAX];
    char destDir[PATH_MAX];
} DirPair;

FilePair buffer[BUFFER_SIZE];
int buffer_count = 0;

// Mutex and condition variables for buffer access
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_not_empty = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_not_full = PTHREAD_COND_INITIALIZER;
pthread_barrier_t barrier;

int done = 0;  // Flag to indicate that the manager is done adding files to the buffer

void* manager_function(void* arg);
void* worker_function(void* arg);


int main(int argc, char* argv[]) {
    if (argc != 5) {
        fprintf(stderr, "Usage: %s <buffer_size> <num_workers> <source_dir> <dest_dir>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int buffer_size = atoi(argv[1]);
    int num_workers = atoi(argv[2]);
    char* source_dir = argv[3];
    char* dest_dir = argv[4];

    pthread_t manager_thread;
    pthread_t worker_threads[num_workers];

    DirPair dirs;
    strcpy(dirs.sourceDir, source_dir);
    strcpy(dirs.destDir, dest_dir);

    // Initialize the barrier
    pthread_barrier_init(&barrier, NULL, num_workers + 1);

    // Create the manager thread
    pthread_create(&manager_thread, NULL, manager_function, (void*)&dirs);

    // Create worker threads
    for (int i = 0; i < num_workers; i++) {
        pthread_create(&worker_threads[i], NULL, worker_function, NULL);
    }

    // Wait for the manager thread to finish
    pthread_join(manager_thread, NULL);

    // Wait for worker threads to finish
    for (int i = 0; i < num_workers; i++) {
        pthread_join(worker_threads[i], NULL);
    }

    // Destroy the barrier
    pthread_barrier_destroy(&barrier);

    return 0;
}




void* manager_function(void* arg) {
    DirPair* dirs = (DirPair*)arg;
    char* source_dir = dirs->sourceDir;
    char* dest_dir = dirs->destDir;
    struct dirent *entry;
    DIR *dp = opendir(source_dir);

    if (dp == NULL) {
        perror("opendir");
        return NULL;
    }

    while ((entry = readdir(dp))) {
        if (entry->d_type == DT_DIR) {
            // Skip the "." and ".." directories
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                continue;
            }
            // Construct new paths for recursive directory reading
            char new_source_path[PATH_MAX];
            char new_dest_path[PATH_MAX];
            snprintf(new_source_path, sizeof(new_source_path), "%s/%s", source_dir, entry->d_name);
            snprintf(new_dest_path, sizeof(new_dest_path), "%s/%s", dest_dir, entry->d_name);

            // Create the directory in the destination
            mkdir(new_dest_path, 0777);

            // Recursively read the new directory
            DirPair new_dirs = { .sourceDir = "", .destDir = "" };
            strcpy(new_dirs.sourceDir, new_source_path);
            strcpy(new_dirs.destDir, new_dest_path);
            manager_function((void*)&new_dirs);
        } else {
            // Construct full file paths for the source and destination
            char source_path[PATH_MAX];
            char dest_path[PATH_MAX];
            snprintf(source_path, sizeof(source_path), "%s/%s", source_dir, entry->d_name);
            snprintf(dest_path, sizeof(dest_path), "%s/%s", dest_dir, entry->d_name);

            pthread_mutex_lock(&mutex);

            while (buffer_count == BUFFER_SIZE) {
                pthread_cond_wait(&cond_not_full, &mutex);
            }

            // Add file pair to buffer
            strcpy(buffer[buffer_count].sourcePath, source_path);
            strcpy(buffer[buffer_count].destPath, dest_path);
            buffer_count++;

            pthread_cond_signal(&cond_not_empty);
            pthread_mutex_unlock(&mutex);
        }
    }

    closedir(dp);

    pthread_mutex_lock(&mutex);
    done = 1;
    pthread_cond_broadcast(&cond_not_empty);
    pthread_mutex_unlock(&mutex);

    return NULL;
}



void* worker_function(void* arg) {
    while (1) {
        pthread_mutex_lock(&mutex);

        while (buffer_count == 0 && !done) {
            pthread_cond_wait(&cond_not_empty, &mutex);
        }

        if (buffer_count == 0 && done) {
            pthread_mutex_unlock(&mutex);
            break;
        }

        // Remove file pair from buffer
        FilePair filePair = buffer[--buffer_count];

        pthread_cond_signal(&cond_not_full);
        pthread_mutex_unlock(&mutex);

        // Open the source file for reading
        int src_fd = open(filePair.sourcePath, O_RDONLY);
        if (src_fd < 0) {
            perror("open source file");
            continue;
        }

        // Create or truncate the destination file
        int dest_fd = open(filePair.destPath, O_WRONLY | O_CREAT | O_TRUNC, 0666);
        if (dest_fd < 0) {
            perror("open destination file");
            close(src_fd);
            continue;
        }

        // Copy the file contents
        char buffer[8192];
        ssize_t bytes_read, bytes_written;
        while ((bytes_read = read(src_fd, buffer, sizeof(buffer))) > 0) {
            bytes_written = write(dest_fd, buffer, bytes_read);
            if (bytes_written != bytes_read) {
                perror("write");
                break;
            }
        }

        if (bytes_read < 0) {
            perror("read");
        }

        close(src_fd);
        close(dest_fd);

        // Output the completion status
        printf("Copied %s to %s\n", filePair.sourcePath, filePair.destPath);
    }

    // Synchronize with other threads
    pthread_barrier_wait(&barrier);
    return NULL;
}
