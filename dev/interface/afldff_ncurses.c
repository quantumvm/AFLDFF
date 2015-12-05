#include <panel.h>

//these values are wrong but I will figure out how to get the
//terminal length later.

static size_t terminal_x = 80;
static size_t terminal_y = 24;

void draw_afldff_interface(){
    WINDOW * win;
    PANEL  * panel;
    
    initscr();
    cbreak();
    noecho();

    win = newwin(terminal_y, terminal_x/2-2, 0, terminal_x/2 +1);
    box(win,0,0);
    panel = new_panel(win);

    update_panels();
    //draw all the things
    doupdate();
    

    getch();
    endwin();

}
