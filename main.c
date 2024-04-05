#include <stdio.h>
// #include <curses.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

// VARIABLES---------------------------------------------
#define MAX_ROWS 100
#define MAX_COLS 100
#define MAX_DIRECTIONS 4

char charThread = '?';
int rowsQty, colsQty;
int foundExit = 0;
int threadsQty = 0;
// Define the mutex
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

// ENUMS-----------------------------------------------
enum Direction
{
    UP,
    DOWN,
    LEFT,
    RIGHT
};

enum threadStatus
{
    RUNNING,
    FINISHED_WITHOUT_EXIT,
    FINISHED_WITH_EXIT
};

// STRUCTS---------------------------------------------
struct thread_data
{
    int threadId;
    int x;
    int y;
    enum Direction direction;
    int steps;
    enum threadStatus status;
};

struct thread_data *thread_data = NULL;

typedef struct
{
    char labyrinth;
    enum Direction direction[MAX_DIRECTIONS];
    int directionsQty;
} labyrinth_data;

labyrinth_data labyrinth[MAX_ROWS][MAX_COLS];

// FUNCTIONS-------------------------------------------

/*----------------------------------------------------
Creates a new thread
Entries:
   x: x position
   y: y position
   direction: direction of the thread
   steps: number of steps
Output:
   void
-----------------------------------------------------*/
void createThread(int x, int y, enum Direction direction, int steps);

/*----------------------------------------------------
Set cursor position
Entries:
   x: x position
   y: y position
Output:
   void
-----------------------------------------------------*/
void setCursor(int x, int y, enum Direction direction)
{
    x = ((x + 1) * 3) + x;
    y = y + 2;

    if(direction == UP){
        charThread = '^';
        printf("\033[36m"); // Cian
    } else if(direction == DOWN){
        charThread = 'v';
        printf("\033[35m"); // Magenta
    } else if(direction == LEFT){
        charThread = '<';
        printf("\033[34m"); // Azul
    } else if(direction == RIGHT){
        charThread = '>';
        printf("\033[33m"); // Amarillo
    }
    printf("\033[%d;%dH", y, x);
    fflush(stdout);

    printf("%c", charThread);
    fflush(stdout);

    printf("\033[0m"); 
    fflush(stdout);

    printf("\033[%d;%dH", rowsQty + 4, 1);
    fflush(stdout);
}

/*----------------------------------------------------
Prints a separator line on the console
Entries:
   None
Output:
   void
-----------------------------------------------------*/
void printSeparator()
{
    printf("-");
    for (int j = 0; j < colsQty * 2; j++)
    {
        printf("--");
    }
    printf("-\n");
}

/*----------------------------------------------------
Prints the labyrinth on the console
Entries:
   None
Output:
   void
-----------------------------------------------------*/
void printLabyrinth()
{
    system("clear");
    printSeparator();
    // sleep(2);

    // prints each row of the labyrinth
    printf("|");
    for (int i = 0; i < rowsQty; i++)
    {
        for (int j = 0; j < colsQty; j++)
        {
            printf(" %c |", labyrinth[i][j].labyrinth);
        }

        if (i < rowsQty)
        {
            printf("\n|");
        }
        else
        {
            printf("\n");
        }
    }

    printSeparator();
}

/*----------------------------------------------------
reads the labyrinth from a file and stores it in a matrix
set the number of rows and columns of the labyrinth
Entries:
   fileName: name of the file to read
Output:
   void
-----------------------------------------------------*/
void readLabyrinth(char *fileName)
{
    FILE *file;
    file = fopen(fileName, "r");

    if (file == NULL)
    {
        printf("Error opening file\n");
        return;
    }

    // obtains the number of rows and columns
    fscanf(file, "%d %d", &rowsQty, &colsQty);
    fgetc(file);
    fgetc(file);

    // reads the file content and stores it in the labyrinth matrix of chars
    int row = 0;
    int column = 0;
    int c;
    while ((c = fgetc(file)) != EOF)
    {
        if (c == '\n')
        {
            row++;
            column = 0;
        }
        else
        {
            labyrinth[row][column].labyrinth = c;
            labyrinth[row][column].directionsQty = 0;
            column++;
        }
    }

    fclose(file);
}

