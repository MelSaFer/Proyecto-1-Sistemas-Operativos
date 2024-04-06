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
int controlStats = 0;
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
    int thisThreadId;
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
    controlStats = rowsQty + 4;

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

    thread_data[threadsQty].thisThreadId = threadsQty;
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
        if (thread_data[i].thisThreadId == id) {
            thread_data[i].status = status;
            thread_data[i].steps = steps;
            thread_data[i].x = x;
            thread_data[i].y = y;

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
        printf("\nThread %d:\n", thread_data[i].thisThreadId);
        printf("  Posición Inicial: (%d, %d)\n", thread_data[i].x, thread_data[i].y);
        printf("  Dirección: %d\n", thread_data[i].direction);
        printf("  Pasos: %d\n", thread_data[i].steps);
        printf("  Estado: %d\n", thread_data[i].status);
    }
}


/*----------------------------------------------------
Prints the statistics of the threads
Entries:
    None
Output:
    void
-----------------------------------------------------*/
void printAnStatistic(int id) {
    pthread_mutex_lock(&mutex);

    printf("\033[%d;%dH", controlStats, 1);
    fflush(stdout);
    

    for (int i = 0; i < threadsQty; i++) {
        if(thread_data[i].thisThreadId == id){
            printf("\n*El thread %d ha terminado*\n", thread_data[i].thisThreadId);
            printf("  Posición Inicial: (%d, %d)\n", thread_data[i].x, thread_data[i].y);
            printf("  Dirección: %d\n", thread_data[i].direction);
            printf("  Pasos: %d\n", thread_data[i].steps);
            printf("  Estado: %d\n", thread_data[i].status);
        }
    }
    //controlStats= controlStats + 6;
    //printf("\033[%d;%dH", rowsQty+4, 1);
    //fflush(stdout);
    pthread_mutex_unlock(&mutex);
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
void verifyPath(int x, int y, enum Direction direction) {
    for (int dir = 0; dir < MAX_DIRECTIONS; dir++) {
        int newX = x, newY = y;
        if ((dir == 0 && direction == UP) ||
            (dir == 1 && direction == DOWN) ||
            (dir == 2 && direction == LEFT) ||
            (dir == 3 && direction == RIGHT)) {
            continue;
        }

        switch (dir) {
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

        if (newX >= 0 && newX < colsQty && newY >= 0 && newY < rowsQty &&
            labyrinth[newY][newX].labyrinth == ' ' && labyrinth[newY][newX].directionsQty == 0) {
            createThread(newX, newY, (enum Direction)dir, 0);
        }
    }
}

// Verifica si las coordenadas están dentro del laberinto
int isInsideLabyrinth(int x, int y) {
    return (x >= 0 && x < colsQty && y >= 0 && y < rowsQty);
}

// Verifica si una celda es transitable
int isWalkable(int x, int y) {
    return (labyrinth[y][x].labyrinth != '*' && labyrinth[y][x].labyrinth != '/');
}

// Verifica si una celda es un obstáculo
int isObstacle(int x, int y) {
    return (labyrinth[y][x].labyrinth == '*');
}

// Verifica si una celda es la salida
int isExit(int x, int y) {
    return (labyrinth[y][x].labyrinth == '/');
}

// Actualiza la posición y estado del hilo
void updateThreadPosition(int id, int x, int y, int steps, enum Direction direction, enum threadStatus status) {
    pthread_mutex_lock(&mutex);
    setCursor(x, y, direction);
    labyrinth[y][x].labyrinth = charThread;
    labyrinth[y][x].direction[labyrinth[y][x].directionsQty] = direction;
    labyrinth[y][x].directionsQty++;
    updateThreadInfo(id, x, y, steps, status);
    pthread_mutex_unlock(&mutex);
}

// Actualiza el estado del hilo
void updateThreadStatus(int id, int x, int y, int steps, enum threadStatus status) {
    pthread_mutex_lock(&mutex);
    updateThreadInfo(id, x, y, steps, status);
    pthread_mutex_unlock(&mutex);
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
    int id = data->thisThreadId;
    int x = data->x;
    int y = data->y;
    enum Direction direction = data->direction;
    int steps = data->steps;
    enum threadStatus status = data->status;
    free(data);

    // Sets the the thread in the initial position
    if (isInsideLabyrinth(x, y) && isWalkable(x, y)) {
        steps++;
        updateThreadPosition(id, x, y, steps, direction, status);
        usleep(1000000);
    }

    while (!foundExit)
    {
       int nextX = x, nextY = y;
        verifyPath(x, y, direction);

        switch (direction) {
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

        if (!isInsideLabyrinth(nextX, nextY))
            break;

        if (isObstacle(nextX, nextY)) {
            status = FINISHED_WITHOUT_EXIT;
            updateThreadStatus(id, x, y, steps, status);
            printAnStatistic(id);
            break;
        }

        if (isExit(nextX, nextY)) {
            foundExit = 1;
            status = FINISHED_WITH_EXIT;
            updateThreadStatus(id, x, y, steps, status);
            break;
        }

        x = nextX;
        y = nextY;
        steps++;
        updateThreadPosition(id, x, y, steps, direction, status);
        usleep(500000);
        usleep(1000000);
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
    data->thisThreadId = threadsQty;

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

void printLabyrinthData() {
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            printf("Position [%d][%d]:\n", i, j);
            printf("  Labyrinth: %c\n", labyrinth[i][j].labyrinth);
            printf("  Directions Quantity: %d\n", labyrinth[i][j].directionsQty);
            printf("  Directions:");
            for (int k = 0; k < labyrinth[i][j].directionsQty; k++) {
                printf(" %d", labyrinth[i][j].direction[k]);
            }
            printf("\n");
        }
    }
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