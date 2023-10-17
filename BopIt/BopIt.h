/**
 * @file BopIt.h
 *
 * @brief Keep track of game state and handle commands and actions
 *
 ******************************************************************************/

/* Includes
 ******************************************************************************/
#include <stdint.h>

/* Typedefs
 ******************************************************************************/

typedef uint32_t BopIt_TimeMs_t; /* Time in milliseconds */

typedef enum
{
    INIT,
    START,
    COMMAND,
    WAIT,
    SUCCESS,
    FAIL,
    END,
    DEINIT
} BopIt_GameState_t;

typedef struct
{
    void (*IssueCommand)(void);
    void (*SuccessFeedback)(void);
    void (*FailFeedback)(void);
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
} BopIt_GameContext_t;

/* Function Prototypes
 ******************************************************************************/

void BopIt_Init(BopIt_GameContext_t *const gameContext);
void BopIt_Run(BopIt_GameContext_t *const gameContext);