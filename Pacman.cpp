/*******************************************************************************
 * constants.h
 ******************************************************************************/
#ifndef CONSTANTS_H
#define CONSTANTS_H

const int ROWS = 20;
const int COLS = 20;
const int MIDROW = ROWS / 2;
const int MIDCOL = COLS / 2;
const char SPACE = ' ';
const char GHOST = 'G';
const char DOT = '.';
const char BOULDER = '#';

const char CMD_UP = 'w';
const char CMD_DWN = 's';
const char CMD_LFT = 'a';
const char CMD_RGT = 'd';

const char UP = '^';
const char DOWN = 'v';
const char LEFT = '<';
const char RIGHT = '>';

#endif


/*******************************************************************************
 * termfuncs.h
 ******************************************************************************/
#ifndef TERMFUNCS_H
#define TERMFUNCS_H

const char KEY_UP    = 130;
const char KEY_DOWN  = 131;
const char KEY_LEFT  = 132;
const char KEY_RIGHT = 133;
const char ESCAPE    = 27;

char getachar();
char getacharnow(int);
void screen_clear();
void screen_home();

int  random_int(int, int);
void seed_random(int);

#endif


/*******************************************************************************
 * pacman.h
 ******************************************************************************/
#ifndef PACMAN_H_INCLUDED
#define PACMAN_H_INCLUDED

#include "constants.h"

class Pacman {
    public:
        Pacman();
        void center();
        bool move(char[ROWS][COLS], char command);
        void place_on_board(char[ROWS][COLS]);
        int get_num_moves();
        void set_col(int);
        void set_row(int);
        int  get_col();
        int  get_row();
        bool is_at(int, int);
        void add_to_score(int);
        int get_score();
        void die();
        bool is_alive();
        void set_dots(int);
        int get_dots();

    private:
        bool alive;
        char head;
        int row, col;
        int num_moves;
        int score;
        int dots_eaten;

        void rotate_head(char);
        bool boulder_ahead(char[ROWS][COLS], char);
};

#endif


/*******************************************************************************
 * ghost.h
 ******************************************************************************/
#ifndef GHOST_H_INCLUDED
#define GHOST_H_INCLUDED

#include "constants.h"

class Ghost {
    public:
        Ghost();
        void set_location(int r, int c);
        void set_random_location();
        bool move(char[ROWS][COLS], int target_r, int target_c);
        void place_on_board(char[ROWS][COLS]);
        bool is_at(int, int);

    private:
        int row, col;
        char board_char;
};

#endif


/*******************************************************************************
 * game.h
 ******************************************************************************/
#ifndef GAME_H_INCLUDED
#define GAME_H_INCLUDED

#include "pacman.h"
#include "ghost.h"

class Game {
    public:
        Game();
        void print_manual();
        void run();
    private:
        char board[ROWS][COLS];
        Pacman pacman;
        Ghost ghost;
        void setup_board();
        void draw_board();
        bool game_over();
};

#endif


/*******************************************************************************
 * termfuncs.cpp
 ******************************************************************************/
#include <iostream>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <ctype.h>
using namespace std;
#include "termfuncs.h"

static const string CSI = "\033[";

static termios prev_tty_state;
static int prev_state_stored = 0;

static char handle_escape_seq();

char getachar()
{
    char q_key = 0;
    char *qk = NULL;
    char c;
    int  nread;

    if ( ( qk = getenv("TF_QUIT") ) != NULL ){
        q_key = qk[0];
    }
    cout << std::flush;
    if ( isatty(0) ){
        struct termios	info, orig;
        tcgetattr(0, &info);
        orig = info;
        prev_tty_state = orig;
        prev_state_stored = 1;

        info.c_lflag &= ~ECHO;
        info.c_lflag &= ~ICANON;
        tcsetattr(0, TCSANOW, &info);

        nread = read(0, &c, 1);
        if ( nread == 1 && c == ESCAPE )
            c = handle_escape_seq();

        tcsetattr(0, TCSANOW, &orig);
    }
    else {
        if ( cin.get(c) )
            nread = 1;
        else
            nread = 0;
        if ( nread == 0 && qk != NULL )
            exit(0);
    }

    if ( nread == 1 && qk != NULL && c == q_key ){
        exit(0);
    }

    return ( nread > 0 ? c : 0 );
}

