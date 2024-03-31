#include <stdio.h>
// #include <curses.h>
#include <pthread.h>

// VARIABLES---------------------------------------------
#define MAX_ROWS 100
#define MAX_COLS 100
int rowsQty, colsQty;
char labyrinth[MAX_ROWS][MAX_COLS];

// STRUCTS---------------------------------------------
struct Labyrinth
{
   char wall;
   int x;
   int y;
   // int **m;
};

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
Creates a labyrinth
Entries:
   None
Output:
   void
-----------------------------------------------------*/
void createlabyrinth()
{
   struct Labyrinth Wall;
   Wall.wall = '#';
   Wall.x = 10;
   Wall.y = 10;
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
         printf(" %c |", labyrinth[i][j]);
      }
      printf("\n|");
      for (int j = 0; j < colsQty; j++)
      {
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
         labyrinth[row][column] = c;
         column++;
      }
   }

   fclose(file);
}

// MAIN-----------------------------------------------
int main()
{
   readLabyrinth("inputs/lab1.txt");
   printLabyrinth();
   return 0;
}