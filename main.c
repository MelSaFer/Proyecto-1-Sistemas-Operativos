#include <stdio.h>
// #include <curses.h>
#include <pthread.h>

// VARIABLES---------------------------------------------
#define MAX_ROWS 100
#define MAX_COLS 100
#define MAX_DIRECTIONS 4
int rowsQty, colsQty;


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
   int x,y;
   enum Direction direction;
   int steps;
};

typedef struct {
    char labyrinth;
    enum Direction direction[MAX_DIRECTIONS];
    int directionsQty;
} labyrinth_data;

labyrinth_data labyrinth[MAX_ROWS][MAX_COLS];

// FUNCTIONS-------------------------------------------

/*----------------------------------------------------
Set cursor position
Entries:
   x: x position
   y: y position
Output:
   void
-----------------------------------------------------*/
void setCursor(int x, int y)
{
   // printf("\033[%d;%df", x, y);

   printf("\033[%d;%dH", y, x);
   fflush(stdout);

   printf("%c", '?');
   fflush(stdout);

   printf("\033[%d;%dH", y + 1, 1);
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
   printSeparator();

   // prints each row of the labyrinth
   printf("|");
   for (int i = 1; i < rowsQty + 1; i++)
   {
      //   printf("|");
      for (int j = 0; j < colsQty; j++)
      {
         printf(" %c |", labyrinth[i][j].labyrinth);
      }

      if( i < rowsQty){
         printf("\n|");
      } else{
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
Creates a new thread
Entries:
   None
Output:  
   void
-----------------------------------------------------*/
void createThread()
{
   // pthread_t thread;
   // struct thread_data data;
   // pthread_create(&thread, NULL, moveThread, (void *)&data);
}


/*----------------------------------------------------
Moves the thread in the labyrinth
Entries:
   None 
Output:
   void
-----------------------------------------------------*/
void moveThread(struct thread_data *data)
{
   

}

// MAIN-----------------------------------------------
int main()
{
   readLabyrinth("inputs/lab1.txt");
   printLabyrinth();
   return 0;
}