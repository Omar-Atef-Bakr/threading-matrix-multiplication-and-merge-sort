#include <cstdio>
#include <cstdlib>
#include <pthread.h>
#include <ctime>

struct Matrix{
    int **m;
    int rows;
    int cols;
};

void read_matricies(char *filename, struct Matrix *m1, struct Matrix *m2);
void read_matrix(FILE *fp, struct Matrix *m1);
void print_matrix(int **m, int rows, int cols);
void multiply_elements();
void multiply_rows();
void output_matrix(FILE *file);
void free_matrix(struct Matrix *m);

struct Matrix *matrix1, *matrix2;
struct Matrix *result;

const char* OUTFILE = "output.txt";

int main(int argc, char **argv){

    // check for correct number of arguments
    if (argc != 2){
        printf("Usage: %s <filename>\n", argv[0]);
        exit(1);
    }

    // get input filename
    char *FILENAME = argv[1];

    // read matrices from file
    matrix1 = (struct Matrix *)malloc(sizeof(struct Matrix));
    matrix2 = (struct Matrix *)malloc(sizeof(struct Matrix));
    read_matricies(FILENAME, matrix1, matrix2);

    // print matrices
    print_matrix(matrix1->m, matrix1->rows, matrix1->cols);
    print_matrix(matrix2->m, matrix2->rows, matrix2->cols);



    // multiply matrices element at a time and time it
    clock_t start, end;
    double cpu_time_used;

    start = clock();
    multiply_elements();
    end = clock();

    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    print_matrix(result->m, result->rows, result->cols);
    printf("time: %f\n", cpu_time_used);

    // output result to file
    FILE *file = fopen(OUTFILE, "w");
    if (file == nullptr){
        printf("Error opening file\n");
        exit(1);
    }
    output_matrix(file);
    fprintf(file, "END1 %f\n", cpu_time_used);

    // multiply matrices row at a time
    start = clock();
    multiply_rows();
    end = clock();

    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    print_matrix(result->m, result->rows, result->cols);
    printf("time: %f\n", cpu_time_used);

    // output result to file
    output_matrix(file);
    fprintf(file, "END2 %f\n", cpu_time_used);

    // close files and free memory
    fclose(file);
    free_matrix(matrix1);
    free_matrix(matrix2);
    free_matrix(result);

    return 0;
}

void free_matrix(struct Matrix *m){
    for (int i = 0; i < m->rows; i++) {
        free(m->m[i]);
    }
    free(m->m);
    free(m);
    return;
}

void read_matricies(char *filename, struct Matrix *m1, struct Matrix *m2){

    // open file
    FILE *file = fopen(filename, "r");
    if (file == nullptr){
        printf("Error opening file\n");
        exit(1);
    }

    // read matrices
    read_matrix(file, m1);
    read_matrix(file, m2);

    if(m1->cols != m2->rows){
        printf("Cannot multiply matrices: Dimensions error\n");
        fclose(file);
        file = fopen(OUTFILE, "w");
        fprintf(file, "Multipication of Matrix is not Possible !!");
        fclose(file);
        exit(1);
    }
    fclose(file);
    return;
}

void read_matrix(FILE *fp, struct Matrix *m){
    // read matrix dimensions
    int *rows, *cols;
    fscanf(fp, "%d %d", &m->rows, &m->cols);

    // allocate memory for matrix 1
    m->m = (int **)malloc(m->rows * sizeof(int *));

    // read matrix 1 values
    for (int i = 0; i < m->rows; i++){
        // allocate memory for each row
        m->m[i] = (int *)malloc(m->cols * sizeof(int));

        // read values
        for (int j = 0; j < m->cols; j++){
            fscanf(fp, "%d", &m->m[i][j]);
        }
    }

    return;
}

void print_matrix(int **m, int rows, int cols){
    printf("matrix dimensions: %d %d...\n", rows, cols);
    for (int i = 0; i < rows; i++){
        for (int j = 0; j < cols; j++){
//            printf("element: %d,%d\n", i, j);
            printf("%d ", m[i][j]);
        }
        printf("\n");
    }
    return;
}

void *multiply_element(void *position){

    // get position
    int *pos = (int *)position;
    int row = pos[0];
    int col = pos[1];

    // multiply element
    int sum = 0;
    for (int i = 0; i < matrix1->cols; i++){
        sum += matrix1->m[row][i] * matrix2->m[i][col];
    }

    // store result
    result->m[row][col] = sum;

    return NULL;
}

void multiply_elements(){
    // free memory if result matrix already exists
    if(result != nullptr){
        free_matrix(result);

    }

    // allocate memory for result matrix
    result = (struct Matrix *)malloc(sizeof(struct Matrix));
    result->rows = matrix1->rows;
    result->cols = matrix2->cols;
    result->m = (int **)malloc(result->rows * sizeof(int *));
    for (int i = 0; i < result->rows; i++){
        result->m[i] = (int *)malloc(result->cols * sizeof(int));
    }

    // create array of thread ids
    pthread_t tids[result->rows * result->cols];

    // multiply matrices
    for (int i = 0; i < matrix1->rows; i++){
        for (int j = 0; j < matrix2->cols; j++){
            // create position argument
            int *position = (int *)malloc(2 * sizeof(int));
            position[0] = i;
            position[1] = j;

            // create thread
            pthread_create(&tids[i * result->cols + j], NULL, multiply_element,
                           (void *) position);
        }
    }

     // wait for threads to finish
    for (int i = 0; i < result->rows * result->cols; i++)
        pthread_join(tids[i], NULL);

    return;
}

void *multiply_row(void *row){

    // get row
    int *r = (int *)row;
    int row_num = *r;

    // multiply row
    for (int i = 0; i < matrix2->cols; i++){
        int sum = 0;
        for (int j = 0; j < matrix1->cols; j++){
            sum += matrix1->m[row_num][j] * matrix2->m[j][i];
        }
        result->m[row_num][i] = sum;
    }

    return nullptr;
}

void multiply_rows(){
    // free memory if result matrix already exists
    if(result != nullptr){
        free_matrix(result);
    }

    // allocate memory for result matrix
    result = (struct Matrix *)malloc(sizeof(struct Matrix));
    result->rows = matrix1->rows;
    result->cols = matrix2->cols;
    result->m = (int **)malloc(result->rows * sizeof(int *));
    for (int i = 0; i < result->rows; i++){
        result->m[i] = (int *)malloc(result->cols * sizeof(int));
    }

    // create array of thread ids
    pthread_t tids[result->rows];

    // multiply matrices
    for (int i = 0; i < result->rows; i++){
        // create thread
        pthread_create(&tids[i], NULL, multiply_row,(void *) &i);
        pthread_join(tids[i], NULL);
    }

    // wait for threads to finish
    for (int i = 0; i < result->rows; i++)
        pthread_join(tids[i], NULL);

    return;
}

void output_matrix(FILE *file){
    // write matrix dimensions
    fprintf(file, "%d %d\n", result->rows, result->cols);

    // write matrix values
    for (int i = 0; i < result->rows; i++){
        for (int j = 0; j < result->cols; j++){
            fprintf(file, "%d ", result->m[i][j]);
        }
        fprintf(file, "\n");
    }

    return;
}
