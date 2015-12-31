#include <ncurses.h>
#include <menu.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <time.h>

#include <pthread.h>
#include <glib.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "../networking/afldff_access.h"

#include "../networking/afldff_networking.h"


extern GQueue * NEW_NODE_QUEUE;
extern pthread_mutex_t q_mutex;
extern pthread_mutex_t ll_mutex;


//Just assume the user has a normal terminal. This needs
//to be adjusted later.

static size_t terminal_x = 80;
static size_t terminal_y = 24;

//Our state machine kept in the data segment
enum afldff_state {MAIN_MENU, WOOPS};
enum afldff_state global_state = MAIN_MENU;

static char * hash_to_string(char * hash){
    char * hash_string = malloc(32+1);
    for(int i=0; i<16;i++){
        sprintf(&hash_string[i*2], "%02x", hash[i]);
    }

    hash_string[32] = '\x00';
    return hash_string;
}

static void main_menu_bottom(WINDOW * window){
    pthread_mutex_lock(&ll_mutex);
    pthread_mutex_lock(&q_mutex);
    if(!g_queue_is_empty(NEW_NODE_QUEUE)){
        packet_info * pi = g_queue_pop_head(NEW_NODE_QUEUE);
        
        //
        char * hash = hash_to_string(pi->p->hash);
        
        struct sockaddr_in * sockin = (struct sockaddr_in *) &pi->src_addr;
        char * source_ip = inet_ntoa(sockin->sin_addr);
        
        struct tm * time_info;
        time_info = localtime(&pi->time_joined);

        wprintw(window,"New node %d has joined from %s at %s", 
                pi->p->instance_id, 
                source_ip, 
                asctime(time_info));
        
        free(hash);
    }
    pthread_mutex_unlock(&q_mutex);
    pthread_mutex_unlock(&ll_mutex);
    wrefresh(window);

}

static void main_menu_right(WINDOW * window){

    werase(window);
    box(window,0,0);
    
    wattron(window, COLOR_PAIR(1));
    mvwprintw(window,1,1,"Name: ALL");
    wattroff(window, COLOR_PAIR(1));
   
    wattron(window, COLOR_PAIR(2));
    mvwprintw(window,2,1,"Tests:   %ld", get_all_test_cases());
    wattroff(window, COLOR_PAIR(2));
    
    wattron(window, COLOR_PAIR(3));
    mvwprintw(window,3,1,"Crashes: %ld", get_all_crash_cases());
    wattroff(window, COLOR_PAIR(3));
    
   
    wrefresh(window);

}


//Update left window logic. If enter key was not pressed
//return NULL. Otherwise return pointer to selected item.
ITEM * main_menu_left(WINDOW * window, MENU * menu){
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
        case 10:
            return current_item(menu);
            break;
    }

    return NULL;
    
}


static char * left_main_menu_options[]={
    "View jobs",
    "Collect crashes",
    "Apply patch",
    "Exit"
};

static void main_menu(){
    MENU *my_menu;
    WINDOW *my_menu_win, *right_win, *bottom_win_box, *bottom_win;
    int n_options;
    
    //initialize color schemes used by update window
    init_pair(1, COLOR_BLUE, 0);
    init_pair(2, COLOR_YELLOW, 0); 
    init_pair(3, COLOR_RED, 0);


    
    //Initialize an array of items to put in menu
    n_options = sizeof(left_main_menu_options)/sizeof(ITEM *);
    ITEM ** my_items = (ITEM **)calloc(n_options+1, sizeof(ITEM *)); 
    for(int i = 0; i < n_options; i++){
            my_items[i] = new_item(left_main_menu_options[i], "");
    }
    
    //initialize menu
    my_menu = new_menu(my_items);
    
    //create the left, right, and bottom windows for main menu.
    my_menu_win = newwin(terminal_y/2, (terminal_x/2), 0, 0);
    right_win = newwin(terminal_y/2, (terminal_x/2), 0, terminal_x/2);
    bottom_win_box = newwin(terminal_y/2, terminal_x, terminal_y/2, 0);
    bottom_win = newwin(terminal_y/2-2, terminal_x-2, terminal_y/2+1,1);
    
    keypad(my_menu_win, TRUE);
    nodelay(my_menu_win, TRUE);
 
    // main/sub window
    set_menu_win(my_menu, my_menu_win);
    set_menu_sub(my_menu, derwin(my_menu_win, 6, (terminal_x/2)-2, 3, 1));
    
    //Set title of left menu and selection character
    char program_title[] = "Welcome to AFLDFF!";
    mvwprintw(my_menu_win, 1, (terminal_x/4)-(strlen(program_title)/2), program_title);
    set_menu_mark(my_menu, " * ");
    
    //box seperator for left menu
    box(my_menu_win, 0, 0);
    mvwaddch(my_menu_win, 2, 0, ACS_LTEE);
    mvwhline(my_menu_win, 2, 1, ACS_HLINE, (terminal_x/2)-2);
    mvwaddch(my_menu_win, 2, (terminal_x/2)-1, ACS_RTEE);

    //menu post
    post_menu(my_menu);
    wrefresh(my_menu_win);
    
    box(bottom_win_box, 0, 0);
    wrefresh(bottom_win_box);

    while(1){
        main_menu_right(right_win);
        main_menu_bottom(bottom_win);
        ITEM * choice = main_menu_left(my_menu_win, my_menu); 

        /************************************************
         *Switch used to control Main menu options      *
         ************************************************/
                
        if(choice != NULL){
            char * menu_selection = (char *) item_name(choice);
            if(strcmp(menu_selection, left_main_menu_options[0]) == 0){ 
                global_state = WOOPS;
                return;
            }
            else if(strcmp(menu_selection, left_main_menu_options[1]) == 0){
                global_state = WOOPS;
                return;
            }
            else if(strcmp(menu_selection, left_main_menu_options[2]) == 0){
                global_state = WOOPS;
                return;
            }               
            else if(strcmp(menu_selection, left_main_menu_options[3]) == 0){
                endwin();
                exit(0);
            }
            else{
                global_state = WOOPS;
                return;
            }
        
        }
                
        usleep(10000);
    }


    /* Unpost and free all the memory taken up */
    unpost_menu(my_menu);
    free_menu(my_menu);
    for(int i = 0; i < n_options; ++i){
            free_item(my_items[i]);
    }
    delwin(my_menu_win);
    delwin(right_win);

}


static void woops(){
    char error_message[] = "Woops! your not suppose to be here!";
    
    clear();
    mvprintw(terminal_y/2, (terminal_x/2)-(strlen(error_message)/2), error_message);
    refresh();
    getchar();
    endwin();
    exit(1);
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
            case WOOPS: woops(); break;

        }
    }
    printw("I'm sorely grieved I may not steera");
    exit(0);

}
