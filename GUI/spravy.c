
// Created by Acer on 8. 1. 2022.
//
//CHAT, aj pre s kupinu s nejakymi upravami
#include <ncurses.h>
#include <form.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

static FORM *form;
static FIELD *fields[3];
static WINDOW *win_body, *win_form;
int pocet = 0;

/*
 * This is useful because ncurses fill fields blanks with spaces.
 */
static char* trim_whitespaces(char *str)
{
    char *end;

    // trim leading space
    while(isspace(*str))
        str++;

    if(*str == 0) // all spaces?
        return str;

    // trim trailing space
    end = str + strnlen(str, 128) - 1;

    while(end > str && isspace(*end))
        end--;

    // write new null terminator
    *(end+1) = '\0';

    return str;
}

static void driver(int ch)
{
    int i;

    switch (ch) {
        case KEY_F(2):
            // Or the current field buffer won't be sync with what is displayed
            form_driver(form, REQ_NEXT_FIELD);
            form_driver(form, REQ_PREV_FIELD);

            move(LINES-22+pocet, 2);

            for (i = 0; fields[i]; i++) {
                printw("%s", trim_whitespaces(field_buffer(fields[i], 0)));

                if (field_opts(fields[i]) & O_ACTIVE)
                    printw("\"\t");
                else
                    printw(": \"");
            }
            pocet += 3;
            form_driver(form, REQ_CLR_EOF);
            refresh();
            pos_form_cursor(form);
            break;

        case KEY_DOWN:
            form_driver(form, REQ_NEXT_FIELD);
            form_driver(form, REQ_END_LINE);
            break;

        case KEY_UP:
            form_driver(form, REQ_PREV_FIELD);
            form_driver(form, REQ_END_LINE);
            break;

        case KEY_LEFT:
            form_driver(form, REQ_PREV_CHAR);
            break;

        case KEY_RIGHT:
            form_driver(form, REQ_NEXT_CHAR);
            break;

            // Delete the char before cursor
        case KEY_BACKSPACE:
        case 127:
            form_driver(form, REQ_DEL_PREV);
            break;

            // Delete the char under the cursor
        case KEY_DC:
            form_driver(form, REQ_DEL_CHAR);
            break;

        default:
            form_driver(form, ch);
            break;
    }

    wrefresh(win_form);
}

int main()
{
    int ch;

    initscr();
    noecho();
    cbreak();
    keypad(stdscr, TRUE);

    win_body = newwin(8, 80, 0, 0);
    assert(win_body != NULL);
    box(win_body, 0, 0);
    win_form = derwin(win_body, 5, 78, 2, 1);
    assert(win_form != NULL);
    box(win_form, 0, 0);
    mvwprintw(win_body, 1, 1, "F1 to quit, F2 send message, CHAT");

    fields[0] = new_field(1, 10, 2, 5, 0, 0);
    fields[1] = new_field(1, 40, 2, 15, 0, 0);
    fields[2] = NULL;
    assert(fields[0] != NULL && fields[1] != NULL);

    set_field_buffer(fields[0], 0, "MESSAGE");
    set_field_buffer(fields[1], 0, " ");

    set_field_opts(fields[0], O_VISIBLE | O_PUBLIC | O_AUTOSKIP);
    set_field_opts(fields[1], O_VISIBLE | O_PUBLIC | O_EDIT | O_ACTIVE);

    set_field_back(fields[1], A_UNDERLINE);

    form = new_form(fields);
    assert(form != NULL);
    set_form_win(form, win_form);
    set_form_sub(form, derwin(win_form, 3, 76, 1, 1));
    post_form(form);

    refresh();
    wrefresh(win_body);
    wrefresh(win_form);

    while ((ch = getch()) != KEY_F(1))
        driver(ch);

    unpost_form(form);
    free_form(form);
    free_field(fields[0]);
    free_field(fields[1]);
    delwin(win_form);
    delwin(win_body);
    endwin();

    return 0;
}
