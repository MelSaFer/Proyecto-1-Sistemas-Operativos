#include <stdio.h>
// #include <curses.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

// VARIABLES---------------------------------------------
#define MAX_ROWS 100
#define MAX_COLS 100
#define MAX_DIRECTIONS 4
#define CHAR_THREAD '?'

int rowsQty, colsQty;
int foundExit = 0;
int threadsQty = 0;


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
    if(direction == UP || direction == DOWN){
        x = x + 3;
        y = y + 1;
    } else if(direction == LEFT || direction == RIGHT){
        x = x + 3;
        y = y + 1;
    }
    printf("\033[%d;%dH", y, x);
    fflush(stdout);

    printf("%c", CHAR_THREAD);
    fflush(stdout);

    printf("\033[%d;%dH", rowsQty + 3, 1);
    fflush(stdout);
}

void printLabyrinthRaw() {
    printf("[");
    for (int i = 0; i < rowsQty; i++) {
        if (i > 0) printf(",\n"); // Añade una nueva línea entre filas, excepto antes de la primera
        printf("(");
        for (int j = 0; j < colsQty; j++) {
            printf("(%d, %d, %c)", i, j, labyrinth[i][j].labyrinth);
            if (j < colsQty - 1) printf(", "); // No añade coma después del último elemento
        }
        printf(")");
    }
    printf("]\n");
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
    //sleep(2);

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
    // Verifica si hay posibles caminos alrededor de la celda actual
    for (int dir = 0; dir < MAX_DIRECTIONS; dir++) {
        int newX = x, newY = y;
        switch (dir) {
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
        // Verifica si la posición está dentro de los límites y no es un obstáculo ni una posición visitada
        if (newX >= 0 && newX < colsQty && newY >= 0 && newY < rowsQty &&
            labyrinth[newY][newX].labyrinth == ' ' && labyrinth[newY][newX].directionsQty == 0) {
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
        (labyrinth[y][x].labyrinth != '*' && labyrinth[y][x].labyrinth != '/')) {
        labyrinth[y][x].labyrinth = CHAR_THREAD;  
        //printLabyrinth();
        setCursor(x, y, direction);
        sleep(1);
    }
    

    while (!foundExit) { 
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

        // Verifies if the next step is out of bounds
        if (nextX < 0 || nextX >= colsQty || nextY < 0 || nextY >= rowsQty) {
            break; 
        }

        // Verifies if the next step is an obstacle or the exit
        if (labyrinth[nextY][nextX].labyrinth == '*') {
            break; 
        }

        //Verifies if the next step is the exit
        if (labyrinth[nextY][nextX].labyrinth == '/') {
            foundExit = 1;
            break; // Termina el loop y sale de la función, lo que termina el thread.
        }

        x = nextX;
        y = nextY;

        labyrinth[y][x].labyrinth = CHAR_THREAD;
        labyrinth[y][x].directionsQty++;
        labyrinth[y][x].direction[labyrinth[y][x].directionsQty - 1] = direction;
        //printLabyrinth();
        setCursor(x, y, direction);
        // verifyPath(x, y, direction);

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
void createThread(int x, int y, enum Direction direction, int steps) {
    threadsQty++;
    struct thread_data *data = malloc(sizeof(struct thread_data));
    if (data == NULL) {
        printf("Error\n");
        return;
    }

    data->x = x;
    data->y = y;
    data->direction = direction;
    data->steps = steps;

    pthread_t thread;
    if (pthread_create(&thread, NULL, moveThread, (void *)data) != 0) {
        printf("Error al crear el thread\n");
        free(data);
        return;
    }

    // Opcionalmente, puedes hacer join o detach del hilo aquí
    // pthread_join(thread, NULL);
    pthread_detach(thread);

    //printf("Thread creado\n");
}



// MAIN-----------------------------------------------
int main()
{
    printf("Iniciando programa\n");
    readLabyrinth("inputs/lab2.txt");
    //printLabyrinthRaw();
    printLabyrinth();
    createThread(0, 0, DOWN, 0);
    // createThread(0, 0, RIGHT, 0);
    while(!foundExit){
        // sleep(1);
    }
    
    printf("Salida encontrada. Terminando programa.\n");
    printf("Threads creados: %d\n", threadsQty);
    return 0; // Sale del programa.
}