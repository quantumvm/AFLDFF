#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "../access_methods/afldff_access.h"

//these values are wrong but I will figure out how to get the
//terminal length later.

static size_t terminal_x = 80;
static size_t terminal_y = 24;

void draw_afldff_interface(){
    WINDOW * win;
    
    initscr();
    noecho();
    curs_set(0);

    win = newwin(terminal_y, terminal_x/2-2, 0, terminal_x/2 +1);
    
    while(1){
        wclear(win);
        box(win,0,0);
        mvwprintw(win,1,1,"Crash cases: %ld", get_all_crash_cases());
        mvwprintw(win,2,1,"Test cases: %ld", get_all_test_cases());
        wrefresh(win);
        sleep(2);
    }
    
    endwin();


}
