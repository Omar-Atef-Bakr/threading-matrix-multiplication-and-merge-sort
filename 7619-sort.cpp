#include <cstdio>
#include <cstdlib>
#include <pthread.h>

int* DATA;
int SIZE;

void read_array(char *filename);
void *sort(void *args);


struct sort_args {
    int start;
    int end;
};

int main(int argc, char **argv) {

    // check for correct number of arguments
    if (argc != 2) {
        printf("Usage: %s <filename>\n", argv[0]);
        exit(1);
    }

    // get input filename
    char *FILENAME = argv[1];

    // read array from file
    read_array(FILENAME);

    // sort array
    struct sort_args *args = (struct sort_args *) malloc(sizeof(struct sort_args));
    args->start = 0;
    args->end = SIZE-1;

    sort((void *) args);

    // print sorted array
    for (int i = 0; i < SIZE; i++) {
        printf("%d ", DATA[i]);
    }
    printf("\n");

    return 0;
}

void read_array(char *filename) {

    // open file
    FILE *file = fopen(filename, "r");
    if (file == nullptr) {
        printf("Error opening file\n");
        exit(1);
    }

    // read array
    fscanf(file, "%d", &SIZE);
    DATA = (int *) malloc(sizeof(int) * SIZE);

    for(int i = 0; i < SIZE; i++){
        fscanf(file, "%d", &DATA[i]);
    }

    // close file
    fclose(file);
}

void *sort(void *arg) {

    // get bounds
    struct sort_args *bounds = (struct sort_args *) arg;
    int start = bounds->start;
    int end = bounds->end;


    // base case
    if (start >= end) {
        return NULL;
    }

    int mid = (start + end) / 2;

    // sort left half
    struct sort_args *left_args = (struct sort_args *) malloc(sizeof(struct sort_args));
    left_args->start = start;
    left_args->end = mid;
    pthread_t tids[2];
    pthread_create(&tids[0], NULL, sort, (void *) left_args);

    pthread_join(tids[0], NULL);
    // sort right half
    struct sort_args *right_args = (struct sort_args *) malloc(sizeof(struct sort_args));
    right_args->start = mid+1;
    right_args->end = end;
    pthread_create(&tids[1], NULL, sort, (void *) right_args);

    // wait for threads to finish
    pthread_join(tids[1], NULL);

    // merge halves
    int *temp = (int *) malloc(sizeof(int) * (end - start + 1));
    int i = start;
    int j = mid + 1;
    int k = 0;

    while (i <= mid && j <= end) {
        if (DATA[i] < DATA[j]) {
            temp[k] = DATA[i];
            i++;
        } else {
            temp[k] = DATA[j];
            j++;
        }
        k++;
    }

    // empty remaining elements
    while (i <= mid) {
        temp[k] = DATA[i];
        i++;
        k++;
    }

    while (j <= end) {
        temp[k] = DATA[j];
        j++;
        k++;
    }

    // copy temp to DATA
    for (i = start; i <= end; i++) {
        DATA[i] = temp[i - start];
    }

    return NULL;
}