# MONOPOLY
**by Ostap Pelekh, STU FEI, Applied Informatics, 1.bc**

This is monopoly game with Text-based UI (ncurses.h)



## RULES

### OBJECTIVE OF THE GAME
Become the wealthiest player through buying and renting property and force other players into bankruptcy.

### START
Each time a player passes over START the player is paid a $2 salary.

### BUYING PROPERTY
When a player lands on an unowned property they have to buy that property at its printed price.

### PAYING RENT
When a player lands on a property owned by another player, the owner collects rent.

### JAIL
A player can be sent to jail when they land on "GO TO JAIL".
The player has to pay 1$ to get out of jail.
If the player doesn't have 1$ to pay he will bankrupt and the game ends.

### FREE PARKING
Just a free location with no reward or pentaly.

### MONOPOLY
If the player has all properties of the same color he has a monopoly and receives double rent.

### BANKRUPTCY
When the player has 0 cash they are bankrupt and out.


## GAMEPLAY 

You can choose number of players and Your player.
Dice values for each player (except for Your player) are generated randomly. You will have to write a dice value for your player manually.

Follow game prompts and instructions.


## NOTES 

**COMPILE with -lncurses argument**
Example:

    gcc -lncurses main.c

<ctrl+c> to exit game immediately
