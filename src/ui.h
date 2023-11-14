#ifndef UI_U_
#define UI_U_ 

typedef struct _ui_state *UiState;
UiState ui_init();
void ui_destroy(UiState);

typedef struct _ui_cursor *UiCursor;

// Update
int ui_update(UiState, int);

// Draw
void ui_draw(UiState);

// Actions
void ui_left(UiState);
void ui_right(UiState);
void ui_up(UiState);
void ui_down(UiState);
void ui_dec_current_col(UiState);
void ui_inc_current_col(UiState);

void ui_open_editor(UiState);

#endif
