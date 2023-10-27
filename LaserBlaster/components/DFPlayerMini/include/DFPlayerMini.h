/**
 * @file DFPlayerMini.h
 *
 * @brief C-compatible wrapper around DFPlayerMini_PlayerMini
 *
 ******************************************************************************/

#ifndef DFPLAYERMINI_H
#define DFPLAYERMINI_H

/* Includes
 ******************************************************************************/
#include "driver/uart.h"
#include <stdint.h>

/* Defines
 ******************************************************************************/

#define DFPLAYERMINI_DFPLAYER_EQ_NORMAL 0U
#define DFPLAYERMINI_DFPLAYER_EQ_POP 1U
#define DFPLAYERMINI_DFPLAYER_EQ_ROCK 2U
#define DFPLAYERMINI_DFPLAYER_EQ_JAZZ 3U
#define DFPLAYERMINI_DFPLAYER_EQ_CLASSIC 4U
#define DFPLAYERMINI_DFPLAYER_EQ_BASS 5U

#define DFPLAYERMINI_DFPLAYER_DEVICE_U_DISK 1U
#define DFPLAYERMINI_DFPLAYER_DEVICE_SD 2U
#define DFPLAYERMINI_DFPLAYER_DEVICE_AUX 3U
#define DFPLAYERMINI_DFPLAYER_DEVICE_SLEEP 4U
#define DFPLAYERMINI_DFPLAYER_DEVICE_FLASH 5U

/* Function Prototypes
 ******************************************************************************/

/* Declare with C linkage */
#ifdef __cplusplus
extern "C"
{
#endif

    void *DFPlayerMini_CreateHandle(const uart_port_t uartNum, const uint32_t rxPin, const uint32_t txPin);
    void DFPlayerMini_FreeHandle(void *handle);
    bool DFPlayerMini_HandleMessage(void *const handle, const uint8_t type, const uint16_t parameter);
    bool DFPlayerMini_HandleError(void *const handle, const uint8_t type, const uint16_t parameter);
    uint8_t DFPlayerMini_ReadCommand(void *const handle);
    bool DFPlayerMini_Begin(void *const handle, const bool isACK, const bool doReset);
    bool DFPlayerMini_WaitAvailable(void *const handle, const uint64_t duration);
    bool DFPlayerMini_Available(void *const handle);
    uint8_t DFPlayerMini_ReadType(void *const handle);
    uint16_t DFPlayerMini_Read(void *const handle);
    void DFPlayerMini_SetTimeOut(void *const handle, const uint64_t timeOutDuration);
    void DFPlayerMini_Next(void *const handle);
    void DFPlayerMini_Previous(void *const handle);
    void DFPlayerMini_Play(void *const handle, const int32_t fileNumber);
    void DFPlayerMini_VolumeUp(void *const handle);
    void DFPlayerMini_VolumeDown(void *const handle);
    void DFPlayerMini_Volume(void *const handle, const uint8_t volume);
    void DFPlayerMini_EQ(void *const handle, const uint8_t eq);
    void DFPlayerMini_Loop(void *const handle, const int32_t fileNumber);
    void DFPlayerMini_OutputDevice(void *const handle, const uint8_t device);
    void DFPlayerMini_Sleep(void *const handle);
    void DFPlayerMini_Reset(void *const handle);
    void DFPlayerMini_Start(void *const handle);
    void DFPlayerMini_Pause(void *const handle);
    void DFPlayerMini_PlayFolder(void *const handle, const uint8_t folderNumber, const uint8_t fileNumber);
    void DFPlayerMini_OutputSetting(void *const handle, const bool enable, const uint8_t gain);
    void DFPlayerMini_EnableLoopAll(void *const handle);
    void DFPlayerMini_DisableLoopAll(void *const handle);
    void DFPlayerMini_PlayMp3Folder(void *const handle, const int32_t fileNumber);
    void DFPlayerMini_Advertise(void *const handle, int fileNumber);
    void DFPlayerMini_PlayLargeFolder(void *const handle, const uint8_t folderNumber, const uint16_t fileNumber);
    void DFPlayerMini_StopAdvertise(void *const handle);
    void DFPlayerMini_Stop(void *const handle);
    void DFPlayerMini_LoopFolder(void *const handle, const int32_t folderNumber);
    void DFPlayerMini_RandomAll(void *const handle);
    void DFPlayerMini_EnableLoop(void *const handle);
    void DFPlayerMini_DisableLoop(void *const handle);
    void DFPlayerMini_EnableDAC(void *const handle);
    void DFPlayerMini_DisableDAC(void *const handle);
    int32_t DFPlayerMini_ReadState(void *const handle);
    int32_t DFPlayerMini_ReadVolume(void *const handle);
    int32_t DFPlayerMini_ReadEQ(void *const handle);
    int32_t DFPlayerMini_ReadFileCounts(void *const handle, const uint8_t device);
    int32_t DFPlayerMini_ReadCurrentFileNumber(void *const handle, const uint8_t device);
    int32_t DFPlayerMini_ReadFileCountsInFolder(void *const handle, const int32_t folderNumber);
    int32_t DFPlayerMini_ReadFolderCounts(void *const handle);

#ifdef __cplusplus
}
#endif

#endif