#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <pthread.h>

#include "../access_methods/afldff_access.h"

//these values are wrong but I will figure out how to get the
//terminal length later.

static size_t terminal_x = 80;
static size_t terminal_y = 24;

enum {MAIN_MENU, RANDOM_WINDOW} afldff_state;

void * draw_update_window(void *ptr){
    /*********************************
     * HANDLE UPDATE WIN STUFF       *
     *********************************/
    int * is_alive = ptr;

    WINDOW * status_win;
    
    //set up colors
    init_pair(1, COLOR_YELLOW, 0); 
    init_pair(2, COLOR_RED, 0); 

    status_win = newwin(terminal_y, terminal_x/2-2, 0, terminal_x/2 +1);

    while(*is_alive){
        wclear(status_win);
        box(status_win,0,0);

        wattron(status_win, COLOR_PAIR(1));
        mvwprintw(status_win,1,1,"Tests:   %ld", get_all_test_cases());
        wattroff(status_win, COLOR_PAIR(1));
        
        wattron(status_win, COLOR_PAIR(2));
        mvwprintw(status_win,2,1,"Crashes: %ld", get_all_crash_cases());
        wattroff(status_win, COLOR_PAIR(2));
        
        wrefresh(status_win);
        sleep(2);
    }
    
    delwin(status_win);
    pthread_exit(0);
}

static void draw_main_menu(){
 
    WINDOW * menu_win;

    /********************************
     *HANDLE MENU WINDOW            *
     ********************************/
    char welcome_message[] = "Welcome to AFLDFF!";
    
    //used to center message
    int title_x = terminal_x/4 - (strlen(welcome_message)/2) - 1;
    
    menu_win = newwin(terminal_y, terminal_x/2,0,1);
    mvwprintw(menu_win, 1, title_x, welcome_message);
    wrefresh(menu_win);

    getchar();
    delwin(menu_win);


}


static void draw_main(){
    //this will be a shared flag that will tell the update window to 
    //destroy itself
    int * main_menu_alive = malloc(sizeof(int));
    *main_menu_alive = 1;

    pthread_t update_window;
    pthread_create(&update_window, NULL, draw_update_window, main_menu_alive);
    draw_main_menu();
    endwin();
    
    free(main_menu_alive);

}


void draw_afldff_interface(){
    initscr();
    noecho();
    start_color();
    curs_set(0);

    switch(afldff_state){
        case MAIN_MENU: draw_main(); return;
        case RANDOM_WINDOW: return;

    }
    
    exit(0);

}
