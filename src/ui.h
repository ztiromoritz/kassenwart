#ifndef UI_U_
#define UI_U_

typedef struct _ui_state *UiState;
UiState ui_init();
void ui_destroy(UiState);

typedef struct _ui_cursor *UiCursor;

// Draw
void ui_draw_col_head(UiState);
void ui_draw_row_head(UiState);
void ui_draw_status_line(UiState);
void ui_draw_cells(UiState);
void ui_draw_cursor(UiState);

// Actions
void ui_left(UiState);
void ui_right(UiState);
void ui_up(UiState);
void ui_down(UiState);
void ui_dec_current_col(UiState);
void ui_inc_current_col(UiState);

void ui_open_editor(UiState);

#endif
