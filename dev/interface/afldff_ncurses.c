#define _GNU_SOURCE

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
#include <libssh/libssh.h>

#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "../networking/afldff_access.h"
#include "../networking/afldff_networking.h"
#include "../../afl-dff.h"

#define LEFT 0
#define RIGHT 1
#define FALSE 0
#define TRUE 1


extern GQueue * NEW_NODE_QUEUE;
extern GSList * GLOBAL_JOB_MATRIX;
extern pthread_mutex_t q_mutex;
extern pthread_mutex_t ll_mutex;


//Just assume the user has a normal terminal. This needs
//to be adjusted later.

static size_t terminal_x = 80;
static size_t terminal_y = 24;

//Our state machine kept in the data segment
enum afldff_state {MAIN_MENU, VIEW_JOBS, APPLY_PATCH, STOP_JOBS, COLLECT_CRASHES, WOOPS};
enum afldff_state global_state = MAIN_MENU;



/*
 * @global_job_list_iterator function for performing an action ($action) for
 * each element in the global job list. It also provides a thread safe 
 * context to do so.
 *
 */
static void global_job_list_iterator(void (*action)(packet_info * pi, job_node * job, void * data), void * data){

    pthread_mutex_lock(&ll_mutex);
        
        //iterate through 
        for(GSList * gslp_job = GLOBAL_JOB_MATRIX; 
            gslp_job; 
            gslp_job = gslp_job->next){
            
            job_node * job = gslp_job->data;
            for(GSList * gslp_node = job->packet_info_list; 
                gslp_node; 
                gslp_node = gslp_node->next){
            
                action(gslp_node->data, gslp_job->data, data);
            }
        }
        
    pthread_mutex_unlock(&ll_mutex);
}



/************************************************
 *                                              *
 *                  MAIN MENU                   *
 *                                              *
 * **********************************************/

