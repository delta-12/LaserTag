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

    void *DFPlayerMini_CreateHandle(const uint32_t rxPin, const uint32_t txPin);
    void DFPlayerMini_FreeHandle(void *handle);
    bool DFPlayerMini_HandleMessage(const void *const handle, const uint8_t type, const uint16_t parameter);
    bool DFPlayerMini_HandleError(const void *const handle, const uint8_t type, const uint16_t parameter);
    uint8_t DFPlayerMini_ReadCommand(const void *const handle);
    bool DFPlayerMini_Begin(const void *const handle, const bool isACK, const bool doReset);
    bool DFPlayerMini_WaitAvailable(const void *const handle, const uint64_t duration);
    bool DFPlayerMini_Available(const void *const handle);
    uint8_t DFPlayerMini_ReadType(const void *const handle);
    uint16_t DFPlayerMini_Read(const void *const handle);
    void DFPlayerMini_SetTimeOut(const void *const handle, const uint64_t timeOutDuration);
    void DFPlayerMini_Next(const void *const handle);
    void DFPlayerMini_Previous(const void *const handle);
    void DFPlayerMini_Play(const void *const handle, const int32_t fileNumber);
    void DFPlayerMini_VolumeUp(const void *const handle);
    void DFPlayerMini_VolumeDown(const void *const handle);
    void DFPlayerMini_Volume(const void *const handle, const uint8_t volume);
    void DFPlayerMini_EQ(const void *const handle, const uint8_t eq);
    void DFPlayerMini_Loop(const void *const handle, const int32_t fileNumber);
    void DFPlayerMini_OutputDevice(const void *const handle, const uint8_t device);
    void DFPlayerMini_Sleep(const void *const handle);
    void DFPlayerMini_Reset(const void *const handle);
    void DFPlayerMini_Start(const void *const handle);
    void DFPlayerMini_Pause(const void *const handle);
    void DFPlayerMini_PlayFolder(const void *const handle, const uint8_t folderNumber, const uint8_t fileNumber);
    void DFPlayerMini_OutputSetting(const void *const handle, const bool enable, const uint8_t gain);
    void DFPlayerMini_EnableLoopAll(const void *const handle);
    void DFPlayerMini_DisableLoopAll(const void *const handle);
    void DFPlayerMini_PlayMp3Folder(const void *const handle, const int32_t fileNumber);
    void DFPlayerMini_Advertise(const void *const handle, int fileNumber);
    void DFPlayerMini_PlayLargeFolder(const void *const handle, const uint8_t folderNumber, const uint16_t fileNumber);
    void DFPlayerMini_StopAdvertise(const void *const handle);
    void DFPlayerMini_Stop(const void *const handle);
    void DFPlayerMini_LoopFolder(const void *const handle, const int32_t folderNumber);
    void DFPlayerMini_RandomAll(const void *const handle);
    void DFPlayerMini_EnableLoop(const void *const handle);
    void DFPlayerMini_DisableLoop(const void *const handle);
    void DFPlayerMini_EnableDAC(const void *const handle);
    void DFPlayerMini_DisableDAC(const void *const handle);
    int32_t DFPlayerMini_ReadState(const void *const handle);
    int32_t DFPlayerMini_ReadVolume(const void *const handle);
    int32_t DFPlayerMini_ReadEQ(const void *const handle);
    int32_t DFPlayerMini_ReadFileCounts(const void *const handle, const uint8_t device);
    int32_t DFPlayerMini_ReadCurrentFileNumber(const void *const handle, const uint8_t device);
    int32_t DFPlayerMini_ReadFileCountsInFolder(const void *const handle, const int32_t folderNumber);
    int32_t DFPlayerMini_ReadFolderCounts(const void *const handle);

#ifdef __cplusplus
}
#endif

#endif