char getacharnow(int decisecs)
{
    char c;

    cout << std::flush;
    if ( isatty(0) ) 
    {
        struct termios	info, orig;
        tcgetattr(0, &info);
        orig = info;
        prev_tty_state = orig;
        prev_state_stored = 1;

        info.c_lflag &= ~ECHO;
        info.c_lflag &= ~ICANON;
        info.c_cc[VMIN] = 0;
        info.c_cc[VTIME] = decisecs;
        tcsetattr(0, TCSANOW, &info);

        if ( read(0, &c, 1) == 1 ){
            if ( c == ESCAPE )
                c = handle_escape_seq();
        }
        else
            c = '\0';

        tcsetattr(0, TCSANOW, &orig);
    }
    else {
        if ( read(0, &c, 1) != 1 )
            c = '\0';
    }

    return (int) c;
}

char handle_escape_seq()
{
    struct termios	info, orig;
    char		buf[100];
    int		pos = 0;
    char		c;
    void		(*oldhand)(int);

    if ( !isatty(0) ) 
        return ESCAPE;

    oldhand = signal(SIGINT, SIG_IGN);

    tcgetattr(0, &info);
    orig = info;

    info.c_lflag &= ~ECHO;
    info.c_lflag &= ~ICANON;
    info.c_cc[VMIN] = 0;
    info.c_cc[VTIME] = 1;
    tcsetattr(0, TCSANOW, &info);

    do
    {
        if ( read(0, &c, 1) == 1 )    
            buf[pos++] = c;        
        else
            break;            
    }
    while( !isalpha((int)c) && pos < 99 );    

    if ( pos == 0 || (buf[0] != '[' && buf[0] != 'O' ) )
        c = ESCAPE;
    else 
        switch(buf[1]){
            case 'A':        c = KEY_UP;    break;
            case 'B':        c = KEY_DOWN;    break;
            case 'C':        c = KEY_RIGHT;    break;
            case 'D':        c = KEY_LEFT;    break;
            default :
                    c = ESCAPE;
        }

    tcsetattr(0, TCSANOW, &orig);
    signal(SIGINT, oldhand);
    return c;
}

void screen_clear()
{
    cout << (CSI + "H");
    cout << (CSI + "2J");
}

void screen_home()
{
    cout << (CSI + "H");
}

static int rand_seed = -1;

void seed_random(int s)
{
    if ( s > 0 ) {
        srand(s);
        rand_seed = s;
    }
}

int random_int(int lo, int hi)
{
    int	range = hi - lo + 1;
    char	*seed_str;

    if ( rand_seed == -1 ) {
        seed_str = getenv( "SNAKE_SEED" );
        if ( seed_str != NULL )
            seed_random( (unsigned) atoi(seed_str) );
        else 
            seed_random( (unsigned) time(NULL) );
    }
    if ( range <= 0 )
        return 17;

    return lo + ( rand() % range );
}


/*******************************************************************************
 * pacman.cpp
 ******************************************************************************/
#include <iostream>
#include "pacman.h"
#include "constants.h"

Pacman::Pacman() {
    center();
    alive = true;
    head = UP;
    num_moves = 0;
    score = 0;
    dots_eaten = 0;
}

void Pacman::center() {
    row = MIDROW;
    col = MIDCOL;
    head = UP;
}

bool Pacman::move(char board[ROWS][COLS], char command) {
    bool ate_dot = false;
    rotate_head(command);
    num_moves++;
    if (!boulder_ahead(board, command)){
        board[row][col] = SPACE;
        if (command == CMD_UP) {
            if (row == 0) { row = ROWS-1; } else { row--; }
        } else if (command == CMD_DWN) {
            if (row == ROWS-1) { row = 0; } else { row++; }
        } else if (command == CMD_RGT) {
            if (col == COLS-1) { col = 0; } else { col++; }
        } else if (command == CMD_LFT) {
            if (col == 0) { col = COLS-1; } else { col--; }
        }
    }
    if (board[row][col] == GHOST) die();
    if (board[row][col] == DOT) {
        ate_dot = true;
        dots_eaten++;
    }
    return ate_dot;
}

