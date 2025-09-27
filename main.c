#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "state.h"

void post_status(WINDOW *, int);
void print_menu(WINDOW *, int);
void log_state(char *);

uint32_t state;
uint8_t lock;

char* choices[] = {"SAFE TO APPROACH\0", "READY TO LAUNCH\0", "CRAWL\0", "FAULT\0", "quit\0"};
int n_choices = sizeof(choices) / sizeof(char *);

static void change_state(uint32_t *state, uint32_t target, uint8_t *lock, uint8_t *success){
    if(*lock){
        *success = 0; return;
    }
    else{
        // fault check first saves cycles in event of failure
        if(target != FAULT){ /* continue */ }
        if(target != (*state + 1) % FAULT /* always last in enum */){
            *success = 0;
            *lock = 1;
            *state = FAULT;
            log_state("Invalid state transition attempted, entering FAULT state");
        }
        *lock = ! *lock;
        *state = target;
        *lock = ! *lock;
        *success = 1;
        return;
    }
}


int main(int argc, char *argv[]){
    WINDOW *menu_win;
    WINDOW *stat_win;
    // initialize ncurses
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE); // non-blocking input
    curs_set(0);
    
    int highlight = 0;
    int choice = -1;
    int c;

    lock = 0;
    state = SAFE_TO_APPROACH;
    
    char bw[] = "-bw";
    int color_mode = strcmp(argv[1 % argc], bw);

    if (has_colors() == FALSE && !color_mode) {
        endwin();
        printf("Your terminal does not support color, run again with -bw\n");
        return 1;
    }
    else if(color_mode){
        start_color();
        init_pair(1, COLOR_WHITE, COLOR_BLUE);  // main screen color (white on blue)
        init_pair(2, COLOR_WHITE, COLOR_RED); // highlight color (white on black)
        init_pair(3, COLOR_BLACK, COLOR_WHITE); // window color (black on white)
        init_pair(4, COLOR_WHITE, COLOR_GREEN); // status color

    }
    else{
        init_pair(1, COLOR_WHITE, COLOR_BLACK);
        init_pair(2, COLOR_BLACK, COLOR_WHITE);
        init_pair(3, COLOR_WHITE, COLOR_BLACK);
    }

    wbkgd(stdscr, COLOR_PAIR(1));
    refresh();

    int height = n_choices + 4; // Menu items + padding + border
    int width = 30;
    int starty = (LINES - height) / 2; // Center it vertically
    int startx = (COLS - width) / 2;   // Center it horizontally

    menu_win = newwin(height, width, starty, startx);
    keypad(menu_win, TRUE); // Enable keyboard input for the window
    wbkgd(menu_win, COLOR_PAIR(3));
    refresh();
    
    mvprintw(0, 0, "Use arrow keys to go up and down, Press enter to select a choice");
    refresh();

    print_menu(menu_win, 0);
    refresh();

    // current status output

    int stat_h = 3;
    int stat_w = 30;
    int stat_y = 5;
    int stat_x = (COLS - width) / 2;

    stat_win = newwin(stat_h, stat_w, stat_y, stat_x);
    wbkgd(stat_win, COLOR_PAIR(3));
    refresh();

    char *cstate = "SAFE TO APPROACH\0";

    box(stat_win, 0, 0);

    wattron(stat_win, COLOR_PAIR(4));
    mvwprintw(stat_win, 1, 2, "%s", cstate);
    wattroff(stat_win, COLOR_PAIR(4));
    wrefresh(stat_win);

    while (1) {
        c = wgetch(menu_win); // Get user input
        switch (c) {
            case KEY_UP:
                if (highlight == 0)
                    highlight = n_choices - 1;
                else
                    --highlight;
                break;
            case KEY_DOWN:
                if (highlight == n_choices - 1)
                    highlight = 0;
                else
                    ++highlight;
                break;
            case 10: // Enter key
                choice = highlight;

                if(choice == n_choices - 1){
                    goto cleanup;
                }

                post_status(stat_win, choice);

                break;
            default:
                // You can add other key handling here if needed
                break;
        }
        print_menu(menu_win, highlight);

    }

cleanup:
    clrtoeol();
    refresh();
    delwin(menu_win);
    delwin(stat_win);
    endwin();
   
    return 0;

}

void log_state(char *msg){
    FILE *logf = fopen("state.log", "a");
    if(!logf) return;

    time_t now = time(NULL);
    char *timestr = ctime(&now);
    timestr[strlen(timestr) - 1] = '\0'; // remove newline

    fprintf(logf, "[%s] %s\n", timestr, msg);
    fclose(logf);
}

void post_status(WINDOW *stat_win, int istatus){
    box(stat_win, 0, 0);
    refresh();

    int pal; // color pallete

    uint8_t success = 0;
    
    change_state(&state, istatus, &lock, &success);

    if(istatus == FAULT || !success){
        wattron(stat_win, COLOR_PAIR(2));
        pal = 2;
    }
    else{
        wattron(stat_win, COLOR_PAIR(4));
        pal = 4;
    }

    mvwprintw(stat_win, 1, 2, "                       ");

    if(!success){
        mvwprintw(stat_win, 1, 2, "%s", choices[FAULT]);
        wrefresh(stat_win);
        return;
    }

    log_state(choices[istatus]);
    mvwprintw(stat_win, 1, 2, "%s", choices[istatus]);
    wattroff(stat_win, COLOR_PAIR(pal));
    wrefresh(stat_win);
}

void print_menu(WINDOW *menu_win, int highlight) {
    int x = 2, y = 2; // Starting position for menu items inside the window
    box(menu_win, 0, 0);

    for (int i = 0; i < n_choices; ++i) {
        if (highlight == i) { // Highlight the present choice
            wattron(menu_win, COLOR_PAIR(2));
            mvwprintw(menu_win, y, x, "%s", choices[i]);
            wattroff(menu_win, COLOR_PAIR(2));
        } else {
            mvwprintw(menu_win, y, x, "%s", choices[i]);
        }
        ++y;
    }
    wrefresh(menu_win);
}