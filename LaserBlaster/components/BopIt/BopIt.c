/**
 * @file BopIt.c
 *
 * @brief Keep track of game state and handle commands and actions.
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

#define BOPIT_LOG_BUFFER_SIZE 1024U                                                                        /* Size of buffer for storing messages to be logged */
#define BOPIT_MAX_SCORE 99U                                                                                /* Maximum game score, game is over after player reaches this score */
#define BOPIT_INIT_LIVES 3U                                                                                /* Starting player lives */
#define BOPIT_MAX_WAIT_TIME_MS 8000U                                                                       /* Maximum time in milliseconds to complete a command */
#define BOPIT_MIN_WAIT_TIME_MS 2000U                                                                       /* Minimum time in milliseconds to complete a command */
#define BOPIT_WAIT_TIME_DECREMENT_MS (BOPIT_MAX_WAIT_TIME_MS - BOPIT_MIN_WAIT_TIME_MS) / (BOPIT_MAX_SCORE) /* Numer of milliseconds to decrease time to complete a command */

/* Globals
 ******************************************************************************/

static char BopIt_LogBuffer[BOPIT_LOG_BUFFER_SIZE]; /* Buffer for storing messages to be logged */

static void (*BopIt_Logger)(const char *const message) = NULL; /* Client-specified logging function, not be called directly */
static BopIt_TimeMs_t (*BopIt_Time)(void) = NULL;              /* Client-specified function to get current time in milliseconds, not be called directly */

/* Function Prototypes
 ******************************************************************************/

static void BopIt_Log(const char *const format, ...);
static BopIt_TimeMs_t BopIt_GetTime(void);
static BopIt_TimeMs_t BopIt_GetElapsedTime(const BopIt_TimeMs_t startTime);
static BopIt_Command_t *BopIt_GetRandomCommand(const BopIt_Command_t *const *const commands, const uint32_t commandCount);
static void BopIt_HandleStart(BopIt_GameContext_t *const gameContext);
static void BopIt_HandleCommand(BopIt_GameContext_t *const gameContext);
static void BopIt_HandleWait(BopIt_GameContext_t *const gameContext);
static void BopIt_HandleSuccess(BopIt_GameContext_t *const gameContext);
static void BopIt_HandleFail(BopIt_GameContext_t *const gameContext);
static void BopIt_HandleEnd(BopIt_GameContext_t *const gameContext);

/* Function Definitions
 ******************************************************************************/

/**
 * @brief Register a function for logging.
 *
 * @param[in] logger Logging function
 ******************************************************************************/
void BopIt_RegisterLogger(void (*logger)(const char *const message))
{
    if (logger != NULL)
    {
        BopIt_Logger = logger;
    }
}

/**
 * @brief Register a function for getting the current time in milliseconds.
 *
 * @param[in] time Function to get current time in milliseconds
 ******************************************************************************/
void BopIt_RegisterTime(BopIt_TimeMs_t (*time)(void))
{
    if (time != NULL)
    {
        BopIt_Time = time;
    }
}

/**
 * @brief Initialize a BopIt game.  Sets initial game state, player score and
 * lives, and time to complete a command.  Also seeds rand for selecting a
 * random command to issue.
 *
 * @param[in,out] gameContext Context for a BopIt game
 ******************************************************************************/
void BopIt_Init(BopIt_GameContext_t *const gameContext)
{
    if (gameContext != NULL)
    {
        gameContext->GameState = BOPIT_GAMESTATE_START;
        gameContext->Score = 0U;
        gameContext->Lives = BOPIT_INIT_LIVES;
        gameContext->WaitTime = BOPIT_MAX_WAIT_TIME_MS;

        srand(time(NULL));
    }
}

/**
 * @brief Handle the current state of a BopIt game.  Call to update the game
 * state.
 *
 * @param[in,out] gameContext Context for a BopIt game
 ******************************************************************************/
