/**
 * @file BopIt.c
 *
 * @brief Keep track of game state and handle commands and actions
 *
 ******************************************************************************/

/* Includes
 ******************************************************************************/
#include "BopIt.h"
#include <stddef.h>
#include <stdlib.h>
#include <time.h>

/* Defines
 ******************************************************************************/

#define BOPIT_MAX_SCORE 99U
#define BOPIT_INIT_LIVES 3U
#define BOPIT_MAX_WAIT_TIME_MS 5000U
#define BOPIT_MIN_WAIT_TIME_MS 500U
#define BOPIT_WAIT_TIME_DECREMENT_MS (BOPIT_MAX_WAIT_TIME_MS - BOPIT_MIN_WAIT_TIME_MS) / (BOPIT_MAX_SCORE)

/* Function Prototypes
 ******************************************************************************/

static void BopIt_Log(const char *const message);
static BopIt_TimeMs_t BopIt_GetTime();
static BopIt_TimeMs_t BopIt_GetElapsedTime(const BopIt_TimeMs_t startTime);
static BopIt_Command_t *BopIt_GetRandomCommand(const BopIt_Command_t *const commands, const uint32_t commandCount);

/* Function Definitions
 ******************************************************************************/

void BopIt_Init(BopIt_GameContext_t *const gameContext)
{
    if (gameContext != NULL)
    {
        gameContext->GameState = INIT;
        gameContext->Lives = BOPIT_INIT_LIVES;
        gameContext->WaitTime = BOPIT_MAX_WAIT_TIME_MS;

        srand(time(NULL));
    }
}

void BopIt_Run(BopIt_GameContext_t *const gameContext)
{
    if (gameContext != NULL)
    {
        switch (gameContext->GameState)
        {
        case INIT:
            /* TODO configure game */
            gameContext->GameState = START;
            break;
        case START:
            /* TODO start game */
            BopIt_Log("Starting game");
            gameContext->GameState = COMMAND;
            break;
        case COMMAND:
            /* TODO log score and lives remaining */

            BopIt_Log("Issuing command");
            gameContext->CurrentCommand = BopIt_GetRandomCommand(gameContext->Commands, gameContext->CommandCount);
            gameContext->CurrentCommand->IssueCommand();

            BopIt_Log("Waiting for player action");
            gameContext->WaitStart = BopIt_GetTime();
            gameContext->GameState = WAIT;

            break;
        case WAIT:
            /* TODO check for inputs */

            if (BopIt_GetElapsedTime(gameContext->WaitStart) > gameContext->WaitTime)
            {
                BopIt_Log("Out of time");
                gameContext->GameState = FAIL;
            }
            /* TODO check input for success */
            break;
        case SUCCESS:
            BopIt_Log("Player action success");
            gameContext->CurrentCommand->SuccessFeedback();
            gameContext->Score++;
            if (gameContext->Score < BOPIT_MAX_SCORE)
            {
                gameContext->WaitTime -= BOPIT_WAIT_TIME_DECREMENT_MS;
                gameContext->GameState = COMMAND;
            }
            else
            {
                gameContext->GameState = END;
            }
            break;
        case FAIL:
            BopIt_Log("Player action fail");
            gameContext->CurrentCommand->FailFeedback();
            gameContext->Lives--;
            if (gameContext->Lives == 0U)
            {
                gameContext->GameState = END;
            }
            break;
        case END:
            BopIt_Log("Game over");
            /* TODO log score and lives remaining */
            /* TODO end game */
            break;
        case DEINIT:
            /* final state */
            break;
        default:
            break;
        }
    }
}

static BopIt_Command_t *BopIt_GetRandomCommand(const BopIt_Command_t *const commands, const uint32_t commandCount)
{
    BopIt_Command_t *command = NULL;

    if (commands != NULL)
    {
        command = (BopIt_Command_t *)(commands + (rand() % commandCount));
    }

    return command;
}
