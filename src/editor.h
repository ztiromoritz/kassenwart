#ifndef EDITOR_H_
#define EDITOR_H_

typedef struct _editor_state *EditorState;
EditorState editor_init();
void editor_destroy(EditorState);


int editor_update(EditorState, int);
// Draw
void editor_draw(EditorState);

#endif
