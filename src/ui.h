#include <ncurses.h>
#ifndef UI_U_   /* Include guard */
#define UI_U_


typedef struct {
   int cell_x;
   int cell_y;
} UiState;

int foo(int x); 

void drawHeader(WINDOW *,UiState *);
void drawRowHeader(WINDOW *,UiState *);
void drawStatusLine(WINDOW *,UiState *);
void drawCursor(WINDOW *, UiState *);

void uiInit();


#endif 

