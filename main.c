#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// #include <time.h>
#include <unistd.h>
#include <ncurses.h>


#define sth 0


typedef struct _space {

} SPACE;

typedef struct _property {

} PROPERTY;

typedef struct _player {

} PLAYER;

typedef struct _flag {

} FLAG;

typedef enum _endpoint {
    game_start,
    game_process,
    game_end,
    eol
} ENDPOINT;



SPACE * gameboard = NULL;
int space_count;

PROPERTY * properiies = NULL;
int properties_count;

PLAYER * players = NULL;
int player_count, playerIndex;

ENDPOINT game_state;

void getOptions(int argc, char * argv[]);

void drawBoard();

void drawResults();

void moveNextPlayer();

void Game(){

    game_state = game_start;

    while (1) {
        switch (game_state)
        {
        case game_start:
            initGameboard();
            drawBoard();
            break;

        case game_process:
            moveNextPlayer();
            drawBoard();
            break;

        case game_end:
            drawResults();
            break;

        case eol:
            return;

        default:
            break;
        }
    }

}

int main(int argc, char const *argv[])
{
    getOptions(argc, argv); // +FLAGS

    Game(); //FLAGS here

    return 0;
}
