#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <ncurses.h>


#define DEF_SPACE_COUNT 24
#define DEF_PROP_COUNT 16
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

PROPERTY * properties = NULL;
int properties_count;

PLAYER * players[MAX_PLAYERS] = {};
int player_count, playerIndex;

ENDPOINT game_state;


// const values definition
const char *space_types[NUM_SPACE_TYPES] = {
        "PROPERTY",
        "START",
        "FREE PARKING",
        "IN JAIL",
        "GO TO JAIL",
        "JAIL PASS"
};

// nazvy farieb nehnutelnosti
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

PROPERTY def_properties[DEF_PROP_COUNT] = {
        {.name = "FOOD TRUCK", .price = 1, .color = Brown},
        {.name = "PIZZA RESTAURANT", .price = 1, .color = Brown},
        {.name = "DOUGHNUT SHOP", .price = 1, .color = Skyblue},
        {.name = "ICE CREAM SHOP", .price = 1, .color = Skyblue},
        {.name = "MUSEUM", .price = 2, .color = Purple},
        {.name = "LIBRARY", .price = 2, .color = Purple},
        {.name = "THE PARK", .price = 2, .color = Orange},
        {.name = "THE BEACH", .price = 2, .color = Orange},
        {.name = "POST OFFICE", .price = 3, .color = Red},
        {.name = "TRAIN STATION", .price = 3, .color = Red},
        {.name = "COMMUNITY GARDEN", .price = 3, .color = Yellow},
        {.name = "PET RESCUE", .price = 3, .color = Yellow},
        {.name = "AQUARIUM", .price = 4, .color = Green},
        {.name = "THE ZOO", .price = 4, .color = Green},
        {.name = "WATER PARK", .price = 5, .color = Blue},
        {.name = "AMUSEMENT PARK", .price = 5, .color = Blue}
};

// inicializacia hracieho pola
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

// initialise players, gameboard, properties and global variables
int initGame(FLAG flag_b, FLAG flag_t){
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
            // exit(1);
            return 1;
        }
    }
    if (flag_t.isOn == 1){
        file_t = fopen(flag_t.argument, "r");
        if (file_t ==  NULL){
            printf("Error opening the file \"%s\"\n", flag_t.argument);
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
        player_count = 0;

        while (feof(file_t) == 0){
            memset(temp_row, 0, MAX_NAME-1);
            fgets(temp_row, MAX_NAME, file_t);
            if (temp_row[0] == '"')
            {
                (player_count)++;
            }
        }
    } else {
        player_count = DEF_PROP_COUNT;
    }

    if (needed_properties_count > (player_count)){
        // printf("Not enough properties in %s\n", path_t);
        // exit(1);
        return 2;
    }

    //fill gameboard and proerties
    int temp_nameLength = 0, temp_price = 0;
    COLOR temp_color = 0;
    SPACE_TYPE temp_propType;

    if (flag_t.isOn){
        properties = (PROPERTY *) calloc((player_count), sizeof(PROPERTY));
        
        rewind(file_t);

        for (int propertyIndex = 0; propertyIndex < player_count; ){
            
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
        gameboard = (SPACE *) calloc(space_count, sizeof(SPACE));//(space_count), sizeof(game_board)/NUM_SPACES);
        
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

void drawBoard(){

};

void drawResults(){

};

void initPlayers(){

    for (int playerNumber = 0; playerNumber < player_count; playerNumber++)
    {
        for (int i = 0; i < player_count; i++)
        {
            players[i] = calloc(1, sizeof(PLAYER));

            players[i]->index = i;
            players[i]->space_number = 0;
            switch (player_count)
            {
            case 2:
                players[i]->cash = 20;
                break;
            
            case 3:
                players[i]->cash = 18;
                break;
            
            case 4:
                players[i]->cash = 16;
                break;
            
            default:
                break;
            }

            
            players[i]->num_jail_pass = 0;
            players[i]->is_in_jail = 0;
            players[i]->num_properties = 0;

        }
        
        
    }
    
    return;
}

void moveNextPlayer(){

};

void Game(int argc, char * argv[]){

    FLAG flag_board = {0, NULL}, flag_properties = {0, NULL};

    getOptions(argc, argv, &flag_board, &flag_properties); // +FLAGS

    game_state = game_start;

    while (1) {
        switch (game_state)
        {
        case game_start:
            initPlayers();
            initGame(flag_board, flag_properties);

            game_state = eol;
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
            break;

        default:
            break;
        }
    }

}

int main(int argc, char *argv[])
{
    Game(argc, argv);

    return 0;
}

