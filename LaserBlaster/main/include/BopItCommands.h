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

extern bool BopItCommands_TriggerInputFlag;
extern SemaphoreHandle_t BopItCommands_TriggerInputFlagMutex;
extern StaticSemaphore_t BopItCommands_TriggerInputFlagMutexBuffer;
extern bool BopItCommands_TriggerCommandIssuedFlag;
extern SemaphoreHandle_t BopItCommands_TriggerCommandIssuedFlagMutex;
extern StaticSemaphore_t BopItCommands_TriggerCommandIssuedFlagMutexBuffer;
extern BopIt_Command_t BopItCommands_Trigger;

extern bool BopItCommands_PrimeInputFlag;
extern SemaphoreHandle_t BopItCommands_PrimeInputFlagMutex;
extern StaticSemaphore_t BopItCommands_PrimeInputFlagMutexBuffer;
extern BopIt_Command_t BopItCommands_Prime;

extern bool BopItCommands_ReloadInputFlag;
extern SemaphoreHandle_t BopItCommands_ReloadInputFlagMutex;
extern StaticSemaphore_t BopItCommands_ReloadInputFlagMutexBuffer;
extern BopIt_Command_t BopItCommands_Reload;

/* Function Prototypes
 ******************************************************************************/

void BopItCommands_Init(void);
void BopItCommands_DeInit(void);

void BopItCommands_TriggerIssueCommand(void);
void BopItCommands_TriggerSuccessFeedback(void);
void BopItCommands_TriggerFailFeedback(void);
bool BopItCommands_TriggerGetInput(void);

void BopItCommands_PrimeIssueCommand(void);
void BopItCommands_PrimeSuccessFeedback(void);
void BopItCommands_PrimeFailFeedback(void);
bool BopItCommands_PrimeGetInput(void);

void BopItCommands_ReloadIssueCommand(void);
void BopItCommands_ReloadSuccessFeedback(void);
void BopItCommands_ReloadFailFeedback(void);
bool BopItCommands_ReloadGetInput(void);

#endif
