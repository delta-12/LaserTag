/**
 * @file BopIt.h
 *
 * @brief Keep track of game state and handle commands and actions.
 *
 ******************************************************************************/

/* Includes
 ******************************************************************************/
#include <stdbool.h>
#include <stdint.h>

/* Typedefs
 ******************************************************************************/

typedef uint32_t BopIt_TimeMs_t; /* Time in milliseconds */

/* BopIt game states */
typedef enum
{
    BOPIT_GAMESTATE_START,   /* Start game */
    BOPIT_GAMESTATE_COMMAND, /* Select and issue a command */
    BOPIT_GAMESTATE_WAIT,    /* Wait for input from the player */
    BOPIT_GAMESTATE_SUCCESS, /* Player successfully completed the selected command */
    BOPIT_GAMESTATE_FAIL,    /* Player fail to completed the selected command */
    BOPIT_GAMESTATE_END,     /* Game is over */
} BopIt_GameState_t;

/* Command for player */
typedef struct
{
    const char *Name;              /* Name of the command */
    void (*IssueCommand)(void);    /* Perform all tasks needed to issue the command */
    void (*SuccessFeedback)(void); /* Provide feedback for successfully completing the command */
    void (*FailFeedback)(void);    /* Provide feedback for failing to complete the command */
    bool (*GetInput)(void);        /* Check if the player made the input corresponding to the command */
} BopIt_Command_t;

typedef struct BopIt_GameContext BopIt_GameContext_t; /* Context for a BopIt game, manages game state */
struct BopIt_GameContext
{
    BopIt_GameState_t GameState;                                 /* State of BopIt game */
    uint8_t Score;                                               /* Player score */
    uint8_t Lives;                                               /* Remaining player lives */
    BopIt_Command_t **Commands;                                  /* List of possible commands the game can issue to player */
    uint32_t CommandCount;                                       /* Number of possible game commands */
    BopIt_Command_t *CurrentCommand;                             /* Command currently issued to player */
    BopIt_TimeMs_t WaitTime;                                     /* Time player has to complete command currently issued */
    BopIt_TimeMs_t WaitStart;                                    /* Time at which the current command was issued */
    void (*OnGameStart)(BopIt_GameContext_t *const gameContext); /* Callback executed on game start */
    void (*OnGameEnd)(BopIt_GameContext_t *const gameContext);   /* Callback executed on game end */
};

/* Function Prototypes
 ******************************************************************************/

void BopIt_RegisterLogger(void (*logger)(const char *const message));
void BopIt_RegisterTime(BopIt_TimeMs_t (*time)(void));
void BopIt_Init(BopIt_GameContext_t *const gameContext);
void BopIt_Run(BopIt_GameContext_t *const gameContext);
