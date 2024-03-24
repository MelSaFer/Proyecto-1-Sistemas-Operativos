#include <stdio.h>
//#include <curses.h>
#include <pthread.h> 


//STRUCTS---------------------------------------------
struct Labyrinth{
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
   struct Labyrinth Wall;
   Wall.wall = '#';
   Wall.x = 10;
   Wall.y = 10;
}

/*----------------------------------------------------
reads the file and prints it, also creates the labyrinth
initializing the wall struct
Entries:
   fileName: name of the file to read
Output:
   void
-----------------------------------------------------*/
void initlabyrinth(char *fileName) {
    FILE *file;
    file = fopen(fileName, "r");
    struct Labyrinth labyrinth;
    labyrinth.x = 0; 
    labyrinth.y = 0;

    if (file == NULL) {
        printf("Error opening file\n");
        return;
    }

    char c;

    while ((c = fgetc(file)) != EOF) {
        printf("%c", c);
        labyrinth.wall = c;

        if (c == '\n') { 
            labyrinth.y++;
            labyrinth.x = 0;
        } else {
            labyrinth.x++; // Si no, incrementa x
        }
    }

    printf("\n");
    printf("Wall: %c\n", labyrinth.wall);
    printf("X: %d\n", labyrinth.x);
    printf("Y: %d\n", labyrinth.y);
	printf("\n");
    fclose(file);
}



//MAIN-----------------------------------------------
int main() {
   initlabyrinth("labyrinth.txt");
   return 0;
}