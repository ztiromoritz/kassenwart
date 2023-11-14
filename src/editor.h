#ifndef EDITOR_U_
#define EDITOR_U_

typedef struct _editor_state *EditorState;
EditorState editor_init();
void editor_destroy(EditorState);


int editor_update(EditorState, int);
// Draw
void editor_draw(EditorState);

#endif
