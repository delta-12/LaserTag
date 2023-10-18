/**
 * @file BopIt.c
 *
 * @brief Keep track of game state and handle commands and actions
 *
 ******************************************************************************/

/* Includes
 ******************************************************************************/
#include "BopIt.h"
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Defines
 ******************************************************************************/

#ifndef BOPIT_LOG_BUFFER_SIZE
#define BOPIT_LOG_BUFFER_SIZE 1024U
#endif
#define BOPIT_MAX_SCORE 99U
#define BOPIT_INIT_LIVES 3U
#define BOPIT_MAX_WAIT_TIME_MS 5000U
#define BOPIT_MIN_WAIT_TIME_MS 500U
#define BOPIT_WAIT_TIME_DECREMENT_MS (BOPIT_MAX_WAIT_TIME_MS - BOPIT_MIN_WAIT_TIME_MS) / (BOPIT_MAX_SCORE)

static char BopIt_LogBuffer[BOPIT_LOG_BUFFER_SIZE];

static void (*BopIt_Logger)(const char *const message) = NULL;
static BopIt_TimeMs_t (*BopIt_Time)() = NULL;

/* Function Prototypes
 ******************************************************************************/

static void BopIt_Log(const char *const format, ...);
static BopIt_TimeMs_t BopIt_GetTime();
static BopIt_TimeMs_t BopIt_GetElapsedTime(const BopIt_TimeMs_t startTime);
static BopIt_Command_t *BopIt_GetRandomCommand(const BopIt_Command_t *const commands, const uint32_t commandCount);
static void BopIt_HandleInit(BopIt_GameContext_t *const gameContext);
static void BopIt_HandleStart(BopIt_GameContext_t *const gameContext);
static void BopIt_HandleCommand(BopIt_GameContext_t *const gameContext);
static void BopIt_HandleWait(BopIt_GameContext_t *const gameContext);
static void BopIt_HandleSuccess(BopIt_GameContext_t *const gameContext);
static void BopIt_HandleFail(BopIt_GameContext_t *const gameContext);
static void BopIt_HandleEnd(BopIt_GameContext_t *const gameContext);
static void BopIt_HandleDeinit(BopIt_GameContext_t *const gameContext);

/* Function Definitions
 ******************************************************************************/

void BopIt_RegisterLogger(void (*logger)(const char *const message))
{
    if (logger != NULL)
    {
        BopIt_Logger = logger;
    }
}

void BopIt_RegisterTime(BopIt_TimeMs_t (*time)())
{
    if (time != NULL)
    {
        BopIt_Time = time;
    }
}

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
            BopIt_HandleInit(gameContext);
            break;
        case START:
            BopIt_HandleStart(gameContext);
            break;
        case COMMAND:
            BopIt_HandleCommand(gameContext);
            break;
        case WAIT:
            BopIt_HandleWait(gameContext);
            break;
        case SUCCESS:
            BopIt_HandleSuccess(gameContext);
            break;
        case FAIL:
            BopIt_HandleFail(gameContext);
            break;
        case END:
            BopIt_HandleEnd(gameContext);
            break;
        case DEINIT:
            BopIt_HandleDeinit(gameContext);
            break;
        default:
            break;
        }
    }
}

static void BopIt_Log(const char *const format, ...)
{
    va_list args;

    va_start(args, format);
    vsnprintf(BopIt_LogBuffer, BOPIT_LOG_BUFFER_SIZE, format, args);
    va_end(args);

    if (format != NULL)
    {
        if (BopIt_Logger == NULL)
        {
            printf("%s", BopIt_LogBuffer);
        }
        else
        {
            (*BopIt_Logger)(BopIt_LogBuffer);
        }
    }
}

static BopIt_TimeMs_t BopIt_GetTime()
{
    BopIt_TimeMs_t time = 0U;

    if (BopIt_Time != NULL)
    {
        time = (*BopIt_Time)();
    }

    return time;
}

static BopIt_TimeMs_t BopIt_GetElapsedTime(const BopIt_TimeMs_t startTime)
{
    BopIt_TimeMs_t time = 0U;

    if (BopIt_Time != NULL)
    {
        time = (*BopIt_Time)() - startTime;
    }

    return time;
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

static void BopIt_HandleInit(BopIt_GameContext_t *const gameContext)
{
    if (gameContext != NULL)
    {
        BopIt_Log("Initializing game\n");

        /* TODO configure game */

        gameContext->GameState = START;
    }
}

static void BopIt_HandleStart(BopIt_GameContext_t *const gameContext)
{
    if (gameContext != NULL)
    {
        BopIt_Log("Starting game\n");

        /* TODO start game */

        gameContext->GameState = COMMAND;
    }
}

static void BopIt_HandleCommand(BopIt_GameContext_t *const gameContext)
{
    if (gameContext != NULL)
    {
        BopIt_Log("Score: %d, Lives: %d\n", gameContext->Score, gameContext->Lives);

        BopIt_Log("Issuing command\n");
        gameContext->CurrentCommand = BopIt_GetRandomCommand(gameContext->Commands, gameContext->CommandCount);
        gameContext->CurrentCommand->IssueCommand();

        BopIt_Log("Waiting for player action\n");
        gameContext->WaitStart = BopIt_GetTime();
        gameContext->GameState = WAIT;
    }
}

static void BopIt_HandleWait(BopIt_GameContext_t *const gameContext)
{
    if (gameContext != NULL)
    {
        /* TODO check for inputs */

        if (BopIt_GetElapsedTime(gameContext->WaitStart) > gameContext->WaitTime)
        {
            BopIt_Log("Out of time\n");
            gameContext->GameState = FAIL;
        }

        /* TODO check input for success */
    }
}

static void BopIt_HandleSuccess(BopIt_GameContext_t *const gameContext)
{
    if (gameContext != NULL)
    {
        BopIt_Log("Player action success\n");
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
    }
}

static void BopIt_HandleFail(BopIt_GameContext_t *const gameContext)
{
    if (gameContext != NULL)
    {
        BopIt_Log("Player action fail\n");
        gameContext->CurrentCommand->FailFeedback();
        gameContext->Lives--;
        if (gameContext->Lives == 0U)
        {
            gameContext->GameState = END;
        }
    }
}

static void BopIt_HandleEnd(BopIt_GameContext_t *const gameContext)
{
    if (gameContext != NULL)
    {
        BopIt_Log("Game over\n");
        BopIt_Log("Score: %d, Lives: %d\n", gameContext->Score, gameContext->Lives);

        /* TODO end game */

        gameContext->GameState = DEINIT;
    }
}

static void BopIt_HandleDeinit(BopIt_GameContext_t *const gameContext)
{
    if (gameContext != NULL)
    {
        /* TODO handle deinit */
    }
}