static void main_menu_bottom(WINDOW * window){
    werase(window);
    pthread_mutex_lock(&q_mutex);
    for(int i = 0; i<g_queue_get_length(NEW_NODE_QUEUE); i++){
        char * message = g_queue_peek_nth(NEW_NODE_QUEUE, i);
        wprintw(window,"%s", message);
    }
    pthread_mutex_unlock(&q_mutex);
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
static ITEM * main_menu_left(WINDOW * window, MENU * menu){
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
    "Deploy jobs",
    "Apply patch",
    "Exit"
};

static void main_menu(){
    MENU *my_menu;
    WINDOW *left_win, *right_win, *bottom_win_box, *bottom_win;
    int n_options;
    
    clear();

    //initialize color schemes used by update window
    init_pair(1, COLOR_BLUE, 0);
    init_pair(2, COLOR_YELLOW, 0); 
    init_pair(3, COLOR_RED, 0);


    
    //Initialize an array of items to put in menu
    n_options = sizeof(left_main_menu_options)/sizeof(ITEM *);
    ITEM ** my_items = calloc(n_options+1, sizeof(ITEM *)); 
    for(int i = 0; i < n_options; i++){
            my_items[i] = new_item(left_main_menu_options[i], "");
    }
    
    //initialize menu
    my_menu = new_menu(my_items);
    
    //create the left, right, and bottom windows for main menu.
    left_win = newwin(terminal_y/2, (terminal_x/2), 0, 0);
    right_win = newwin(terminal_y/2, (terminal_x/2), 0, terminal_x/2);
    bottom_win = newwin(terminal_y/2-2, terminal_x-2, terminal_y/2+1,1);
    bottom_win_box = newwin(terminal_y/2, terminal_x, terminal_y/2, 0);
    
    keypad(left_win, TRUE);
    nodelay(left_win, TRUE);
 
    // main/sub window
    set_menu_win(my_menu, left_win);
    set_menu_sub(my_menu, derwin(left_win, 6, (terminal_x/2)-2, 3, 1));
    
    //Set title of left menu and selection character
    char program_title[] = "Welcome to AFLDFF!";
    mvwprintw(left_win, 1, (terminal_x/4)-(strlen(program_title)/2), program_title);
    set_menu_mark(my_menu, " * ");
    
    //box seperator for left menu
    box(left_win, 0, 0);
    mvwaddch(left_win, 2, 0, ACS_LTEE);
    mvwhline(left_win, 2, 1, ACS_HLINE, (terminal_x/2)-2);
    mvwaddch(left_win, 2, (terminal_x/2)-1, ACS_RTEE);

    //menu post
    post_menu(my_menu);
    wrefresh(left_win);
    
    box(bottom_win_box, 0, 0);
    wrefresh(bottom_win_box);

    while(1){
        main_menu_right(right_win);
        main_menu_bottom(bottom_win);
        ITEM * choice = main_menu_left(left_win, my_menu); 

//SWITCH USED TO CONTROL MAIN MENU OPTIONS

        if(choice != NULL){
            char * menu_selection = (char *) item_name(choice);
            //VIEW JOBS
            if(strcmp(menu_selection, left_main_menu_options[0]) == 0){ 
                global_state = VIEW_JOBS;
                break;
            }
            //DEPLOY JOBS
            else if(strcmp(menu_selection, left_main_menu_options[1]) == 0){
                global_state = WOOPS;
                return;
            }
            //APPLY PATCH
            else if(strcmp(menu_selection, left_main_menu_options[2]) == 0){
                global_state = APPLY_PATCH;
                break;
            }
            //EXIT
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
    delwin(left_win);
    left_win = NULL;
    delwin(right_win);
    right_win = NULL;
    delwin(bottom_win);
    bottom_win = NULL;
    delwin(bottom_win_box);
    bottom_win_box = NULL;
}

static void apply_patch(){
    clear(); 
    char error_message[] = "In apply patch!";
    mvprintw(terminal_y/2-1, (terminal_x/2)-(strlen(error_message)/2), error_message);
    refresh();
    getchar();
    global_state = MAIN_MENU;
    return;
}


/************************************************
 *                                              *
 *                  STOP JOBS                   *
 *                                              *
 * **********************************************/

struct ssh_on_connect_handler{
    void (*handler)(ssh_session * sshs, void * data);
};


//Do better error checking here later...

static int verify_known_host(ssh_session session){
    int state;
    
    state = ssh_is_server_known(session);
    
    switch(state){
        case SSH_SERVER_KNOWN_OK:
            break;
        case SSH_SERVER_KNOWN_CHANGED:
            break;
        case SSH_SERVER_FOUND_OTHER:
            break;
        case SSH_SERVER_FILE_NOT_FOUND:
            break;
        case SSH_SERVER_NOT_KNOWN:
            break;
        case SSH_SERVER_ERROR:
            printw("Failed to verify known host: %s\n", ssh_get_error(session));
            return -1;
    }

    return 0;
    
}


/*
 * @ssh_handler This function establishes a ssh context for issuing commands
 */

void ssh_handler(packet_info * pi, job_node * job, void * data){
    ssh_session my_ssh_session;
    int rc;


    //node is not selected skip it
    if(pi->is_selected == FALSE){
        return;
    }

    //create the ssh session
    my_ssh_session = ssh_new();
    if(my_ssh_session == NULL){
        printw("Failed to create instance for node %d\n", pi->p->instance_id); 
        refresh();
        return;
    }
    
    //set the ip for the ssh session we will assume the box is just
    //listening on port 22. Would like to make that configureable later.
    
    //get the ip that the box connected to the server from (This might
    //be wrong really need to add a config file for the user)
    struct sockaddr_in * sockin = (struct sockaddr_in *) &pi->src_addr;
    char * source_ip = inet_ntoa(sockin->sin_addr);

    ssh_options_set(my_ssh_session, SSH_OPTIONS_HOST, source_ip);
    ssh_options_set(my_ssh_session, SSH_OPTIONS_USER, "user");
    
    //try and connect to selected node 
    rc = ssh_connect(my_ssh_session);
    
    if(rc != SSH_OK){
        
        attron(COLOR_PAIR(3));
        printw("Failed to connect to %s: %s\n", source_ip, ssh_get_error(my_ssh_session));
        attroff(COLOR_PAIR(3));
        
        refresh();
        return;
    }

    //server authentication
    verify_known_host(my_ssh_session);

    //authenticate with public/private key
    ssh_userauth_publickey_auto(my_ssh_session, NULL, NULL);
    
    //perform actions after authentication
    ((struct ssh_on_connect_handler *) data)->handler(&my_ssh_session, pi);
    

    //Tell ncurses to refresh the screen
    refresh();
    

    //free the ssh session
    ssh_free(my_ssh_session);
    
}


void stop_job_on_connect(ssh_session * sshs, void * data){
    ssh_channel channel;
    int rc;
    char msg_buffer[512];

    packet_info * pi = data;
    int mach_id = pi->p->instance_id;

    printw("Attempting to stop machine %d...\n", mach_id);
    refresh();

    //open up new ssh channel
    channel = ssh_channel_new(*sshs);
    if(channel == NULL){
        attron(COLOR_PAIR(3));
        printw("Failed to stop process (bad ssh channel)");
        attroff(COLOR_PAIR(3));
        refresh();
        return;
    }
    
    //start ssh session
    rc = ssh_channel_open_session(channel);
    if(rc != SSH_OK){
        attron(COLOR_PAIR(3));
        printw("Failed to stop process (could not create ssh session)");
        attroff(COLOR_PAIR(3));
        refresh();
        return;
    }

    
    //Exec command!
    rc = ssh_channel_request_exec(channel, "pkill afl-fuzz");
    if(rc != SSH_OK){
        attron(COLOR_PAIR(3));
        printw("Failed to stop process (Failed to execute command)");
        attroff(COLOR_PAIR(3));
        refresh();
        ssh_channel_close(channel);
        ssh_channel_free(channel);
        return;
    }
    
    //The pkill command should have returned nothing. If it returned
    //something else notify the user and print the returned output.
    int n_bytes = ssh_channel_read(channel, msg_buffer, sizeof(msg_buffer), 0);
    if(n_bytes != 0){
        attron(COLOR_PAIR(3));
        printw("pkill on afl-fuzz may not have executed properly...");
        attroff(COLOR_PAIR(3));       
        
        refresh();
    }
    
    while(n_bytes > 0){
        attron(COLOR_PAIR(2));
        printw(msg_buffer);
        attroff(COLOR_PAIR(2)); 
        refresh();
        
        n_bytes = ssh_channel_read(channel, msg_buffer, sizeof(msg_buffer), 0);
    }

    if (n_bytes < 0){
         
        attron(COLOR_PAIR(3));
        printw("pkill bad read...");
        attroff(COLOR_PAIR(3));       
        refresh();
        
        ssh_channel_close(channel);
        ssh_channel_free(channel);

        return;
    }
    
    ssh_channel_send_eof(channel);
    ssh_channel_close(channel);
    ssh_channel_free(channel);
    
    printw("Machine %d has been stopped!\n", mach_id);
    refresh();
}

static void stop_jobs(){
    clear();
    
    attron(COLOR_PAIR(2));
    char message[] = "Stopping jobs...\n";
    attroff(COLOR_PAIR(2));
    
    printw(message);
    refresh();
   
    struct ssh_on_connect_handler stop_job_handler;
    stop_job_handler.handler = stop_job_on_connect; 

    global_job_list_iterator(ssh_handler, &stop_job_handler);
    
    attron(COLOR_PAIR(4));
    printw("Finished stopping jobs. [Press enter to continue]\n");
    attroff(COLOR_PAIR(4));
    refresh();

    getchar();

    global_state = VIEW_JOBS;
    return;
}


/************************************************
 *                                              *
 *               COLLECT CRASHES                *
 *                                              *
 * **********************************************/

void collect_crashes_on_connect(ssh_session * sshs, void * data){
    ssh_scp scp;
    int rc;
    
    scp = ssh_scp_new(*sshs, SSH_SCP_READ|SSH_SCP_RECURSIVE, "/home/user/out/crashes/*" );
    rc = ssh_scp_init(scp);
    rc = ssh_scp_pull_request(scp);

    while(rc == SSH_SCP_REQUEST_NEWFILE){
        
        int size = ssh_scp_request_get_size(scp);
        char * filename = strdup(ssh_scp_request_get_filename(scp));
        char * buffer = malloc(size);
        
        printw("Recieveing file %s, - %d\n", filename, size);
        
        //make path for fopen...
        char * new_file_path;
        asprintf(&new_file_path, "./crashes/%s", filename);
        FILE * fd = fopen(new_file_path, "w");

        ssh_scp_accept_request(scp);
        rc = ssh_scp_read(scp, buffer, size);

        if(rc == SSH_ERROR){
            attron(COLOR_PAIR(3));
            printw("Failed to retrieve file:%s\n", filename);
            attroff(COLOR_PAIR(3));
            continue;
        }
        
        fwrite(buffer, 1, rc, fd);
        fclose(fd);
        
        free(new_file_path);
        free(filename);
        free(buffer);
        
        //start the process over again.
        rc = ssh_scp_pull_request(scp);
    }

    ssh_scp_close(scp);
    ssh_scp_free(scp);


}


static void collect_crashes(){
    clear();
    
    attron(COLOR_PAIR(2));
    char message[] = "Collecting crashes...\n";
    printw(message);
    attroff(COLOR_PAIR(2));
    refresh();
    
    
    printw("Creating directory ./crashes to store crashes...\n");
    mkdir("./crashes", 0755);


    struct ssh_on_connect_handler collect_crashes_handler;
    collect_crashes_handler.handler = collect_crashes_on_connect; 

    global_job_list_iterator(ssh_handler, &collect_crashes_handler);
    
    attron(COLOR_PAIR(4));
    printw("Finished collecting crashes. [Press enter to continue]\n");
    attroff(COLOR_PAIR(4));
    refresh();

    getchar();

    global_state = VIEW_JOBS;
    return;
}




/************************************************
 *                                              *
 *                  VIEW JOBS                   *
 *                                              *
 * **********************************************/


static char * left_view_jobs_options[]={
    "Stop Job",
    "Collect crashes",
    "Main menu"
};

static ITEM * view_jobs_left(WINDOW * left_win, MENU * menu, int c){
    switch(c){
        case KEY_DOWN:
            menu_driver(menu, REQ_DOWN_ITEM);
            wrefresh(left_win);
            break;
        case KEY_UP:
            menu_driver(menu, REQ_UP_ITEM);
            wrefresh(left_win);
            break;
        case 10:
            return current_item(menu);
            break;
    }

    return NULL;
}

static size_t get_total_jobs(GSList * start){
    pthread_mutex_lock(&ll_mutex);
        size_t counter = 0;
        for(; start; start=start->next){
            counter++;
        }
    pthread_mutex_unlock(&ll_mutex);
    return counter; 
}

static char * hash_to_string(unsigned char * hash){
    char * hash_string = calloc(1,32+1);

    for(int i=0; i<16;i++){
        sprintf(&hash_string[i*2], "%02x", hash[i]);
    }

    hash_string[32] = '\x00';
    
    return hash_string;
}



/*Doing some janky stuff here that mimics object oriented programing.
  Object pushed onto the menu queue are of type menu queue handler.
  structure in menu_queue_handler is either a job_node or a packet 
  info structure. The handler lets us set the item in the structure
  appropriately.
  
  Most of these functions are not static because of the use of 
  function pointers.
  
  */

struct menu_queue_handler{
    void * structure;
    void (*toggle)(void * structure);
    void (*print)(void * structure, int line, int selected_element, int active_window, WINDOW * window);
};

void toggle_job(void * structure){
    job_node * job = structure;
    job->is_open = job->is_open ? 0:1;
}

void print_job(void * structure, int line, int selected_element, int active_window, WINDOW * window){

    job_node * job = structure;
    char * hash = hash_to_string((unsigned char *)job->hash);
    
    if(line!=selected_element){
        wattron(window, COLOR_PAIR(2));
    }else{
        if(active_window == FALSE){
            wattron(window, COLOR_PAIR(2));
        }
    }

    mvwprintw(window, 3+line, 1+(terminal_x-20)/4 - 6, "%.16s...", hash); 
    mvwprintw(window, 3+line, 1+(terminal_x-20)/4 * 2, "%lld", get_crash_cases_by_job(job->packet_info_list));
    mvwprintw(window, 3+line, 1+(terminal_x-20)/4 * 3, "%lld", get_test_cases_by_job(job->packet_info_list));
    
    wattroff(window, COLOR_PAIR(2));
    
    free(hash);
    
}

void toggle_packet(void * structure){
    packet_info * pi = structure;
    pi->is_selected = pi->is_selected ? 0:1;
}

void print_packet(void * structure, int line, int selected_element, int active_window, WINDOW * window){
 
    packet_info * pi = structure;
    packet * p = pi->p;
    

    if(pi->is_selected && (line!=selected_element)){
        wattron(window, COLOR_PAIR(3));
    }

    if(pi->is_selected){
        mvwprintw(window, 3+line, 2, "S"); 
    }

    mvwprintw(window, 3+line, 1+(terminal_x-20)/4 - 6, "%d", p->instance_id); 
    mvwprintw(window, 3+line, 1+(terminal_x-20)/4 * 2, "%lld", p->crashes);
    mvwprintw(window, 3+line, 1+(terminal_x-20)/4 * 3, "%lld", p->test_cases);

    if(pi->is_selected){
        wattroff(window, COLOR_PAIR(3));
    }

}


static int view_jobs_right_list_jobs(WINDOW * right_win, int selected_element, int keyboard_input, int active_window){
    
    
    size_t max_display = terminal_y/2;
    
    pthread_mutex_lock(&ll_mutex);
        
//BUILD MENU QUEUE

        GQueue * menu_queue = g_queue_new();

        for(int i = 0; i < get_total_jobs(GLOBAL_JOB_MATRIX); i++){
            GSList * job_group = g_slist_nth(GLOBAL_JOB_MATRIX, i);

            //set the job handler
            struct menu_queue_handler * mqhj = calloc(1, sizeof(struct menu_queue_handler));
            mqhj->structure = job_group->data;
            mqhj->toggle = toggle_job;
            mqhj->print = print_job;
            g_queue_push_tail(menu_queue, mqhj);

            //set the packet_handler
            job_node * current_job = mqhj->structure;
            if(current_job->is_open == 1){
                for(GSList * gslp = current_job->packet_info_list; gslp; gslp=gslp->next){
                    struct menu_queue_handler * mqhp = calloc(1, sizeof(struct menu_queue_handler));
                    mqhp->structure = gslp->data;
                    mqhp->toggle = toggle_packet;
                    mqhp->print = print_packet;
                    g_queue_push_tail(menu_queue, mqhp);
                }
            }

        }
        
//HANDLE PRINTING

        //statement below looks redundent but used to truncate elements max_display
        int start_element = max_display*(selected_element/max_display);

        for(int i = start_element; (i < (start_element+max_display)); i++){
            
            struct menu_queue_handler * queue_item =  g_queue_peek_nth(menu_queue, i);
            
            if(queue_item == NULL){
                break;
            }
            
            //color the currently selected job blue (Change this to highlight later?)
            if((i == selected_element) && active_window){
                wattron(right_win, COLOR_PAIR(1));
            }
            
            queue_item->print(queue_item->structure, i, selected_element, active_window, right_win);  

            if((i == selected_element) && active_window){
                wattroff(right_win, COLOR_PAIR(1));
            }

        }
        

//CHECK FOR RIGHT MENU SELECTION
        if(active_window){
            if(keyboard_input == '\n'){
                struct menu_queue_handler * queue_item =  g_queue_peek_nth(menu_queue, selected_element);
                queue_item->toggle(queue_item->structure);
            }

        }


    pthread_mutex_unlock(&ll_mutex);
        
    //free all the menu_queue_handler structures and return
    //the number of items pushed onto the queue
    
    int queue_items=0;
    for(struct menu_queue_handler * mqh = g_queue_pop_head(menu_queue); mqh; mqh = g_queue_pop_head(menu_queue)){
        queue_items++;
        free(mqh);
    }


    return queue_items;

}


/*
 * @view_jobs_right draws box and tittles for the right menu. This
 * function also calls view_jobs_right_list_jobs to handle its
 * components that need live updating.
 */

static int view_jobs_right(WINDOW * right_win, int selected_element, int keyboard_input, int window_is_active){
    
    werase(right_win);
    box(right_win, 0, 0);

    char *  message = "Status";
    mvwprintw(right_win, 1, 1, message);
    
    message = "Group Name";
    mvwprintw(right_win, 1, 1+(terminal_x-20)/4 - 6, message);   
    
    message = "Crashes";
    mvwprintw(right_win, 1, 1+(terminal_x-20)/4 * 2, message);  
    
    message = "Tests";
    mvwprintw(right_win, 1, 1+(terminal_x-20)/4 * 3, message);
    
    int job_items = view_jobs_right_list_jobs(right_win, selected_element, keyboard_input, window_is_active);

    wrefresh(right_win);
    return job_items;
}

/*
 * @view_jobs intializes the state of the view_jobs menu. This function 
 * initializes windows, handles the logic for the menu system, and 
 * creates the main loop used for live updating.
 */

static void view_jobs(){
    clear();
    refresh();

    WINDOW *left_win, *right_win;
    
    //initialize color schemes 
    init_pair(1, COLOR_BLUE, 0);
    init_pair(2, COLOR_YELLOW, 0); 
    init_pair(3, COLOR_RED, 0);
    init_pair(4, COLOR_GREEN, 0); 

    //create windows
    left_win = newwin(terminal_y, 21, 0, 0);
    right_win = newwin(terminal_y, terminal_x - 21, 0, 21);
    box(left_win,0,0);
    box(right_win,0,0);
    wrefresh(left_win);
    wrefresh(right_win);

    MENU * my_menu;
    int n_options;
     
    //Initialize an array of items to put in menu
    n_options = sizeof(left_view_jobs_options)/sizeof(ITEM *);
    ITEM ** my_items = calloc(n_options+1, sizeof(ITEM *)); 
    for(int i = 0; i < n_options; i++){
            my_items[i] = new_item(left_view_jobs_options[i], "");
    }
    
    //initialize menu
    my_menu = new_menu(my_items);
    
    //enable control for left_win and right_win 
    keypad(left_win, TRUE);
    nodelay(left_win, TRUE);
    nodelay(right_win, TRUE);

    // main/sub window
    set_menu_win(my_menu, left_win);
    set_menu_sub(my_menu, derwin(left_win, 6, 18, terminal_y/2 - n_options, 1));
    set_menu_mark(my_menu, " * ");

    //menu post
    post_menu(my_menu);
    wrefresh(left_win);
       
    int selected_panel = LEFT; 
    int right_selected_item = 0;
    int right_panel_active = 0;
    int last_queue_elements = 0;
    ITEM * choice = NULL; 
    

    while(1){
        int c;

        if(selected_panel == LEFT){ 
            c = wgetch(left_win);
            right_panel_active = 0;
            if(c == KEY_RIGHT){
                keypad(left_win, FALSE);
                keypad(right_win, TRUE);
                selected_panel = RIGHT;
            }
            choice = view_jobs_left(left_win, my_menu, c);
        }
        
        if(selected_panel == RIGHT){
            c = wgetch(right_win);
            right_panel_active = 1;
            //right_selected_item = 1; 

            if(c == KEY_LEFT){
                keypad(right_win, FALSE);
                keypad(left_win, TRUE);
                selected_panel = LEFT;
                right_selected_item = 0;
            }


            if((c == KEY_DOWN) && (right_selected_item < (last_queue_elements-1))){
                right_selected_item++;
            }
            

            if((c == KEY_UP) && (right_selected_item > 0)){
                right_selected_item--;
            }


        }

        last_queue_elements = view_jobs_right(right_win, right_selected_item, c, right_panel_active); 

        if(choice != NULL){
            char * menu_selection = (char *) item_name(choice);
            //STOP JOB
            if(strcmp(menu_selection, left_view_jobs_options[0]) == 0){ 
                global_state = STOP_JOBS;
                break;
            }
            //COLLECT CRASHES
            else if(strcmp(menu_selection, left_view_jobs_options[1]) == 0){
                global_state = COLLECT_CRASHES;
                return;
            }
            //MAIN MENU
            else if(strcmp(menu_selection, left_view_jobs_options[2]) == 0){
                global_state = MAIN_MENU;
                break;
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
    delwin(left_win);
    delwin(right_win);

    return;   
}


/*
 * @woops user selects a menu option that does not exist.
 * This function should never get called.
 */

static void woops(){
    char error_message[] = "Woops! your not suppose to be here!";
    
    clear();
    mvprintw(terminal_y/2-1, (terminal_x/2)-(strlen(error_message)/2), error_message);
    refresh();
    getchar();
    endwin();
    exit(1);
}


/*
 * @draw_afldff_interface this should be the only function
 * called outside of a afldff-ncurses.c This function controls
 * the state of main menu.
 */

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
            case APPLY_PATCH: apply_patch(); break;
            case VIEW_JOBS: view_jobs(); break;
            case STOP_JOBS: stop_jobs(); break;
            case COLLECT_CRASHES: collect_crashes(); break;
            case WOOPS: woops(); break;

        }
    }
    exit(0);
}
