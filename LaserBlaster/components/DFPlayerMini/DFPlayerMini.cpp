/**
 * @file DFPlayerMini.cpp
 *
 * @brief C-compatible wrapper around DFPlayerMini_PlayerMini
 *
 ******************************************************************************/

/* Includes
 ******************************************************************************/
#include "Arduino.h"
#include "DFPlayerMini.h"
#include "DFPlayerMini_PlayerMini.h"

/* Function Definitions
 ******************************************************************************/

/* Define with C linkage */
extern "C"
{

    void *DFPlayerMini_CreateHandle(const uart_port_t uartNum, const uint32_t rxPin, const uint32_t txPin)
    {
        return (DFPlayerMini_PlayerMini *)new DFPlayerMini_PlayerMini(uartNum, rxPin, txPin);
    }

    void DFPlayerMini_FreeHandle(void *handle)
    {
        if (handle != nullptr)
        {
            delete (DFPlayerMini_PlayerMini *)handle;
        }
    }

    bool DFPlayerMini_HandleMessage(const void *const handle, const uint8_t type, const uint16_t parameter)
    {
        bool handledMessage = false;

        if (handle != nullptr)
        {
            handledMessage = ((DFPlayerMini_PlayerMini *)handle)->handleMessage(type, parameter);
        }

        return handledMessage;
    }

    bool DFPlayerMini_HandleError(const void *const handle, const uint8_t type, const uint16_t parameter)
    {
        bool handledError = false;

        if (handle != nullptr)
        {
            handledError = ((DFPlayerMini_PlayerMini *)handle)->handleMessage(type, parameter);
        }

        return handledError;
    }
    uint8_t DFPlayerMini_ReadCommand(const void *const handle)
    {
        uint8_t command = 0U;

        if (handle != nullptr)
        {
            command = ((DFPlayerMini_PlayerMini *)handle)->readCommand();
        }

        return command;
    }

    bool DFPlayerMini_Begin(const void *const handle, const bool isACK, const bool doReset)
    {
        bool begin = false;

        if (handle != nullptr)
        {
            begin = ((DFPlayerMini_PlayerMini *)handle)->begin(isACK, doReset);
        }

        return begin;
    }

    bool DFPlayerMini_WaitAvailable(const void *const handle, const uint64_t duration)
    {
        bool available = false;

        if (handle != nullptr)
        {
            available = ((DFPlayerMini_PlayerMini *)handle)->waitAvailable(duration);
        }

        return available;
    }

    bool DFPlayerMini_Available(const void *const handle)
    {
        bool available = false;

        if (handle != nullptr)
        {
            available = ((DFPlayerMini_PlayerMini *)handle)->available();
        }

        return available;
    }

    uint8_t DFPlayerMini_ReadType(const void *const handle)
    {
        uint8_t type = 0U;

        if (handle != nullptr)
        {
            type = ((DFPlayerMini_PlayerMini *)handle)->readType();
        }

        return type;
    }

    uint16_t DFPlayerMini_Read(const void *const handle)
    {
        uint16_t read = 0U;

        if (handle != nullptr)
        {
            read = ((DFPlayerMini_PlayerMini *)handle)->read();
        }

        return read;
    }

    void DFPlayerMini_SetTimeOut(const void *const handle, const uint64_t timeOutDuration)
    {
        if (handle != nullptr)
        {
            ((DFPlayerMini_PlayerMini *)handle)->setTimeOut(timeOutDuration);
        }
    }

    void DFPlayerMini_Next(const void *const handle)
    {
        if (handle != nullptr)
        {
            ((DFPlayerMini_PlayerMini *)handle)->next();
        }
    }

    void DFPlayerMini_Previous(const void *const handle)
    {
        if (handle != nullptr)
        {
            ((DFPlayerMini_PlayerMini *)handle)->previous();
        }
    }

    void DFPlayerMini_Play(const void *const handle, const int32_t fileNumber)
    {
        if (handle != nullptr)
        {
            ((DFPlayerMini_PlayerMini *)handle)->play(fileNumber);
        }
    }

    void DFPlayerMini_VolumeUp(const void *const handle)
    {
        if (handle != nullptr)
        {
            ((DFPlayerMini_PlayerMini *)handle)->volumeUp();
        }
    }

    void DFPlayerMini_VolumeDown(const void *const handle)
    {
        if (handle != nullptr)
        {
            ((DFPlayerMini_PlayerMini *)handle)->volumeDown();
        }
    }

    void DFPlayerMini_Volume(const void *const handle, const uint8_t volume)
    {
        if (handle != nullptr)
        {
            ((DFPlayerMini_PlayerMini *)handle)->volume(volume);
        }
    }

    void DFPlayerMini_EQ(const void *const handle, const uint8_t eq)
    {
        if (handle != nullptr)
        {
            ((DFPlayerMini_PlayerMini *)handle)->EQ(eq);
        }
    }

    void DFPlayerMini_Loop(const void *const handle, const int32_t fileNumber)
    {
        if (handle != nullptr)
        {
            ((DFPlayerMini_PlayerMini *)handle)->loop(fileNumber);
        }
    }

    void DFPlayerMini_OutputDevice(const void *const handle, const uint8_t device)
    {
        if (handle != nullptr)
        {
            ((DFPlayerMini_PlayerMini *)handle)->outputDevice(device);
        }
    }

    void DFPlayerMini_Sleep(const void *const handle)
    {
        if (handle != nullptr)
        {
            ((DFPlayerMini_PlayerMini *)handle)->sleep();
        }
    }

    void DFPlayerMini_Reset(const void *const handle)
    {
        if (handle != nullptr)
        {
            ((DFPlayerMini_PlayerMini *)handle)->reset();
        }
    }

    void DFPlayerMini_Start(const void *const handle)
    {
        if (handle != nullptr)
        {
            ((DFPlayerMini_PlayerMini *)handle)->start();
        }
    }

    void DFPlayerMini_Pause(const void *const handle)
    {
        if (handle != nullptr)
        {
            ((DFPlayerMini_PlayerMini *)handle)->pause();
        }
    }

    void DFPlayerMini_PlayFolder(const void *const handle, const uint8_t folderNumber, const uint8_t fileNumber)
    {
        if (handle != nullptr)
        {
            ((DFPlayerMini_PlayerMini *)handle)->playFolder(folderNumber, fileNumber);
        }
    }

    void DFPlayerMini_OutputSetting(const void *const handle, const bool enable, const uint8_t gain)
    {
        if (handle != nullptr)
        {
            ((DFPlayerMini_PlayerMini *)handle)->outputSetting(enable, gain);
        }
    }

    void DFPlayerMini_EnableLoopAll(const void *const handle)
    {
        if (handle != nullptr)
        {
            ((DFPlayerMini_PlayerMini *)handle)->enableLoopAll();
        }
    }

    void DFPlayerMini_DisableLoopAll(const void *const handle)
    {
        if (handle != nullptr)
        {
            ((DFPlayerMini_PlayerMini *)handle)->disableLoopAll();
        }
    }

    void DFPlayerMini_PlayMp3Folder(const void *const handle, const int32_t fileNumber)
    {
        if (handle != nullptr)
        {
            ((DFPlayerMini_PlayerMini *)handle)->playMp3Folder(fileNumber);
        }
    }

    void DFPlayerMini_Advertise(const void *const handle, int fileNumber)
    {
        if (handle != nullptr)
        {
            ((DFPlayerMini_PlayerMini *)handle)->advertise(fileNumber);
        }
    }

    void DFPlayerMini_PlayLargeFolder(const void *const handle, const uint8_t folderNumber, const uint16_t fileNumber)
    {
        if (handle != nullptr)
        {
            ((DFPlayerMini_PlayerMini *)handle)->playLargeFolder(folderNumber, fileNumber);
        }
    }

    void DFPlayerMini_StopAdvertise(const void *const handle)
    {
        if (handle != nullptr)
        {
            ((DFPlayerMini_PlayerMini *)handle)->stopAdvertise();
        }
    }

    void DFPlayerMini_Stop(const void *const handle)
    {
        if (handle != nullptr)
        {
            ((DFPlayerMini_PlayerMini *)handle)->stop();
        }
    }

    void DFPlayerMini_LoopFolder(const void *const handle, const int32_t folderNumber)
    {
        if (handle != nullptr)
        {
            ((DFPlayerMini_PlayerMini *)handle)->loopFolder(folderNumber);
        }
    }

    void DFPlayerMini_RandomAll(const void *const handle)
    {
        if (handle != nullptr)
        {
            ((DFPlayerMini_PlayerMini *)handle)->randomAll();
        }
    }

    void DFPlayerMini_EnableLoop(const void *const handle)
    {
        if (handle != nullptr)
        {
            ((DFPlayerMini_PlayerMini *)handle)->enableLoop();
        }
    }

    void DFPlayerMini_DisableLoop(const void *const handle)
    {
        if (handle != nullptr)
        {
            ((DFPlayerMini_PlayerMini *)handle)->disableLoop();
        }
    }

    void DFPlayerMini_EnableDAC(const void *const handle)
    {
        if (handle != nullptr)
        {
            ((DFPlayerMini_PlayerMini *)handle)->enableDAC();
        }
    }

    void DFPlayerMini_DisableDAC(const void *const handle)
    {
        if (handle != nullptr)
        {
            ((DFPlayerMini_PlayerMini *)handle)->disableDAC();
        }
    }

    int32_t DFPlayerMini_ReadState(const void *const handle)
    {
        int32_t state = 0U;

        if (handle != nullptr)
        {
            state = ((DFPlayerMini_PlayerMini *)handle)->readState();
        }

        return state;
    }

    int32_t DFPlayerMini_ReadVolume(const void *const handle)
    {
        int32_t volume = 0U;

        if (handle != nullptr)
        {
            volume = ((DFPlayerMini_PlayerMini *)handle)->readVolume();
        }

        return volume;
    }

    int32_t DFPlayerMini_ReadEQ(const void *const handle)
    {
        int32_t eq = 0U;

        if (handle != nullptr)
        {
            eq = ((DFPlayerMini_PlayerMini *)handle)->readEQ();
        }

        return eq;
    }

    int32_t DFPlayerMini_ReadFileCounts(const void *const handle, const uint8_t device)
    {
        int32_t fileCounts = 0U;

        if (handle != nullptr)
        {
            fileCounts = ((DFPlayerMini_PlayerMini *)handle)->readFileCounts();
        }

        return fileCounts;
    }

    int32_t DFPlayerMini_ReadCurrentFileNumber(const void *const handle, const uint8_t device)
    {
        int32_t currentFileNumber = 0U;

        if (handle != nullptr)
        {
            currentFileNumber = ((DFPlayerMini_PlayerMini *)handle)->readCurrentFileNumber(device);
        }

        return currentFileNumber;
    }

    int32_t DFPlayerMini_ReadFileCountsInFolder(const void *const handle, const int32_t folderNumber)
    {
        int32_t fileCountsInFolder = 0U;

        if (handle != nullptr)
        {
            fileCountsInFolder = ((DFPlayerMini_PlayerMini *)handle)->readFileCountsInFolder(folderNumber);
        }

        return fileCountsInFolder;
    }

    int32_t DFPlayerMini_ReadFolderCounts(const void *const handle)
    {
        int32_t folderCounts = 0U;

        if (handle != nullptr)
        {
            folderCounts = ((DFPlayerMini_PlayerMini *)handle)->readFolderCounts();
        }

        return folderCounts;
    }
}