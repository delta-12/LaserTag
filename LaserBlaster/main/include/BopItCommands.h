/**
 * @file BopItCommands.h
 *
 * @brief Commands for BopIt game.
 *
 ******************************************************************************/

#ifndef BOP_IT_COMMANDS_H
#define BOP_IT_COMMANDS_H

/* Includes
 ******************************************************************************/
#include "BopIt.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

/* Globals
 ******************************************************************************/

extern bool BopItCommands_Button0InputFlag;
extern SemaphoreHandle_t BopItCommands_Button0InputFlagMutex;
extern StaticSemaphore_t BopItCommands_Button0InputFlagMutexBuffer;
extern BopIt_Command_t BopItCommands_Button0;

extern bool BopItCommands_Button1InputFlag;
extern SemaphoreHandle_t BopItCommands_Button1InputFlagMutex;
extern StaticSemaphore_t BopItCommands_Button1InputFlagMutexBuffer;
extern BopIt_Command_t BopItCommands_Button1;

extern bool BopItCommands_Button2InputFlag;
extern SemaphoreHandle_t BopItCommands_Button2InputFlagMutex;
extern StaticSemaphore_t BopItCommands_Button2InputFlagMutexBuffer;
extern BopIt_Command_t BopItCommands_Button2;

/* Function Prototypes
 ******************************************************************************/

void BopItCommands_Init(void);
void BopItCommands_DeInit(void);

void BopItCommands_Button0IssueCommand(void);
void BopItCommands_Button0SuccessFeedback(void);
void BopItCommands_Button0FailFeedback(void);
bool BopItCommands_Button0GetInput(void);

void BopItCommands_Button1IssueCommand(void);
void BopItCommands_Button1SuccessFeedback(void);
void BopItCommands_Button1FailFeedback(void);
bool BopItCommands_Button1GetInput(void);

void BopItCommands_Button2IssueCommand(void);
void BopItCommands_Button2SuccessFeedback(void);
void BopItCommands_Button2FailFeedback(void);
bool BopItCommands_Button2GetInput(void);

#endif
