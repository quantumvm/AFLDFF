#include <ncurses.h>
#include <menu.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <pthread.h>

#include "../access_methods/afldff_access.h"

//Just assume the user has a normal terminal. This needs
//to be adjusted later.

static size_t terminal_x = 80;
static size_t terminal_y = 24;

//Our state machine kept in the data segment
enum afldff_state {MAIN_MENU, TEST_STATE};
enum afldff_state global_state = MAIN_MENU;


static void main_menu_right(WINDOW * window){

    werase(window);
    box(window,0,0);

    wattron(window, COLOR_PAIR(1));
    mvwprintw(window,1,1,"Tests:   %ld", get_all_test_cases());
    wattroff(window, COLOR_PAIR(1));
    
    wattron(window, COLOR_PAIR(2));
    mvwprintw(window,2,1,"Crashes: %ld", get_all_crash_cases());
    wattroff(window, COLOR_PAIR(2));
    
    wrefresh(window);

}


//Update left window logic. If enter key was not pressed
//return NULL. Otherwise return pointer to selected item.
void stuff(WINDOW * window, MENU * menu){
    int c = wgetch(window);
    switch(c){
        case KEY_DOWN:
            menu_driver(menu, REQ_DOWN_ITEM);
            wrefresh(window);
            break;
        case KEY_UP:
            menu_driver(menu, REQ_UP_ITEM);
            wrefresh(window);
            break;
    }
    
}


char * choices[]={
    "Choice_1",
    "Choice_2",
    "Choice_3",
    "Exit"
};


static void main_menu(){
	MENU *my_menu;
        WINDOW *my_menu_win, *right_win;
        int n_options;
        
        init_pair(1, COLOR_YELLOW, 0); 
        init_pair(2, COLOR_RED, 0);


	/* Create items */
        n_options = sizeof(choices)/sizeof(ITEM *);
        ITEM ** my_items = (ITEM **)calloc(n_options+1, sizeof(ITEM *));
        
        for(int i = 0; i < n_options; i++){
                my_items[i] = new_item(choices[i], "");
        }
	
        /* Crate menu */
	my_menu = new_menu(my_items);

	/* Create the window to be associated with the menu */
        right_win = newwin(12, 37, 0, 41);
        my_menu_win = newwin(12, 40, 0, 0);
        keypad(my_menu_win, TRUE);
        nodelay(my_menu_win, TRUE);
     
	/* Set main window and sub window */
        set_menu_win(my_menu, my_menu_win);
        set_menu_sub(my_menu, derwin(my_menu_win, 6, 38, 3, 1));

	/* Set menu mark to the string " * " */
        set_menu_mark(my_menu, " * ");

	/* Print a border around the main window and print a title */
        box(my_menu_win, 0, 0);
	mvwaddch(my_menu_win, 2, 0, ACS_LTEE);
	mvwhline(my_menu_win, 2, 1, ACS_HLINE, 38);
	mvwaddch(my_menu_win, 2, 39, ACS_RTEE);
	mvprintw(LINES - 2, 0, "F1 to exit");
//	refresh();
        
	/* Post the menu */
	post_menu(my_menu);
	wrefresh(my_menu_win);


        while(1){
            stuff(my_menu_win, my_menu); 
            main_menu_right(right_win);
            usleep(100000);
        }


	/* Unpost and free all the memory taken up */
        unpost_menu(my_menu);
        free_menu(my_menu);
        for(int i = 0; i < n_options; ++i)
                free_item(my_items[i]);
	endwin();
}


static void test_state(){
    printw("This is only a test");
    refresh();
    getchar();
}

void draw_afldff_interface(){
    initscr();
    start_color();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);   
    curs_set(0);
    
    global_state = MAIN_MENU;

    while(1){
        switch(global_state){
            case MAIN_MENU: main_menu(); break;
            case TEST_STATE: test_state(); break;

        }
    }
    printw("I'm sorely grieved I may not steera");
    exit(0);

}
