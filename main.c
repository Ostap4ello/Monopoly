#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <ncurses.h>



#define MAX_NAME 100        // max. namelength
#define MAX_PLAYERS 4       // max. player count
#define NUM_SPACE_TYPES 6
#define NUM_COLORS 8        // color count



// Game states
typedef enum _endpoint {
    game_start,             // game initialisation state
    game_process,           // game process - play
    game_end,               // game end - prompt
    eol
} ENDPOINT;

// Properties' color values
typedef enum _color {
    Brown,
    Skyblue,
    Purple,
    Orange,
    Red,
    Yellow,
    Green,
    Blue
} COLOR;

// Space types
typedef enum _space_type {
    Property,
    Start,
    Free_parking,   // No action space
    In_jail,
    Go_to_jail,
    Jail_pass
} SPACE_TYPE;



// Structure for properties' specs
typedef struct _property {
    char name[MAX_NAME + 1];
    int price;
    COLOR color;
    int isOwned;        // -1 if not owned, playerIndex otherwise
    int isMonopoly;     // 1 if is monopoly, 0 otherwise
} PROPERTY;

// Structure for spaces' specs
typedef struct _space {
    SPACE_TYPE type;        // space type on gameboard
    PROPERTY *property;     // NULL if space is not a property, pointer to corresponding property otherwise
} SPACE;

// Structure for player information
typedef struct _player {
    int index;                      // playerIndex, index of player
    int space_number;               // space number the player is currently standing on
    int cash;                       // player's cash
    int num_jail_pass;              // player's number of jail skip passes
    int is_in_jail;                 // 1 if player is currently in jail, 0 otherwise
    PROPERTY ** owned_properties;   // pointer to list of pointers to properties, owned by player
    int num_properties;             // owned properties count
} PLAYER;

// Structure for flag info
typedef struct _flag {
    int isOn;           // 1 if flag is on, 0 otherwise
    char * argument;    // pointer to argv argument if aviable, NULL otherwise
} FLAG;


// global variables definitions
SPACE * gameboard = NULL;
int space_count;

PROPERTY * properiies = NULL;
int properties_count;

PLAYER * players = NULL;
int player_count, playerIndex;

ENDPOINT game_state;

// get cl arguments and write into FLAG
void getOptions(int argc, char * argv[]);

// initialise players, gameboard, properties and global variables
void initGame();

void drawBoard();

void drawResults();

void moveNextPlayer();

void Game(){

    game_state = game_start;

    while (1) {
        switch (game_state)
        {
        case game_start:
            initGame();

            game_state = game_process;
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
