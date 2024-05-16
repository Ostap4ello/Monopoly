#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <ncurses.h>
#include <ctype.h>


// --- Constants definitions ---
#define DEF_SPACE_COUNT 24
#define DEF_PROP_COUNT 16
#define MAX_NAME 100        // max. namelength
#define MAX_PLAYERS 4       // max. player count
#define NUM_SPACE_TYPES 6
#define NUM_COLORS 8        // color count

#define SPACE_HEIGHT 8
#define SPACE_WIDTH 20
// #define randint(a, b) (a + (rand() % (b-a)))


// --- Custom values ---
// Game states
typedef enum _endpoint {
    menu,                   // game menu
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


// --- Structures ---
// Structure for properties' specs
typedef struct _property {
    char name[MAX_NAME + 1];
    int price;
    COLOR color;
    int ownerIndex;        // -1 if not owned, playerIndex otherwise
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
    int isBot;                      // set it's dice value randomly if 1, user's input otherwise
} PLAYER;

// Structure for flag info
typedef struct _flag {
    int isOn;           // 1 if flag is on, 0 otherwise
    char * argument;    // pointer to argv argument if aviable, NULL otherwise
} FLAG;

// Structure for gameboard gui
struct _window_gameboard {
    WINDOW *frame;
    WINDOW * interraction;
    WINDOW ** playerStates;
    WINDOW ** spaces;
} window_gameboard;


// --- Global variables definitions ---
SPACE * gameboard = NULL;
int space_count;        // gameboard size

PROPERTY * properties = NULL;
int properties_count;

PLAYER * players[MAX_PLAYERS] = {};
int players_count, playerTurnIndex, notBotIndex;

ENDPOINT game_state;

int waitTimeJK;


// Definition of strings corresponding to type PROPERTY (types)
const char *space_types[NUM_SPACE_TYPES] = {
        "PROPERTY",
        "START",
        "FREE PARKING",
        "IN JAIL",
        "GO TO JAIL",
        "JAIL PASS"
};

// Definition of strings corresponding to type COLOR
const char *property_colors[] = {
        "Brown",
        "Skyblue",
        "Purple",
        "Orange",
        "Red",
        "Yellow",
        "Green",
        "Blue"
};


// definition of default gameboard
PROPERTY def_properties[DEF_PROP_COUNT] = {
        {.name = "FOOD TRUCK", .price = 1, .color = Brown, .ownerIndex = -1, .isMonopoly = 0},
        {.name = "PIZZA RESTAURANT", .price = 1, .color = Brown, .ownerIndex = -1, .isMonopoly = 0},
        {.name = "DOUGHNUT SHOP", .price = 1, .color = Skyblue, .ownerIndex = -1, .isMonopoly = 0},
        {.name = "ICE CREAM SHOP", .price = 1, .color = Skyblue, .ownerIndex = -1, .isMonopoly = 0},
        {.name = "MUSEUM", .price = 2, .color = Purple, .ownerIndex = -1, .isMonopoly = 0},
        {.name = "LIBRARY", .price = 2, .color = Purple, .ownerIndex = -1, .isMonopoly = 0},
        {.name = "THE PARK", .price = 2, .color = Orange, .ownerIndex = -1, .isMonopoly = 0},
        {.name = "THE BEACH", .price = 2, .color = Orange, .ownerIndex = -1, .isMonopoly = 0},
        {.name = "POST OFFICE", .price = 3, .color = Red, .ownerIndex = -1, .isMonopoly = 0},
        {.name = "TRAIN STATION", .price = 3, .color = Red, .ownerIndex = -1, .isMonopoly = 0},
        {.name = "COMMUNITY GARDEN", .price = 3, .color = Yellow, .ownerIndex = -1, .isMonopoly = 0},
        {.name = "PET RESCUE", .price = 3, .color = Yellow, .ownerIndex = -1, .isMonopoly = 0},
        {.name = "AQUARIUM", .price = 4, .color = Green, .ownerIndex = -1, .isMonopoly = 0},
        {.name = "THE ZOO", .price = 4, .color = Green, .ownerIndex = -1, .isMonopoly = 0},
        {.name = "WATER PARK", .price = 5, .color = Blue, .ownerIndex = -1, .isMonopoly = 0},
        {.name = "AMUSEMENT PARK", .price = 5, .color = Blue, .ownerIndex = -1, .isMonopoly = 0}
};

// definition of default gameboard
SPACE def_gameboard[DEF_SPACE_COUNT] = {
        {.type = Start, .property = NULL},
        {.type = Property, .property = &def_properties[0]},
        {.type = Property, .property = &def_properties[1]},
        {.type = Jail_pass, .property = NULL},
        {.type = Property, .property =  &def_properties[2]},
        {.type = Property, .property =  &def_properties[3]},
        {.type = In_jail, .property =  NULL},
        {.type = Property, .property =  &def_properties[4]},
        {.type = Property, .property =  &def_properties[5]},
        {.type = Jail_pass, .property =  NULL},
        {.type = Property, .property =  &def_properties[6]},
        {.type = Property, .property =  &def_properties[7]},
        {.type = Free_parking, .property =  NULL},
        {.type = Property, .property =  &def_properties[8]},
        {.type = Property, .property =  &def_properties[9]},
        {.type = Jail_pass, .property =  NULL},
        {.type = Property, .property =  &def_properties[10]},
        {.type = Property, .property =  &def_properties[11]},
        {.type = Go_to_jail, .property =  NULL},
        {.type = Property, .property =  &def_properties[12]},
        {.type = Property, .property =  &def_properties[13]},
        {.type = Jail_pass, .property =  NULL},
        {.type = Property, .property =  &def_properties[14]},
        {.type = Property, .property =  &def_properties[15]}
};


// --- temporary Functions --- //

// get cl arguments and write into FLAG
void getOptions(int argc, char * argv[], FLAG * flag_board, FLAG * flag_prop){
    int temp_c;
    do
    {
        extern char *optarg;

        temp_c = getopt(argc, argv, ":b:t:");
        
        switch (temp_c)
        {
        case 'b':
            flag_board->isOn = 1;

            for (int i = 1; i < argc; i++){
                if (strcmp(optarg, argv[i]) == 0) {
                    flag_board->argument = argv[i];
                    break;
                }
            }
            break;
        
        case 't':
            flag_prop->isOn = 1;

            for (int i = 1; i < argc; i++){
                if (strcmp(optarg, argv[i]) == 0) {
                    flag_prop->argument = argv[i];
                }
            }
            break;
        
        case ':':   // no argument specified 
            // fputs("No path specified\n", stderr);
            printf("No path specified\n");
            exit(0);

        default:
            break;
        }


    } while (temp_c != -1);
}



// --- Game logic functions --- //

// better random
int randint(int a, int b){
    return (int)( a + (random() % b));
}

// set gameboard and properties (if needed - read from file -- TODO)
int initGameboard(FLAG flag_b, FLAG flag_t){
    /* Errors:
    return 1 - cannot open the file
    return 2 - not enough properties
    */
    
    FILE * file_b = NULL;  // b_file is plan of gameboard
    FILE * file_t = NULL;  // t_file is list of properties

    // open files + handle opening file error
    if (flag_b.isOn == 1){
        file_b = fopen(flag_b.argument, "r");
        if (file_b ==  NULL){
            printf("Error opening the file : \"%s\"\n", flag_b.argument);
            // flag_b.isOn = 0
            // exit(1);
            return 1;
        }
    }
    if (flag_t.isOn == 1){
        file_t = fopen(flag_t.argument, "r");
        if (file_t ==  NULL){
            printf("Error opening the file \"%s\"\n", flag_t.argument);
            // flag_t.isOn = 0
            // exit(1);
            return 1;
        }
    }


    // set spaces count and prop count + check compatibility
    char temp_row[MAX_NAME] = "";
    int needed_properties_count = 0;
    
    if (flag_b.isOn){
        fscanf(file_b, "%d", &space_count);

        while (feof(file_b) == 0){
            memset(temp_row, 0, MAX_NAME-1);
            fgets(temp_row, MAX_NAME, file_b);
            if (strncmp(temp_row, "PROPERTY", 8) == 0)
            {
                needed_properties_count++;
            }
        }
    } else { //unchanged gameboard plan
        space_count = DEF_SPACE_COUNT;
        // needed_properties_count = NUM_PROPERTIES;
        needed_properties_count = 0; // when there're no gameboard plan specified, print only properties
    }

    if (flag_t.isOn){
        properties_count = 0;

        while (feof(file_t) == 0){
            memset(temp_row, 0, MAX_NAME-1);
            fgets(temp_row, MAX_NAME, file_t);
            if (temp_row[0] == '"')
            {
                (properties_count)++;
            }
        }
    } else {
        properties_count = DEF_PROP_COUNT;
    }

    if (needed_properties_count > properties_count){
        printf("Not enough properties in %s\n", flag_t.argument);
        // exit(1);
        return 2;
    }

    //fill gameboard and proerties
    int temp_nameLength = 0, temp_price = 0;
    COLOR temp_color = 0;
    SPACE_TYPE temp_propType;

    if (flag_t.isOn){
        properties = (PROPERTY *) malloc(properties_count*sizeof(PROPERTY));
        
        rewind(file_t);

        for (int propertyIndex = 0; propertyIndex < properties_count; ){
            
            memset(temp_row, 0, MAX_NAME-1);
            fgets(temp_row, MAX_NAME, file_t);

            if (temp_row[0] == '['){
                // setcolor
                for (temp_color = 0; temp_color < NUM_COLORS; temp_color++){
                    if (strncmp(&temp_row[1], property_colors[temp_color], strlen(property_colors[temp_color])) == 0){
                        break;
                    }
                }
            } else if (temp_row[0] == '\"'){
                // add property
                temp_nameLength = 0;

                while (temp_row[temp_nameLength+1] != '"' && (temp_nameLength < MAX_NAME - 1)) { ++temp_nameLength; }

                temp_price = atoi(&temp_row[3+temp_nameLength]);

                strncpy(((properties)[propertyIndex].name), &temp_row[1], temp_nameLength);
                (properties)[propertyIndex].price = temp_price;
                (properties)[propertyIndex].color = temp_color;

                propertyIndex++;
            }
        }
    } else {
        properties = def_properties;
    }

    if (flag_b.isOn){
        gameboard = (SPACE *) malloc(space_count*sizeof(SPACE));
        
        rewind(file_b);
        fgets(temp_row, MAX_NAME, file_b);
        
        for (int propertyIndex = 0, spaceIndex = 0; spaceIndex < space_count; spaceIndex++) {

            memset(temp_row, 0, MAX_NAME-1);
            fgets(temp_row, MAX_NAME, file_b);
            
            temp_propType = 0;
            for (temp_propType = 0; temp_propType < NUM_SPACE_TYPES; temp_propType++){
                if (strncmp(temp_row, space_types[temp_propType], strlen(space_types[temp_propType])) == 0){
                    break;
                }
            }

            (gameboard)[spaceIndex].type = temp_propType;
            if (temp_propType == Property)
            {
                (gameboard)[spaceIndex].property = &((properties)[propertyIndex]);
                propertyIndex++;
            }

        }
    } else {
        gameboard = def_gameboard;
    }

    // close files
    if (flag_t.isOn) {
        // char _c;
        while (feof(file_t) == 0){ fgetc(file_t); }
        fclose(file_t);
    }
    if (flag_b.isOn) {
        fclose(file_b);
    }
    return 0;
}

// create players and initialize their values
void initPlayers(){

    for (int playerIndex = 0; playerIndex < players_count; playerIndex++)
    {
        // create a player
        players[playerIndex] = calloc(1, sizeof(PLAYER));

        // initialize player values
        players[playerIndex]->index = playerIndex;
        players[playerIndex]->space_number = 0;
        players[playerIndex]->num_jail_pass = 0;
        players[playerIndex]->is_in_jail = 0;
        players[playerIndex]->num_properties = 0;
        players[playerIndex]->owned_properties = (PROPERTY **) calloc(properties_count, sizeof(PROPERTY *));
        if (playerIndex == notBotIndex){
            players[playerIndex]->isBot = 0;
        } else {
            players[playerIndex]->isBot = 1; 
        }

        // set money count depending on players_count
        switch (players_count)
        {
        case 2:
            players[playerIndex]->cash = 20;
            break;
        
        case 3:
            players[playerIndex]->cash = 18;
            break;
        
        case 4:
            players[playerIndex]->cash = 16;
            break;
        
        default:
            break;
        }
    }

    return;
}


void updateMonopolyState(const COLOR color) {

    int color_ownerIndex;
    int propertyIndex = 0;

    // check monopoly for this color
    while (properties[propertyIndex].color != color) {      // get first owner of the color
        propertyIndex++;
    }

    color_ownerIndex = properties[propertyIndex].ownerIndex;

    for (; propertyIndex < properties_count; propertyIndex++) {     // check if first owner is the same for each property with this color
        if ((properties[propertyIndex].color == color) && (properties[propertyIndex].ownerIndex != color_ownerIndex)){
            return;
            // exit if any property has another owner or isn't owned
        }
    }
    
    // else: set everything this color to Monopoly
    for (propertyIndex = 0; propertyIndex < properties_count; propertyIndex++){
        if (properties[propertyIndex].color == color){
            properties[propertyIndex].isMonopoly = 1;
        }
    }
}

// performs the move operation on player with (playerIndex)
int movePlayer(int diceValue) {

    // returns 1 if successful move, 0 if player becomes Bankrupt

    PLAYER * currentPlayer = players[playerTurnIndex];

    // JAIL check
    if (currentPlayer->is_in_jail == 1){
        if (currentPlayer->cash >= 1) {
            currentPlayer->cash -= 1;
        } else {
            return 0; // isBankrupt
        }
        currentPlayer->is_in_jail = 0;
    }
    
    // Change position
    currentPlayer->space_number += diceValue;
    // Check if player has gone to next lap
    if (currentPlayer->space_number>=space_count)
    {
        currentPlayer->space_number = 0;
        currentPlayer->cash += 2;
    }

    //now set currentSpace for better code readibility
    SPACE * currentSpace = &(gameboard[currentPlayer->space_number]);

    // Perform the action based on current position on the gameboard
    switch (currentSpace->type)
    {
    case Start:
        //pass
        break;

    case Free_parking:
        //pass
        break;

    case In_jail:
        //pass
        break;

    case Go_to_jail:
        if (currentPlayer->num_jail_pass > 0){  //skip jail
            currentPlayer->num_jail_pass -= 1;
        } else {
            currentPlayer->space_number = 6;    //go to jail due to no jailPasses
            currentPlayer->is_in_jail = 1;
        }
        break;

    case Jail_pass:
        currentPlayer->num_jail_pass += 1;
        break;

    case Property:

        PROPERTY * currentProperty = currentSpace->property;

        if (currentProperty->ownerIndex == -1){         // currentProperty is not owned

            // check if currentPlayer is UNABLE to buy this property
            if (currentPlayer->cash < currentProperty->price){
                return 0;  // isBankrupt
            }
            // buy otherwise
            currentPlayer->cash -= currentProperty->price;

            currentPlayer->owned_properties[currentPlayer->num_properties] = currentProperty;
            currentPlayer->num_properties += 1;

            currentProperty->ownerIndex = playerTurnIndex;

            updateMonopolyState(currentProperty->color); 

        } else if (currentProperty->ownerIndex != playerTurnIndex) { // currentProperty is owned by another player

            // if currentProperty is Monopoly, currentPlayer should pay twice the price

            if (currentProperty->isMonopoly == 0){  // scenario if currentProperty is NOT Monopoly

                // check if currentPlayer is UNABLE to pay rant this property
                if (currentPlayer->cash < currentProperty->price){
                    return 0;  // isBankrupt
                }

                // pay otherwise
                currentPlayer->cash -= currentProperty->price;
                players[currentProperty->ownerIndex]->cash += currentProperty->price;  // transfer money to owner

            } else {                                // scenario if currentProperty is NOT Monopoly

                // check if currentPlayer is UNABLE to pay rent this property
                if (currentPlayer->cash < 2*currentProperty->price) {
                    return 0;  // isBankrupt
                }

                // pay otherwise
                currentPlayer->cash -= 2*currentProperty->price;
                players[currentProperty->ownerIndex]->cash += 2*currentProperty->price; // transfer money to owner
            }
        }
        break;

    default:
        break;

    }

    return 1;   //success
}


int findWinner(int bankruptPlayerIndex){  // returns index of winner in players
    int max_cash = 0,
        winner_num = -2;
        // -2 for unitialized (within the function),
        // -1 is Unable to find winner,
        // Index of winner otherwise

    //find max cash player
    for (int playerIndex = 0; playerIndex < players_count; playerIndex++)
    {
        if (playerIndex == bankruptPlayerIndex) {
            continue;   // skip bankrupt player
        }
        
        if (players[playerIndex]->cash > max_cash) {
            max_cash = players[playerIndex]->cash;
        }
    }

    // find players that have the same cash as max_cash
    for (int playerIndex = 0; playerIndex < players_count; playerIndex++)
    {
        if (playerIndex == bankruptPlayerIndex) {
            continue;
        }
        if (players[playerIndex]->cash == max_cash) {
            if (winner_num == -2)
            {
                winner_num = playerIndex;
            } else {    //there is more than 1 player with the same cash as max_cash
                winner_num = -1;
            }
        }
    }

    if (winner_num >= 0){
        return winner_num;
    }
    winner_num = -2;

    //find max cash player
    int max_property_price = 0, currentPlayer_property_price = 0;
    for (int playerIndex = 0; playerIndex < players_count; playerIndex++)
    {
        if (playerIndex == bankruptPlayerIndex || players[playerIndex]->cash != max_cash) {
            continue;   // skip bankrupt player
        }

        // get this plater's properties price
        currentPlayer_property_price = 0;
        for (int propertyIndex = 0; propertyIndex < players[playerIndex]->num_properties; propertyIndex++) {
            currentPlayer_property_price += players[playerIndex]->owned_properties[propertyIndex]->price;
        }

        if (currentPlayer_property_price > max_property_price) {
            max_property_price = currentPlayer_property_price;
        }
        
    }

    // find players that have the same sum of props as max_property_price
    for (int playerIndex = 0; playerIndex < players_count; playerIndex++)
    {
        if (playerIndex == bankruptPlayerIndex || players[playerIndex]->cash != max_cash) {
            continue;
        }

        // get this plater's properties price
        currentPlayer_property_price = 0;
        for (int propertyIndex = 0; propertyIndex < players[playerIndex]->num_properties; propertyIndex++) {
            currentPlayer_property_price += players[playerIndex]->owned_properties[propertyIndex]->price;
        }

        if (currentPlayer_property_price == max_property_price) {
            if (winner_num == -2)
            {
                winner_num = playerIndex;
            } else {    //there is more than 1 winner
                winner_num = -1;
            }
        }
    }

    return winner_num;
}


// frees all allocated memory
void freeDrawData(){
    // free window (ncurses) structures
    delwin(window_gameboard.frame);
    delwin(window_gameboard.interraction);
    for(int playerIndex = 0; playerIndex <  players_count; playerIndex++){
        delwin(window_gameboard.playerStates[playerIndex]);
    }
    for (int spaceIndex = 0; spaceIndex < space_count; spaceIndex++){
        delwin(window_gameboard.spaces[spaceIndex]);
    }
    free(window_gameboard.spaces);
    free(window_gameboard.playerStates);

    window_gameboard.frame = NULL;
    window_gameboard.playerStates = NULL;
    window_gameboard.spaces = NULL;
    window_gameboard.interraction = NULL;
}


// free custom gameboard data (which was read from files (not released by now))
void freeGameboard(FLAG flag_b, FLAG flag_p){
    if (flag_b.isOn){
        free(gameboard);
    }

    if (flag_p.isOn){
        free(properties);
    }
}


// free memory, allocated for players
void freePlayers(){
    for (int playerIndex = 0; playerIndex < players_count; playerIndex++)
    {
        free(players[playerIndex]);
    }
}


// --- Interface functions --- 


// vertically draws monopoly 6 rows x 62 cols on chosen window by cordinates y, x
void drawTextMonopoly(WINDOW * window, int y, int x){
    
    mvwprintw(window, y, x,    "  MMM   MMM   OOO   N   N   OOO   PPPP    OOO   L     YY   YY");
    mvwprintw(window, y+1, x,  "  MMMM MMMM  O   O  NN  N  O   O  P   P  O   O  L      YY YY ");
    mvwprintw(window, y+2, x,  "  MM MMM MM O     O N N N O     O PPPP  O     O L       YYY  ");
    mvwprintw(window, y+3, x,  "  MM  M  MM O     O N  NN O     O P     O     O L        Y   ");
    mvwprintw(window, y+4, x,  "  MM     MM  O   O  N   N  O   O  P      O   O  L        Y   ");
    mvwprintw(window, y+5, x,  "  MM     MM   OOO   N   N   OOO   P       OOO   LLLLL    Y   ");

    wrefresh(window);
}


void initDrawScreen(){
    initscr();
    cbreak();
    curs_set(0);

    if(has_colors() == FALSE)
	{	endwin();
		printf("Your terminal does not support color\n");
		exit(1);
	}
	start_color();

    init_pair(1, COLOR_YELLOW, COLOR_BLACK );       // brown
    init_pair(2, COLOR_CYAN , COLOR_BLACK );        // skyblue
    init_pair(3, COLOR_MAGENTA , COLOR_BLACK );     // purple
    init_pair(4, COLOR_RED , COLOR_BLACK );         // orange
    init_pair(5, COLOR_RED , COLOR_BLACK );         // red
    init_pair(6, COLOR_YELLOW , COLOR_BLACK );      // yellow
    init_pair(7, COLOR_GREEN , COLOR_BLACK );       // green
    init_pair(8, COLOR_BLUE , COLOR_BLACK );        // blue
    
    // init_pair(9, COLOR_YELLOW, COLOR_WHITE );       // brown
    // init_pair(10, COLOR_CYAN , COLOR_WHITE );        // skyblue
    // init_pair(11, COLOR_MAGENTA , COLOR_WHITE );     // purple
    // init_pair(12, COLOR_RED , COLOR_WHITE );         // orange

    // init_pair(0, COLOR_WHITE, COLOR_RED);       // brown
    // init_pair(1, COLOR_WHITE, COLOR_CYAN);      // skyblue
    // init_pair(2, COLOR_WHITE, COLOR_MAGENTA);   // purple
    // init_pair(3, COLOR_WHITE, COLOR_YELLOW);    // orange
    // init_pair(4, COLOR_WHITE, COLOR_RED);       // red
    // init_pair(5, COLOR_WHITE, COLOR_YELLOW);    // yellow
    // init_pair(6, COLOR_WHITE, COLOR_GREEN);     // green
    // init_pair(7, COLOR_WHITE, COLOR_BLUE);      // blue

    refresh();

}

void initDrawGameboard(int startY, int startX) {
    
    int maxX, maxY;
    getmaxyx(stdscr, maxY, maxX);  //TODO

    // space distribution
    int x_spaceCount, y_spaceCoordinate;

    y_spaceCoordinate = ((space_count+1)/2)/3+1;
    x_spaceCount = ((space_count+1)/2) - y_spaceCoordinate;

    //  create game state+input screen
    if (y_spaceCoordinate >= 3 && x_spaceCount >= 4){
        window_gameboard.interraction = newwin(SPACE_HEIGHT, 2*SPACE_WIDTH, startY+(y_spaceCoordinate-1)*SPACE_HEIGHT+2, startX+(x_spaceCount/2)*SPACE_WIDTH+2); //TODO: make readebale better
        // set main frame
        window_gameboard.frame = newwin((y_spaceCoordinate+1)*SPACE_HEIGHT+4, (x_spaceCount+1)*SPACE_WIDTH+4, startY, startX);
    } else {
        window_gameboard.interraction = newwin(SPACE_HEIGHT, 2*SPACE_WIDTH, startY+(1)*SPACE_HEIGHT+1, startX+(x_spaceCount)*SPACE_WIDTH+1);
        // set main frame
        window_gameboard.frame = newwin((y_spaceCoordinate+1)*SPACE_HEIGHT+4, (x_spaceCount+2)*SPACE_WIDTH+4+1, startY, startX);
    }


    // create board (spaces)
    window_gameboard.spaces = (WINDOW **) malloc(space_count*sizeof(WINDOW *));
    if (window_gameboard.spaces == NULL) {exit(1);}

    startX +=2;
    startY +=2;

    int timesY = 0, timesX = 0;
    for (; timesX < x_spaceCount; timesX++){
        window_gameboard.spaces[timesX+timesY] = newwin(SPACE_HEIGHT, SPACE_WIDTH, startY+timesY*SPACE_HEIGHT, startX+timesX*SPACE_WIDTH);
    }
    for (; timesY < y_spaceCoordinate; timesY++){
        window_gameboard.spaces[timesX+timesY] = newwin(SPACE_HEIGHT, SPACE_WIDTH, startY+timesY*SPACE_HEIGHT, startX+timesX*SPACE_WIDTH);
    }

    // handle uneven count of spaces
    int temp_offset = 0;
    if (space_count % 2 == 1){
        temp_offset = SPACE_WIDTH/2;
        timesX--;
    }

    for (; timesX > 0; timesX--){
        window_gameboard.spaces[space_count - (timesX+timesY)] = newwin(SPACE_HEIGHT, SPACE_WIDTH, startY+timesY*SPACE_HEIGHT, startX+temp_offset+timesX*SPACE_WIDTH);
    }

    window_gameboard.spaces[space_count - (timesX+timesY)] = newwin(SPACE_HEIGHT, SPACE_WIDTH, startY+timesY*SPACE_HEIGHT, startX+temp_offset+timesX*SPACE_WIDTH);
    timesY--;
    temp_offset = 0;

    for (; timesY > 0; timesY--){
        window_gameboard.spaces[space_count - (timesX+timesY)] = newwin(SPACE_HEIGHT, SPACE_WIDTH, startY+timesY*SPACE_HEIGHT, startX+timesX*SPACE_WIDTH);
    }


    // create playerStates
    window_gameboard.playerStates = (WINDOW **) malloc(players_count*sizeof(WINDOW *));
    if (window_gameboard.playerStates == NULL) { exit(1); }

    temp_offset = 0;
    if (players_count > 3)
    {
        window_gameboard.playerStates[3] = newwin(SPACE_HEIGHT, SPACE_WIDTH, startY+(y_spaceCoordinate-1)*SPACE_HEIGHT, startX+2*SPACE_WIDTH);
    }
    if (players_count > 2)
    {
        window_gameboard.playerStates[2] = newwin(SPACE_HEIGHT, SPACE_WIDTH, startY+(y_spaceCoordinate-1)*SPACE_HEIGHT, startX+1*SPACE_WIDTH);
        temp_offset = 1;
    }
    if (players_count > 1)
    {
        window_gameboard.playerStates[1] = newwin(SPACE_HEIGHT, SPACE_WIDTH, startY+(y_spaceCoordinate-1-temp_offset)*SPACE_HEIGHT, startX+2*SPACE_WIDTH);
    }

    window_gameboard.playerStates[0] = newwin(SPACE_HEIGHT, SPACE_WIDTH, startY+(y_spaceCoordinate-1-temp_offset)*SPACE_HEIGHT, startX+1*SPACE_WIDTH);

}

// draws players on a gameboard
void drawPlayers() {
    int offset;
    WINDOW * currentWindow;

    for (int spaceIndex = 0; spaceIndex < space_count; spaceIndex++) {
        currentWindow = window_gameboard.spaces[spaceIndex];    // better for code readability

        mvwhline(currentWindow, 4, 1, ' ', SPACE_WIDTH-2);      //clear 
        mvwprintw(currentWindow, 4, 2, "");                     //set cursor pos

        for (int playerIndex = 0; playerIndex < players_count; playerIndex++) { // write players on this space
            if (players[playerIndex]->space_number == spaceIndex){
                wattron(currentWindow, A_BOLD | COLOR_PAIR(playerIndex+1));
                wprintw(currentWindow, "P%d ", playerIndex);
            }
        }

        wattrset(currentWindow, ACS_NEQUAL);
        wrefresh(currentWindow);
    }
}


// draw gameboard
void drawBoard(){

    // draw direction arrow
    box(window_gameboard.frame, ACS_VLINE, ACS_HLINE);
    mvwhline(window_gameboard.frame, 1, 2, ACS_HLINE, 7);
    mvwaddch(window_gameboard.frame, 1, 9, ACS_RARROW);
    wrefresh(window_gameboard.frame);

    // draw name
    drawTextMonopoly(window_gameboard.frame, SPACE_HEIGHT+4, SPACE_WIDTH*4-62/2);

    // draw spaces                                                
    for (int spaceIndex = 0; spaceIndex < space_count; spaceIndex++){
        WINDOW * window = window_gameboard.spaces[spaceIndex];
        SPACE currentSpace = gameboard[spaceIndex];
        // wclear(window);
        wattrset(window, A_BOLD);
        box(window, ACS_VLINE, ACS_HLINE);

        //header

        if (currentSpace.type == Property){

            wattrset(window, A_NORMAL);
            wattron(window, A_BOLD | COLOR_PAIR(currentSpace.property->color+1));

            switch (currentSpace.property->color)
            {
            case Brown:
                wattron(window, A_DIM);
                break;

            case Red:
                wattron(window, A_DIM);
                break;

            default:
                break;
            }

            mvwhline(window, 1, 1, ACS_CKBOARD, SPACE_WIDTH-2);
            // mvwhline(window, 2, 1, ACS_CKBOARD, SPACE_WIDTH-2);
            mvwhline(window, 3, 1, ACS_CKBOARD, SPACE_WIDTH-2);

            wattrset(window, COLOR_PAIR(0) | A_BOLD);
            mvwprintw(window, 2, (SPACE_WIDTH - strlen(currentSpace.property->name))/2, "%s", currentSpace.property->name);


            //footer
            wattrset(window, A_NORMAL);
            if (currentSpace.property->ownerIndex == -1){
                mvwprintw(window, SPACE_HEIGHT-3, 2, "No owner");
            } else {
                mvwprintw(window, SPACE_HEIGHT-3, 2, "Owner: ");
                wattrset(window, COLOR_PAIR(currentSpace.property->ownerIndex+1) | A_ITALIC | A_BOLD);
                wprintw(window, "P%d  ", currentSpace.property->ownerIndex);
                wattrset(window, A_NORMAL);
                wprintw(window, "M:%d", currentSpace.property->isMonopoly);
            }
            mvwprintw(window, SPACE_HEIGHT-2, 2, "Price: %d$", currentSpace.property->price);

        } else {
            mvwprintw(window, 2, (SPACE_WIDTH - strlen(space_types[currentSpace.type]))/2, "%s", space_types[currentSpace.type]);
        }

        wrefresh(window);
    }

    // draw player states boxes
    for (int playerIndex = 0; playerIndex < players_count; playerIndex++){
        WINDOW * window = window_gameboard.playerStates[playerIndex];
        PLAYER * currentPlayer = players[playerIndex];
        wclear(window);

        wattrset(window, A_BOLD);
        if (playerIndex == playerTurnIndex){
            box(window, ' ', ' ');
        }

        wattrset(window, COLOR_PAIR(playerIndex+1) | A_BOLD);
        mvwprintw(window, 1, (SPACE_WIDTH - 10)/2, " Player %d ", playerIndex);


        wattrset(window, A_NORMAL);

        mvwhline(window, 2, 1, ACS_HLINE, SPACE_WIDTH-2);
        // mvwaddch(window, 2, 0, ACS_LTEE);
        // mvwaddch(window, 2, SPACE_WIDTH-1, ACS_RTEE);

        mvwprintw(window, 3, 2, "Position: %d", currentPlayer->space_number);
        mvwprintw(window, 4, 2, "Cash: %d", currentPlayer->cash);
        mvwprintw(window, 5, 2, "JP count: %d", currentPlayer->num_jail_pass);
        mvwprintw(window, 6, 2, "Bot: %d", currentPlayer->isBot);
        // mvwprintw(window, 7, 2, "Position: %d", currentPlayer->);

        wrefresh(window);
    }
}

void drawResults(int bankruptIndex){
    drawBoard();
    drawPlayers();

    int winnerindex = findWinner(bankruptIndex);

    // write winner to interraction window
    wclear(window_gameboard.interraction);
    wattrset(window_gameboard.interraction, COLOR_PAIR(winnerindex+1) | A_BOLD | A_BLINK);
    box(window_gameboard.interraction, ACS_VLINE, ACS_HLINE);
    wrefresh(window_gameboard.interraction);

    // draw winner prompt
    wattrset(window_gameboard.interraction, A_BOLD);
    mvwprintw(window_gameboard.interraction, 2, (SPACE_WIDTH-12/2), "WINNER:");
    wattron(window_gameboard.interraction, COLOR_PAIR(winnerindex+1));
    if (winnerindex >= 0){
        wprintw(window_gameboard.interraction, " P%d", winnerindex);
    } else {
        wprintw(window_gameboard.interraction, " P?");
    }

    // draw exit options
    wattrset(window_gameboard.interraction, A_NORMAL);
    mvwprintw(window_gameboard.interraction, 4, (SPACE_WIDTH-30/2), "[Press <m> to return to menu]");
    mvwprintw(window_gameboard.interraction, 5, (SPACE_WIDTH-30/2), "[Press <q> to quit the game ]");

    wrefresh(window_gameboard.interraction);

    while(1){
        switch (getchar())
        {
        case 'q':
            game_state = eol;
            return;
            break;

        case 'm':
            game_state = menu;
            return;
            break;
        
        default:
            break;
        }
    };
};


// get dice value for current player (playerTurnIndex)
void drawInputDiceValue(int * dice){
    wattrset(window_gameboard.interraction, A_NORMAL);

    // draw input box
    wclear(window_gameboard.interraction);
    box(window_gameboard.interraction, ACS_VLINE, ' ');
    wrefresh(window_gameboard.interraction);

    if (players[playerTurnIndex]->isBot == 1){
        *dice = randint(0, 7);

        // write dice prompt
        wattrset(window_gameboard.interraction, A_BOLD);
        mvwprintw(window_gameboard.interraction, 2, (SPACE_WIDTH-26/2), "Player on turn (BOT):");
        wattron(window_gameboard.interraction, COLOR_PAIR(playerTurnIndex+1));
        wprintw(window_gameboard.interraction, " P%d", playerTurnIndex);
        wattroff(window_gameboard.interraction, COLOR_PAIR(playerTurnIndex+1));
        mvwprintw(window_gameboard.interraction, 4, (SPACE_WIDTH-9/2), "DICE: %d", *dice);

        // draw hint
        wattrset(window_gameboard.interraction, A_NORMAL);
        mvwprintw(window_gameboard.interraction, 6, (SPACE_WIDTH-31/2), "[Press any button to continue]");

        wrefresh(window_gameboard.interraction);
        getchar();

    } else {

        // write dice prompt
        mvwprintw(window_gameboard.interraction, 2, (SPACE_WIDTH-10/2), "Your Turn");
        wattrset(window_gameboard.interraction, A_BOLD);
        mvwprintw(window_gameboard.interraction, 3, (SPACE_WIDTH-19/2), "Enter dice value: ");

        //  input
        curs_set(2);
        echo();

        char temp;
        while (1) {
            *dice = -1;

            wmove(window_gameboard.interraction, 4, (SPACE_WIDTH-1));
            temp = wgetch(window_gameboard.interraction);

            mvwhline(window_gameboard.interraction, 5, 1, ' ', 2*SPACE_WIDTH - 2);
            wattrset(window_gameboard.interraction, COLOR_PAIR(Red));

            // incorrect input handling
            if (isdigit(temp)){
                *dice = (int)(temp - '0');
                if (*dice >= 0 && *dice <= 6) {
                    wattrset(window_gameboard.interraction, A_NORMAL);
                    mvwhline(window_gameboard.interraction, 4, 1, ' ', 2*SPACE_WIDTH - 2);
                    break;      // user entered right input (number 1-6)
                } else {
                    mvwprintw(window_gameboard.interraction, 5, SPACE_WIDTH-13, "*Enter value from 0 to 6");
                }
            } else {
                mvwprintw(window_gameboard.interraction, 5, SPACE_WIDTH-11, "*Enter a NUMBER value");
            }

            wattrset(window_gameboard.interraction, A_BOLD);
            mvwhline(window_gameboard.interraction, 4, 1, ' ', 2*SPACE_WIDTH - 2);
        }

        curs_set(0);
        noecho();
    }
};



void drawMenu(){
    char temp;

    //  draw MONOPOLY prompt
    attron(A_BOLD | COLOR_PAIR(Yellow+1));
    drawTextMonopoly(stdscr, 4, 8);
    attroff(A_BOLD| COLOR_PAIR(0));

    // create and draw manu window
    WINDOW * menu = newwin(SPACE_HEIGHT*2, SPACE_WIDTH*2, SPACE_HEIGHT*2, SPACE_WIDTH);
    wattron(menu, A_BOLD);
    box(menu, ' ', ' ');
    wattroff(menu, A_BOLD);

    // draw hints
    wattrset(menu, COLOR_PAIR(Green+1) | A_BOLD);
    mvwprintw(menu, 2, 4, "Press <p> to p PLAY the game");
    wattrset(menu, COLOR_PAIR(0));
    mvwprintw(menu, 3, 4, "Press <c> to set player COUNT");
    mvwprintw(menu, 4, 4, "Press <y> to set YOUR PLAYER");
    wattrset(menu, COLOR_PAIR(Yellow+1));
    mvwprintw(menu, 5, 4, "Press <space> to do nothing");
    wattrset(menu, COLOR_PAIR(Red+1));
    mvwprintw(menu, 6, 4, "Press <q> to QUIT the game");

    // draw players
    wattrset(menu, A_NORMAL);
    mvwprintw(menu, 8, 4, "Players (%d):", players_count);
    for (int playerIndex = 0; playerIndex < players_count; playerIndex++){
        wattrset(menu, COLOR_PAIR(playerIndex+1));
        mvwprintw(menu, 9+playerIndex, 6, "Player%d: ", playerIndex);
        wattrset(menu, A_NORMAL);
        if (playerIndex == notBotIndex){ wattrset(menu, A_BOLD);  wprintw(menu, "<-- YOU"); } else {wprintw(menu, "BOT"); }
    }
    if (notBotIndex == -1){
        mvwprintw(menu, 9+players_count, 5, "You don't take part");
    }

    // refresh
    refresh();
    wrefresh(menu);

    // get input
    temp = getch();
    switch (temp)
    {
    case 'q':
        game_state = eol;
        break;
    
    case 'g':
        game_state = game_start;
        break;

    case 'c':

        //  redraw menu as input field
        wclear(menu);
        wattron(menu, A_BOLD);
        box(menu, ' ', ' ');

        // prompt
        mvwprintw(menu, 5, (SPACE_WIDTH-19/2), "Choose Player count");
        wattroff(menu, A_BOLD);
        mvwprintw(menu, 6, (SPACE_WIDTH-25/2), "(enter number <2-4>)");

        curs_set(2);
        echo();

        //  input + error handling
        char temp2;
        while (1) {
            
            wmove(menu, 8, (SPACE_WIDTH-1));
            temp2 = wgetch(menu);

            mvwhline(menu, 12, 1, ' ', 2*SPACE_WIDTH - 2);
            wattrset(menu, COLOR_PAIR(Red));

            if (isdigit(temp2)){
                if (temp2 > '1' && temp2 < '0' + players_count) {
                    wattrset(menu, A_NORMAL);
                    mvwhline(menu, 10, 1, ' ', 2*SPACE_WIDTH - 2);

                    players_count = temp2 - '0';
                    break;      // user entered right input (number 1-pC)
                } else {
                    mvwprintw(menu, 12, SPACE_WIDTH-13, "*Enter value from 2 to 4");
                }
            } else {
                mvwprintw(menu, 12, SPACE_WIDTH-11, "*Enter a VALID value");
            }

            wattrset(menu, A_BOLD);
            mvwhline(menu, 10, 1, ' ', 2*SPACE_WIDTH - 2);
        }

        curs_set(0);
        noecho();
        break;
    
    case 'y':

        //  redraw menu as input field
        wclear(menu);
        wattron(menu, A_BOLD);
        box(menu, ' ', ' ');

        // prompt
        mvwprintw(menu, 5, (SPACE_WIDTH-19/2), "Choose your player");
        wattroff(menu, A_BOLD);
        mvwprintw(menu, 6, (SPACE_WIDTH-25/2), "(enter number <0-%d>, or", players_count-1);
        mvwprintw(menu, 7, (SPACE_WIDTH-30/2), "<n> to not take part in game)");

        // input + error handling
        curs_set(2);
        echo();

        char temp1;
        while (1) {
            wmove(menu, 10, (SPACE_WIDTH-1));
            temp2 = wgetch(menu);

            mvwhline(menu, 12, 1, ' ', 2*SPACE_WIDTH - 2);
            wattrset(menu, COLOR_PAIR(Red));

            if (isdigit(temp2)){
                if (temp2 >= '0' && temp2 < '0' + players_count) {
                    wattrset(menu, A_NORMAL);
                    mvwhline(menu, 10, 1, ' ', 2*SPACE_WIDTH - 2);

                    notBotIndex = temp2 - '0';
                    break;      // user entered right input (number 1-pC)
                } else {
                    mvwprintw(menu, 12, SPACE_WIDTH-13, "*Enter value from 0 to %d", players_count-1);
                }
            } else {
                if (temp2 == 'n'){
                    wattrset(menu, A_NORMAL);
                    mvwhline(menu, 10, 1, ' ', 2*SPACE_WIDTH - 2);

                    notBotIndex = -1;
                    break;      // user entered right input (number 1-pC)}
                }
                mvwprintw(menu, 12, SPACE_WIDTH-11, "*Enter a VALID value");
            }

            wattrset(menu, A_BOLD);
            mvwhline(menu, 10, 1, ' ', 2*SPACE_WIDTH - 2);
        }

        curs_set(0);
        noecho();
        break;

    case ' ':
        wclear(menu);
        wattron(menu, A_BOLD);
        box(menu, ' ', ' ');

        mvwprintw(menu, 4, SPACE_WIDTH - 8/2, "Well...");
        mvwprintw(menu, 6, SPACE_WIDTH - 23/2, "Do nothing %ds (Wait).", waitTimeJK);
        mvwprintw(menu, 7, SPACE_WIDTH - 32/2, "Next time you'd be waitimg %ds.", waitTimeJK+1);
        mvwprintw(menu, 9, SPACE_WIDTH - 3/2, "...", waitTimeJK+3);
        wattroff(menu, A_BOLD);
        mvwprintw(menu, 13, SPACE_WIDTH - 35/2, "Yea, there IS mistake in 'waiting'", waitTimeJK+1);

        wrefresh(menu);

        sleep(waitTimeJK);
        waitTimeJK+=3;
        break;

    default:
        break;
    }

    wclear(menu);
    delwin(menu);
}


// --- Game Function --- //
void aGame(int argc, char * argv[]){
    FLAG flag_board = {0, NULL}, flag_properties = {0, NULL};   // TODO
    // getOptions(argc, argv, &flag_board, &flag_properties); // +FLAGS

    srandom(time(NULL));    // initialize random

    // init game settings
    waitTimeJK = 3;
    players_count = 4;
    notBotIndex = -1;

    // init UI
    initDrawScreen();

    // game loop
    game_state = menu;
    int dice, bankruptPlayerIndex;

    while (1) {
        // clear();

        switch (game_state)
        {
        case menu:
            drawMenu();

            break;
        case game_start:
            if (initGameboard(flag_board, flag_properties) != 0) {
                game_state = eol;
                break;
            }
            initPlayers();
            playerTurnIndex = 0;

            initDrawGameboard(1, 1);

            game_state = game_process;
            break;

        case game_process:

            drawBoard();
            drawPlayers();

            drawInputDiceValue(&dice); // wait untill input dice/enter

            if(movePlayer(dice) == 0){
                bankruptPlayerIndex = playerTurnIndex;
                game_state = game_end;
                break;
            }

            playerTurnIndex++;
            if (playerTurnIndex >= players_count) {
                playerTurnIndex = 0;
            }

            break;

        case game_end:
            
            drawResults(bankruptPlayerIndex); //handles game_state change

            // free allocated game memory
            freeDrawData();
            freeGameboard(flag_board, flag_properties);
            freePlayers();

            break;

        case eol:
            endwin();
            return;
            break;

        default:
            break;
        }
        // refresh();
    }

}

int main(int argc, char *argv[])
{
    aGame(argc, argv);

    return 0;
}

//  TODO: 
//      rules/help page
//      other TODOs