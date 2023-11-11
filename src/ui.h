#include <ncurses.h>
#ifndef UI_U_   
#define UI_U_


typedef struct {
   int cell_x;
   int cell_y;
} UiState;

void ui_init(WINDOW *);
void ui_draw_col_head(WINDOW *,UiState *);
void ui_draw_row_head(WINDOW *,UiState *);
void ui_draw_status_line(WINDOW *,UiState *);
void ui_draw_cursor(WINDOW *, UiState *);


#endif 

