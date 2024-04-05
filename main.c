#include <stdio.h>
// #include <curses.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

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

// STRUCTS---------------------------------------------
struct thread_data
{
    int x;
    int y;
    enum Direction direction;
    int steps;
};

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
    for (int dir = 0; dir < MAX_DIRECTIONS; dir++)
    {
        int newX = x, newY = y;
        switch (dir)
        {
        case UP:
            newY--;
            break;
        case DOWN:
            newY++;
            break;
        case LEFT:
            newX--;
            break;
        case RIGHT:
            newX++;
            break;
        }

        // for(int i = 0; i < labyrinth[y][x].directionsQty; i++){
        //     if(labyrinth[y][x].direction[i] == (enum Direction)dir){
        //         return;
        //     }
        // }
        
        if (newX >= 0 && newX < colsQty && newY >= 0 && newY < rowsQty &&
            labyrinth[newY][newX].labyrinth == ' ' && labyrinth[newY][newX].directionsQty == 0)
        {
            createThread(newX, newY, (enum Direction)dir, 0);
        }
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
    int x = data->x;
    int y = data->y;
    enum Direction direction = data->direction;
    int steps = data->steps;
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
            break;
        }

        // Verifies if the next step is the exit
        if (labyrinth[nextY][nextX].labyrinth == '/')
        {
            foundExit = 1;
            break;
        }

        x = nextX;
        y = nextY;
        pthread_mutex_lock(&mutex);
        labyrinth[y][x].labyrinth = charThread;
        labyrinth[y][x].directionsQty++;
        labyrinth[y][x].direction[labyrinth[y][x].directionsQty - 1] = direction;
        
        //pthread_mutex_lock(&mutex);
        setCursor(x, y, direction);
        pthread_mutex_unlock(&mutex);

        steps++;
        usleep(500000);
        sleep(3);
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
    threadsQty++;
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

    pthread_t thread;
    if (pthread_create(&thread, NULL, moveThread, (void *)data) != 0)
    {
        printf("Error al crear el thread\n");
        free(data);
        return;
    }

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
    printf("Cantidad de threads creados: %d\n", threadsQty);

    return 0;
}