void BopIt_Run(BopIt_GameContext_t *const gameContext)
{
    if (gameContext != NULL)
    {
        switch (gameContext->GameState)
        {
        case BOPIT_GAMESTATE_START:
            BopIt_HandleStart(gameContext);
            break;
        case BOPIT_GAMESTATE_COMMAND:
            BopIt_HandleCommand(gameContext);
            break;
        case BOPIT_GAMESTATE_WAIT:
            BopIt_HandleWait(gameContext);
            break;
        case BOPIT_GAMESTATE_SUCCESS:
            BopIt_HandleSuccess(gameContext);
            break;
        case BOPIT_GAMESTATE_FAIL:
            BopIt_HandleFail(gameContext);
            break;
        case BOPIT_GAMESTATE_END:
            BopIt_HandleEnd(gameContext);
            break;
        default:
            break;
        }
    }
}

/**
 * @brief Log a message using registered logging function.  Calls printf if no
 * logging function is registered.
 *
 * @param[in] format Message format string
 * @param[in] ...    Arguments for message format string
 ******************************************************************************/
static void BopIt_Log(const char *const format, ...)
{
    if (format != NULL)
    {
        va_list args;

        va_start(args, format);
        vsnprintf(BopIt_LogBuffer, BOPIT_LOG_BUFFER_SIZE, format, args);
        va_end(args);

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

/**
 * @brief Get the current time in milliseconds.  Returns 0 if no function to
 * get the current time in milliseconds is registered.
 *
 * @return Current time in milliseconds
 ******************************************************************************/
static BopIt_TimeMs_t BopIt_GetTime(void)
{
    BopIt_TimeMs_t time = 0U;

    if (BopIt_Time != NULL)
    {
        time = (*BopIt_Time)();
    }

    return time;
}

/**
 * @brief Get the elapsed time from the start time in milliseconds.
 *
 * @param[in] startTime Start time in milliseconds from which to get the
 * elapsed time
 *
 * @return Elapsed time in milliseconds
 ******************************************************************************/
static BopIt_TimeMs_t BopIt_GetElapsedTime(const BopIt_TimeMs_t startTime)
{
    BopIt_TimeMs_t time = 0U;

    if (BopIt_Time != NULL)
    {
        time = (*BopIt_Time)() - startTime;
    }

    return time;
}

/**
 * @brief Get a command randomly selected from a list of commands.
 *
 * @param[in] commands     Pointer to list of command pointers
 * @param[in] commandCount Number of commands in list
 *
 * @return Pointer to a randomly selected command
 ******************************************************************************/
static BopIt_Command_t *BopIt_GetRandomCommand(const BopIt_Command_t *const *const commands, const uint32_t commandCount)
{
    BopIt_Command_t *command = NULL;

    if (commands != NULL)
    {
        command = *(BopIt_Command_t **)(commands + (rand() % commandCount));
    }

    return command;
}

/**
 * @brief Handle the start state of a BopIt game.  Starts the game.
 *
 * @param[in,out] gameContext Context for a BopIt game
 ******************************************************************************/
static void BopIt_HandleStart(BopIt_GameContext_t *const gameContext)
{
    if (gameContext != NULL)
    {
        BopIt_Log("Starting game");

        if (gameContext->OnGameStart != NULL)
        {
            (*gameContext->OnGameStart)(gameContext);
        }

        if (gameContext->CommandCount > 0U)
        {
            gameContext->GameState = BOPIT_GAMESTATE_COMMAND;
        }
        else
        {
            BopIt_Log("No commands, ending game.");
            gameContext->GameState = BOPIT_GAMESTATE_END;
        }
    }
}

/**
 * @brief Handle the command state of a BopIt game.  Issues a randomly selected
 * command.
 *
 * @param[in,out] gameContext Context for a BopIt game
 ******************************************************************************/
static void BopIt_HandleCommand(BopIt_GameContext_t *const gameContext)
{
    if (gameContext != NULL)
    {
        BopIt_Log("Score: %d, Lives: %d, Time to Complete Command: %ldms", gameContext->Score, gameContext->Lives, gameContext->WaitTime);

        gameContext->CurrentCommand = BopIt_GetRandomCommand((const BopIt_Command_t *const *const)(gameContext->Commands), gameContext->CommandCount);
        if (gameContext->CurrentCommand != NULL)
        {
            BopIt_Log("Issuing command %s", gameContext->CurrentCommand->Name);
            gameContext->CurrentCommand->IssueCommand();
        }

        BopIt_Log("Waiting for player action");
        gameContext->WaitStart = BopIt_GetTime();
        gameContext->GameState = BOPIT_GAMESTATE_WAIT;
    }
}

/**
 * @brief Handle the wait state of a BopIt game.  Waits for the player to
 * complete the issued command in a given amount of time and checks if the
 * correct input was made by the player.
 *
 * @param[in,out] gameContext Context for a BopIt game
 ******************************************************************************/
static void BopIt_HandleWait(BopIt_GameContext_t *const gameContext)
{

    if (gameContext != NULL)
    {
        uint32_t commandIndex = 0U;

        /* Check if the player is out of time to complete the issued command */
        if (BopIt_GetElapsedTime(gameContext->WaitStart) > gameContext->WaitTime)
        {
            BopIt_Log("Out of time");
            gameContext->GameState = BOPIT_GAMESTATE_FAIL;
        }

        /* Check if the player made the correct input and no other inputs */
        while (commandIndex < gameContext->CommandCount && gameContext->GameState != BOPIT_GAMESTATE_FAIL)
        {
            BopIt_Command_t *command = *(BopIt_Command_t **)(gameContext->Commands + commandIndex);

            if ((*command->GetInput)())
            {
                if (command == gameContext->CurrentCommand)
                {
                    gameContext->GameState = BOPIT_GAMESTATE_SUCCESS;
                }
                else
                {
                    gameContext->GameState = BOPIT_GAMESTATE_FAIL;
                }
            }

            commandIndex++;
        }
    }
}

/**
 * @brief Handle the success state of a BopIt game.  Calls function to provide
 * feedback for successfully completing the command, increments the player
 * score, and decreases the time to complete the next command.
 *
 * @param[in,out] gameContext Context for a BopIt game
 ******************************************************************************/
static void BopIt_HandleSuccess(BopIt_GameContext_t *const gameContext)
{
    if (gameContext != NULL)
    {
        BopIt_Log("Player action success");
        gameContext->CurrentCommand->SuccessFeedback();
        gameContext->Score++;
        if (gameContext->Score < BOPIT_MAX_SCORE)
        {
            gameContext->WaitTime -= BOPIT_WAIT_TIME_DECREMENT_MS;
            gameContext->GameState = BOPIT_GAMESTATE_COMMAND;
        }
        else
        {
            gameContext->GameState = BOPIT_GAMESTATE_END;
        }
    }
}

/**
 * @brief Handle the success state of a BopIt game.  Calls function to provide
 * feedback for failing to complete the command and decreases player lives.
 *
 * @param[in,out] gameContext Context for a BopIt game
 ******************************************************************************/
static void BopIt_HandleFail(BopIt_GameContext_t *const gameContext)
{
    if (gameContext != NULL)
    {
        BopIt_Log("Player action fail");
        gameContext->CurrentCommand->FailFeedback();
        gameContext->Lives--;
        if (gameContext->Lives == 0U)
        {
            gameContext->GameState = BOPIT_GAMESTATE_END;
        }
        else
        {
            gameContext->GameState = BOPIT_GAMESTATE_COMMAND;
        }
    }
}

/**
 * @brief Handle the end state of a BopIt game.  Ends BopIt game.
 *
 * @param[in,out] gameContext Context for a BopIt game
 ******************************************************************************/
static void BopIt_HandleEnd(BopIt_GameContext_t *const gameContext)
{
    if (gameContext != NULL)
    {
        BopIt_Log("Game over");
        BopIt_Log("Score: %d, Lives: %d", gameContext->Score, gameContext->Lives);

        if (gameContext->OnGameEnd != NULL)
        {
            (*gameContext->OnGameEnd)(gameContext);
        }
    }
}
