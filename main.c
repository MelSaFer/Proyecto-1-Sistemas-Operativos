#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>

// VARIABLES---------------------------------------------
#define MAX_ROWS 100
#define MAX_COLS 100
#define MAX_DIRECTIONS 4

char charThread = '?';
int rowsQty, colsQty;
int foundExit = 0;
int threadsQty = 0;
int controlStats = 0;
int finishedOrder = 1;
// Define the mutex
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

// ENUMS----------------------------------------------------------------------
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

// STRUCTS---------------------------------------------------------------------
struct thread_data
{
    int thisThreadId;
    int x;
    int y;
    enum Direction direction;
    int steps;
    enum threadStatus status;
    int order;
};

struct thread_data *thread_data = NULL;

typedef struct
{
    char labyrinth;
    enum Direction direction[MAX_DIRECTIONS];
    int directionsQty;
} labyrinth_data;

labyrinth_data labyrinth[MAX_ROWS][MAX_COLS];

// FUNCTIONS-----------------------------------------------------------------------

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

// -> CONSOLE RELATED FUNCTIONS-----------------------

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
bool readLabyrinth(char *fileName)
{
    FILE *file;
    file = fopen(fileName, "r");


    if (file == NULL)
    {
        printf("El archivo no existe\n");
        return false;
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
    return true;
}

/*----------------------------------------------------
Prints the status name of threads
Entries:
   state: state of the thread
Output:
    char*: name of the state
-----------------------------------------------------*/
const char* getStatusName(enum threadStatus status) {
    switch (status) {
        case RUNNING:
            return "RUNNING";
        case FINISHED_WITHOUT_EXIT:
            return "FINISHED_WITHOUT_EXIT";
        case FINISHED_WITH_EXIT:
            return "FINISHED_WITH_EXIT";
        default:
            return "UNKNOWN";
    }
}

/*----------------------------------------------------
Prints the direction name of threads
Entries:
   direction: direction of the thread
Output:
    char*: name of the direction
-----------------------------------------------------*/
const char* getDirectionName(enum Direction direction) {
    switch (direction) {
        case UP:
            return "UP";
        case DOWN:
            return "DOWN";
        case LEFT:
            return "LEFT";
        case RIGHT:
            return "RIGHT";
        default:
            return "UNKNOWN";
    }
}


// -> STATISTICS RELATED FUNCTIONS--------------------

/*----------------------------------------------------
Prints the statistics of the threads
Entries:
    None
Output:
    void
-----------------------------------------------------*/
void printStatistics() {

    printf("\033[%d;%dH", controlStats, 1);
    fflush(stdout);

    printf("Cantidad de threads creados: %d\n", threadsQty);
    printf("Información de los threads:\n");

    printf("+-----+----+----+-----------+-------+------------------------+-------+\n");
    printf("| id  | x  | y  | direccion | pasos | estado                 | orden |\n");
    printf("+-----+----+----+-----------+-------+------------------------+-------+\n");

    for (int i = 0; i < threadsQty; i++) {
        printf("| %-3d | %-2d | %-2d | %-9s | %-5d | %-22s | %-5d |\n", 
            thread_data[i].thisThreadId,
            thread_data[i].x, 
            thread_data[i].y, 
            getDirectionName(thread_data[i].direction), 
            thread_data[i].steps, 
            getStatusName(thread_data[i].status),
            thread_data[i].order);
        
        printf("+-----+----+----+-----------+-------+------------------------+-------+\n");
    }
}


/*----------------------------------------------------
Prints the statistics of an specific thread
Entries:
    id: thread id
Output:
    void
-----------------------------------------------------*/
void printAnStatistic(int id) {
    pthread_mutex_lock(&mutex);

    printf("\033[%d;%dH", controlStats, 1);
    fflush(stdout);
    

    for (int i = 0; i < threadsQty; i++) {
        if (thread_data[i].thisThreadId == id) {
            printf("\n*El thread %d ha terminado*\n", thread_data[i].thisThreadId);
            
            
            printf("+----+----+-----------+-------+-----------------------+\n");
            printf("| x  | y  | direccion | pasos | estado                |\n");
            printf("+----+----+-----------+-------+-----------------------+\n");
            
            
            printf("| %-2d | %-2d | %-9s | %-5d | %-20s |\n", 
                thread_data[i].x, 
                thread_data[i].y, 
                getDirectionName(thread_data[i].direction), 
                thread_data[i].steps, 
                getStatusName(thread_data[i].status));
            
            
            printf("+----+----+-----------+-------+-----------------------+\n");
        }
    }
    //controlStats= controlStats + 6;
    //printf("\033[%d;%dH", rowsQty+4, 1);
    //fflush(stdout);
    pthread_mutex_unlock(&mutex);
}


// -> LABYRINTH VERIFICATION FUNCTIONS----------------
/*----------------------------------------------------
Verifies if a cell is inside the labyrinth
Entries:
    x: x position
    y: y position
Output:
    1 if the cell is inside the labyrinth, 0 otherwise
-----------------------------------------------------*/
int isInsideLabyrinth(int x, int y) {
    return (x >= 0 && x < colsQty && y >= 0 && y < rowsQty);
}

/*----------------------------------------------------
Verifies if a cell is walkable
Entries:
    x: x position
    y: y position
Output:
    1: if the cell is walkable
    0: otherwise
-----------------------------------------------------*/
int isWalkable(int x, int y) {
    return (labyrinth[y][x].labyrinth != '*' && labyrinth[y][x].labyrinth != '/');
}

/*----------------------------------------------------
Verifies if a cell is an obstacle
Entries:
    x: x position
    y: y position
Output:
    1: if the cell is an obstacle
    0: otherwise
-----------------------------------------------------*/
int isObstacle(int x, int y) {
    return (labyrinth[y][x].labyrinth == '*');
}

/*----------------------------------------------------
Verifies if a cell is the exit
Entries:
    x: x position
    y: y position
Output:
    1: if the cell is the exit
    0: otherwise
-----------------------------------------------------*/
int isExit(int x, int y) {
    return (labyrinth[y][x].labyrinth == '/');
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

// -> THREAD RELATED FUNCTIONS-----------------------

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

void setOrder(int id) {
    for (int i = 0; i < threadsQty; i++) {
        if (thread_data[i].thisThreadId == id) {
            thread_data[i].order = finishedOrder;
            finishedOrder++;
            break;
        }
    }
}

/*----------------------------------------------------
Updates the position of the thread
Entries:
    id: thread id
    x: x position
    y: y position
    steps: number of steps
    direction: direction of the thread
    status: status of the thread
Output:
    void
-----------------------------------------------------*/
void updateThreadPosition(int id, int x, int y, int steps, enum Direction direction, enum threadStatus status) {
    pthread_mutex_lock(&mutex);
    setCursor(x, y, direction);
    labyrinth[y][x].labyrinth = charThread;
    labyrinth[y][x].direction[labyrinth[y][x].directionsQty] = direction;
    labyrinth[y][x].directionsQty++;
    updateThreadInfo(id, x, y, steps, status);
    pthread_mutex_unlock(&mutex);
}

/*----------------------------------------------------
Updates the status of the thread
Entries:
    id: thread id
    x: x position
    y: y position
    steps: number of steps
    status: status of the thread
Output:
    void
-----------------------------------------------------*/
void updateThreadStatus(int id, int x, int y, int steps, enum threadStatus status) {
    pthread_mutex_lock(&mutex);
    updateThreadInfo(id, x, y, steps, status);
    pthread_mutex_unlock(&mutex);
}

/*----------------------------------------------------
Sets the thread as finished without exit
Entries:
    id: thread id
    x: x position
    y: y position
    steps: number of steps
    status: status of the thread
Output:
    void
-----------------------------------------------------*/
void setFinished(int id, int x, int y, int steps, enum threadStatus status){
    status = FINISHED_WITHOUT_EXIT;
    updateThreadStatus(id, x, y, steps, status);
    setOrder(id);
    printAnStatistic(id);
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

        if (!isInsideLabyrinth(nextX, nextY)){
            setFinished(id, x, y, steps, status);
            break;
        }

        if (isObstacle(nextX, nextY)) {
            setFinished(id, x, y, steps, status);
            break;
        }

        if (isExit(nextX, nextY)) {
            setOrder(id);
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
    data->order = 0;

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


// MAIN FUNCTION--------------------------------------------------------------
int main()
{
    

    printf("Iniciando programa...\n");
    char fileName[100];
    char filePath[150] = "inputs/"; 
    printf("Por favor, ingresa el nombre del archivo txt: ");
    scanf("%99s", fileName);
    strcat(filePath, fileName);

    if((readLabyrinth(filePath)==false)){
        return 0;
    }

    
    readLabyrinth(filePath);
    printLabyrinth();
    controlStats = rowsQty + 6;

    createThread(0, 0, DOWN, 0);
    while (!foundExit)
    {

    }

    printf("Salida encontrada. Terminando programa.\n");
    sleep(2);

    for (int i = 0; i < threadsQty; i++) {
        if(thread_data[i].status == RUNNING){
            updateThreadStatus(thread_data[i].thisThreadId, thread_data[i].x, thread_data[i].y, thread_data[i].steps, FINISHED_WITHOUT_EXIT);
            printAnStatistic(thread_data[i].thisThreadId);
        }
    }
    printStatistics();

    free(thread_data);

    return 0;
}