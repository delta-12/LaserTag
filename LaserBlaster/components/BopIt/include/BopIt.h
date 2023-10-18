/**
 * @file BopIt.h
 *
 * @brief Keep track of game state and handle commands and actions
 *
 ******************************************************************************/

/* Includes
 ******************************************************************************/
#include <stdbool.h>
#include <stdint.h>

/* Typedefs
 ******************************************************************************/

typedef uint32_t BopIt_TimeMs_t; /* Time in milliseconds */

typedef enum
{
    BOPIT_GAMESTATE_START,
    BOPIT_GAMESTATE_COMMAND,
    BOPIT_GAMESTATE_WAIT,
    BOPIT_GAMESTATE_SUCCESS,
    BOPIT_GAMESTATE_FAIL,
    BOPIT_GAMESTATE_END,
} BopIt_GameState_t;

typedef struct
{
    void (*IssueCommand)(void);
    void (*SuccessFeedback)(void);
    void (*FailFeedback)(void);
    void (*UpdateInput)(void);
    bool Detected;
} BopIt_Command_t;

typedef struct
{
    BopIt_GameState_t GameState;
    uint8_t Score;
    uint8_t Lives;
    BopIt_Command_t *Commands;
    uint32_t CommandCount;
    BopIt_Command_t *CurrentCommand;
    BopIt_TimeMs_t WaitTime;
    BopIt_TimeMs_t WaitStart;
    void (*OnGameStart)(BopIt_GameContext_t *const gameContext);
    void (*OnGameEnd)(BopIt_GameContext_t *const gameContext);
} BopIt_GameContext_t;

/* Function Prototypes
 ******************************************************************************/

void BopIt_RegisterLogger(void (*logger)(const char *const message));
void BopIt_RegisterTime(BopIt_TimeMs_t (*time)());
void BopIt_Init(BopIt_GameContext_t *const gameContext);
void BopIt_Run(BopIt_GameContext_t *const gameContext);