void Pacman::place_on_board(char board[ROWS][COLS]) {
    board[row][col] = head;
}

void Pacman::rotate_head(char command) {
    if (command == CMD_UP) { head = UP; }
    else if (command == CMD_DWN) { head = DOWN; }
    else if (command == CMD_LFT) { head = LEFT; }
    else if (command == CMD_RGT) { head = RIGHT; }
}

bool Pacman::boulder_ahead(char board[ROWS][COLS], char command) {
    if (command == CMD_UP) {
        if (row == 0) return (board[ROWS-1][col] == BOULDER);
        return (board[row-1][col] == BOULDER);
    }
    else if (command == CMD_DWN) {
        if (row == ROWS-1) return (board[0][col] == BOULDER);
        return (board[row+1][col] == BOULDER);
    }
    else if (command == CMD_RGT) {
        if (col == COLS-1) return (board[row][0] == BOULDER);
        return (board[row][col+1] == BOULDER);
    }
    else if (command == CMD_LFT) {
        if (col == 0) return (board[row][COLS-1] == BOULDER);
        return (board[row][col-1] == BOULDER);
    }
    return false;
}

void Pacman::add_to_score(int points) {
    score += points;
}

int Pacman::get_score() {
    return score;
}

void Pacman::die() {
    alive = false;
}

bool Pacman::is_alive() {
    return alive;
}

void Pacman::set_dots(int num_dots) {
    dots_eaten = num_dots;
}

int Pacman::get_dots() {
    return dots_eaten;
}


/*******************************************************************************
 * ghost.cpp
 ******************************************************************************/
#include "ghost.h"
#include "constants.h"

Ghost::Ghost() {
    row = rand() % ROWS;
    col = rand() % COLS;
    board_char = GHOST;
}

void Ghost::set_location(int r, int c) {
    row = r;
    col = c;
}

void Ghost::set_random_location() {
    row = rand() % ROWS;
    col = rand() % COLS;
}

bool Ghost::move(char board[ROWS][COLS], int target_r, int target_c) {
    if (target_r > row) row++;
    else if (target_r < row) row--;
    else if (target_c > col) col++;
    else if (target_c < col) col--;
    return true;
}

void Ghost::place_on_board(char board[ROWS][COLS]) {
    board[row][col] = board_char;
}

bool Ghost::is_at(int r, int c) {
    return (row == r && col == c);
}


/*******************************************************************************
 * game.cpp
 ******************************************************************************/
#include <iostream>
#include "game.h"
#include "constants.h"
#include "termfuncs.h"

Game::Game() {
    setup_board();
}

void Game::setup_board() {
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            if ((i + j) % 5 == 0) {
                board[i][j] = DOT;
            } else if (i == 0 || i == ROWS-1 || j == 0 || j == COLS-1) {
                board[i][j] = BOULDER;
            } else {
                board[i][j] = SPACE;
            }
        }
    }
    pacman.center();
    ghost.set_random_location();
}

void Game::print_manual() {
    std::cout << "Use WASD to move Pacman.\n";
}

void Game::draw_board() {
    screen_clear();
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            std::cout << board[i][j] << " ";
        }
        std::cout << std::endl;
    }
}

bool Game::game_over() {
    return !pacman.is_alive();
}

void Game::run() {
    print_manual();
    while (!game_over()) {
        draw_board();
        char input = getachar();
        if (input == CMD_UP || input == CMD_DWN || input == CMD_RGT || input == CMD_LFT) {
            pacman.move(board, input);
        }
    }
    std::cout << "Game Over! Final Score: " << pacman.get_score() << std::endl;
}


/*******************************************************************************
 * main.cpp
 ******************************************************************************/
#include "game.h"

int main() {
    Game game;
    game.run();
    return 0;
}
