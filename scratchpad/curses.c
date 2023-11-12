
void cursesTest() {
  initscr();
  raw();                /* Line buffering disabled	*/
  keypad(stdscr, TRUE); /* We get F1, F2 etc..		*/
  noecho();             /* Don't echo() while we do getch */
  refresh();

  WINDOW *win = newwin(0, 0, 0, 0);

  // making box border with default border styles
  box(win, 0, 0);

  // move and print in window
  mvwprintw(win, 0, 1, "Greeter");
  mvwprintw(win, 1, 1, "Hello");

  // refreshing the window
  wrefresh(win); // making box border with default border styles
  box(win, 0, 0);

  // move and print in window
  mvwprintw(win, 0, 1, "Greeter");
  mvwprintw(win, 1, 1, "Hello");

  // refreshing the window
  wrefresh(win);
  refresh();
  getch();
  endwin();
}

