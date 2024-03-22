#include <stdio.h>
//#include <curses.h>


//STRUCTS---------------------------------------------
struct labyrinth{
      char wall;
      int x;
      int y;
      //int **m;
   };

//FUNCTIONS-------------------------------------------

/*----------------------------------------------------
Set cursor position
Entries:
   x: x position
   y: y position	
Output:
   void
-----------------------------------------------------*/
void setCursor(int x, int y){
   printf("\033[%d;%df", x, y);
}


/*----------------------------------------------------
Creates a labyrinth
Entries:
   None
Output:
   void
-----------------------------------------------------*/
void createlabyrinth(){
   struct labyrinth wall;
   wall.wall = '#';
   wall.x = 10;
   wall.y = 10;
}

/*----------------------------------------------------
reads the file and prints it, also creates the labyrinth
initializing the wall struct
Entries:
   fileName: name of the file to read
Output:
   void
-----------------------------------------------------*/

void initlabyrinth(char *fileName){
   FILE *file;
   file = fopen(fileName, "r");
   if(file == NULL){
      printf("Error opening file\n");
      return;
   }
   char c;
   while((c = fgetc(file)) != EOF){
      printf("%c", c);
   }
   printf("\n");
   fclose(file);
}


//MAIN-----------------------------------------------
int main() {
   //printf("\033[2J");
   //printf("\033[%d;%dH", 1000, 100);printf("Hello Mel!\n");
   //setCursor(100, 50);printf("Hello Mel!\n");
   initlabyrinth("labyrinth.txt");
   return 0;
}