/*----------------------------------------------------
Saves the information of a thread
Entries:
    x: x position
    y: y position
    direction: direction of the thread
    steps: number of steps
    status: status of the thread
Output:
    void
-----------------------------------------------------*/
void saveThreadInfo(int x, int y, enum Direction direction, int steps, enum threadStatus status) {
    size_t new_size = (threadsQty + 1) * sizeof(struct thread_data);
    if (new_size < (threadsQty + 1) || new_size / sizeof(struct thread_data) != (threadsQty + 1)) {
        printf("Error: desbordamiento de entero al asignar memoria\n");
        return;
    }

    thread_data = realloc(thread_data, new_size);
    if (thread_data == NULL) {
        printf("Error: no se pudo asignar memoria\n");
        return;
    }

    thread_data[threadsQty].threadId = threadsQty + 1;
    thread_data[threadsQty].x = x;
    thread_data[threadsQty].y = y;
    thread_data[threadsQty].direction = direction;
    thread_data[threadsQty].steps = steps;
    thread_data[threadsQty].status = status;

}

/*----------------------------------------------------
Updates the information of a thread
Entries:
    id: thread id
    x: x position
    y: y position
    steps: number of steps
    status: status of the thread
Output:
    void
-----------------------------------------------------*/
void updateThreadInfo(int id, int x, int y, int steps, enum threadStatus status)
{
    for (int i = 0; i < threadsQty; i++) {
        if (thread_data[i].threadId == id) {
            thread_data[i].status = status;
            thread_data[i].steps = steps;
            thread_data[i].x = x;
            thread_data[i].y = y;
            //thread_data[i].direction = direction;

            break;
        }
    }
}

/*----------------------------------------------------
Prints the statistics of the threads
Entries:
    None
Output:
    void
-----------------------------------------------------*/
void printStatistics() {
    printf("Cantidad de threads creados: %d\n", threadsQty);

    printf("Información de los threads:\n");

    for (int i = 0; i < threadsQty; i++) {
        printf("\nThread %d:\n", thread_data[i].threadId);
        printf("  Posición Inicial: (%d, %d)\n", thread_data[i].x, thread_data[i].y);
        printf("  Dirección: %d\n", thread_data[i].direction);
        printf("  Pasos: %d\n", thread_data[i].steps);
        printf("  Estado: %d\n", thread_data[i].status);
    }
}

/*----------------------------------------------------
Verifies if the thread can move in a specific direction
Entries:
    x: x position
    y: y position
    direction: direction of the thread
Output:
    void
-----------------------------------------------------*/
void verifyPath(int x, int y, enum Direction direction)
{
    bool flag = true;
    for (int dir = 0; dir < MAX_DIRECTIONS; dir++)
    {
        flag = true;
        int newX = x, newY = y;
        if (dir == 0 && direction == UP){
        //printf("Case 1");
        flag = !flag;
            continue;}
        if (dir == 1 && direction == DOWN){
        flag = !flag;
            continue;}
        if (dir == 2 && direction == LEFT){
        flag = !flag;
            continue;}
        if (dir == 3 && direction == RIGHT){
        flag = !flag;
            continue;}

        switch (dir)
        {
        case 0: // up
            newY--;
            break;
        case 1: // down
            newY++;
            break;
        case 2: // left
            newX--;
            break;
        case 3: // right
            newX++;
            break;
        }

        // bool hasPassedDirection = false;
        // for (int i = 0; i < labyrinth[y][x].directionsQty; i++) {
        //     if (labyrinth[y][x].direction[i] == direction) {
        //         hasPassedDirection = true;
        //         printf("Ha pasado por la direccion %d\n", direction);
        //         break;
        //     }
        // }
        
        if (newX >= 0 && newX < colsQty && newY >= 0 && newY < rowsQty &&
            labyrinth[newY][newX].labyrinth == ' ' && labyrinth[newY][newX].directionsQty == 0 && flag)
        {
            createThread(newX, newY, (enum Direction)dir, 0);
        }
        // if (!hasPassedDirection) {
        //     createThread(newX, newY, (enum Direction)dir, 0);
        // }
    }
}



/*----------------------------------------------------
Moves the thread in the labyrinth
Entries:
   arg: thread data
Output:
   void
-----------------------------------------------------*/
void *moveThread(void *arg)
{
    // Extracts the thread data
    struct thread_data *data = (struct thread_data *)arg;
    int id = data->threadId;
    int x = data->x;
    int y = data->y;
    enum Direction direction = data->direction;
    int steps = data->steps;
    enum threadStatus status = data->status;
    free(data);

    // Sets the the thread in the initial position
    if ((x >= 0 && x < colsQty) && (y >= 0 && y < rowsQty) &&
        (labyrinth[y][x].labyrinth != '*' && labyrinth[y][x].labyrinth != '/'))
    {
        labyrinth[y][x].labyrinth = charThread;

        pthread_mutex_lock(&mutex);
        setCursor(x, y, direction);
        pthread_mutex_unlock(&mutex);
        sleep(1);
    }

    while (!foundExit)
    {
        int nextX = x, nextY = y;
        verifyPath(x, y, direction);

        switch (direction)
        {
        case UP:
            nextY--;
            break;
        case DOWN:
            nextY++;
            break;
        case LEFT:
            nextX--;
            break;
        case RIGHT:
            nextX++;
            break;
        }

        // Verifies if the next step is out of bounds
        if (nextX < 0 || nextX >= colsQty || nextY < 0 || nextY >= rowsQty)
        {
            break;
        }

        // Verifies if the next step is an obstacle or the exit
        if (labyrinth[nextY][nextX].labyrinth == '*')
        {
            status = FINISHED_WITHOUT_EXIT;
            updateThreadInfo(id, x, y, steps, status);
            break;
        }

        // Verifies if the next step is the exit
        if (labyrinth[nextY][nextX].labyrinth == '/')
        {
            foundExit = 1;
            status = FINISHED_WITH_EXIT;
            updateThreadInfo(id, x, y, steps, status);
            break;
        }

        x = nextX;
        y = nextY;
        
        pthread_mutex_lock(&mutex);
        labyrinth[y][x].labyrinth = charThread;
        labyrinth[y][x].direction[labyrinth[y][x].directionsQty] = direction;
        labyrinth[y][x].directionsQty++;
        setCursor(x, y, direction);
        updateThreadInfo(id, x, y, steps, status);
        pthread_mutex_unlock(&mutex);
        

        
        usleep(500000);
        sleep(1);
    }

    return NULL;
}

/*----------------------------------------------------
Creates a new thread
Entries:
   x: x position
   y: y position
   direction: direction of the thread
   steps: number of steps
Output:
   void
-----------------------------------------------------*/
void createThread(int x, int y, enum Direction direction, int steps)
{
    
    
    struct thread_data *data = malloc(sizeof(struct thread_data));
    if (data == NULL)
    {
        printf("Error\n");
        return;
    }

    data->x = x;
    data->y = y;
    data->direction = direction;
    data->steps = steps;
    data->status = RUNNING;

    pthread_t thread;
    if (pthread_create(&thread, NULL, moveThread, (void *)data) != 0)
    {
        printf("Error al crear el thread\n");
        free(data);
        return;
    }


    saveThreadInfo(x, y, direction, steps, RUNNING);

    threadsQty++;
    
    pthread_detach(thread);
}

// MAIN-----------------------------------------------
int main()
{
    printf("Iniciando programa...\n");
    readLabyrinth("inputs/lab1.txt");
    printLabyrinth();
    createThread(0, 0, DOWN, 0);
    while (!foundExit)
    {

    }

    printf("Salida encontrada. Terminando programa.\n");
    //printf("Cantidad de threads creados: %d\n", threadsQty);
    sleep(2);
    printStatistics();

    free(thread_data);

    return 0;
}