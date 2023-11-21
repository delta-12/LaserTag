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
        return static_cast<DFPlayerMini_PlayerMini *>(new DFPlayerMini_PlayerMini(uartNum, rxPin, txPin));
    }

    void DFPlayerMini_FreeHandle(void *handle)
    {
        if (handle != nullptr)
        {
            delete static_cast<DFPlayerMini_PlayerMini *>(handle);
        }
    }

    bool DFPlayerMini_HandleMessage(void *const handle, const uint8_t type, const uint16_t parameter)
    {
        bool handledMessage = false;

        if (handle != nullptr)
        {
            handledMessage = (static_cast<DFPlayerMini_PlayerMini *>(handle))->handleMessage(type, parameter);
        }

        return handledMessage;
    }

    bool DFPlayerMini_HandleError(void *const handle, const uint8_t type, const uint16_t parameter)
    {
        bool handledError = false;

        if (handle != nullptr)
        {
            handledError = (static_cast<DFPlayerMini_PlayerMini *>(handle))->handleMessage(type, parameter);
        }

        return handledError;
    }
    uint8_t DFPlayerMini_ReadCommand(void *const handle)
    {
        uint8_t command = 0U;

        if (handle != nullptr)
        {
            command = (static_cast<DFPlayerMini_PlayerMini *>(handle))->readCommand();
        }

        return command;
    }

    bool DFPlayerMini_Begin(void *const handle, const bool isACK, const bool doReset)
    {
        bool begin = false;

        if (handle != nullptr)
        {
            begin = (static_cast<DFPlayerMini_PlayerMini *>(handle))->begin(isACK, doReset);
        }

        return begin;
    }

    bool DFPlayerMini_WaitAvailable(void *const handle, const uint64_t duration)
    {
        bool available = false;

        if (handle != nullptr)
        {
            available = (static_cast<DFPlayerMini_PlayerMini *>(handle))->waitAvailable(duration);
        }

        return available;
    }

    bool DFPlayerMini_Available(void *const handle)
    {
        bool available = false;

        if (handle != nullptr)
        {
            available = (static_cast<DFPlayerMini_PlayerMini *>(handle))->available();
        }

        return available;
    }

    uint8_t DFPlayerMini_ReadType(void *const handle)
    {
        uint8_t type = 0U;

        if (handle != nullptr)
        {
            type = (static_cast<DFPlayerMini_PlayerMini *>(handle))->readType();
        }

        return type;
    }

    uint16_t DFPlayerMini_Read(void *const handle)
    {
        uint16_t read = 0U;

        if (handle != nullptr)
        {
            read = (static_cast<DFPlayerMini_PlayerMini *>(handle))->read();
        }

        return read;
    }

    void DFPlayerMini_SetTimeOut(void *const handle, const uint64_t timeOutDuration)
    {
        if (handle != nullptr)
        {
            (static_cast<DFPlayerMini_PlayerMini *>(handle))->setTimeOut(timeOutDuration);
        }
    }

    void DFPlayerMini_Next(void *const handle)
    {
        if (handle != nullptr)
        {
            (static_cast<DFPlayerMini_PlayerMini *>(handle))->next();
        }
    }

    void DFPlayerMini_Previous(void *const handle)
    {
        if (handle != nullptr)
        {
            (static_cast<DFPlayerMini_PlayerMini *>(handle))->previous();
        }
    }

    void DFPlayerMini_Play(void *const handle, const int32_t fileNumber)
    {
        if (handle != nullptr)
        {
            (static_cast<DFPlayerMini_PlayerMini *>(handle))->play(fileNumber);
        }
    }

    void DFPlayerMini_VolumeUp(void *const handle)
    {
        if (handle != nullptr)
        {
            (static_cast<DFPlayerMini_PlayerMini *>(handle))->volumeUp();
        }
    }

    void DFPlayerMini_VolumeDown(void *const handle)
    {
        if (handle != nullptr)
        {
            (static_cast<DFPlayerMini_PlayerMini *>(handle))->volumeDown();
        }
    }

    void DFPlayerMini_Volume(void *const handle, const uint8_t volume)
    {
        if (handle != nullptr)
        {
            (static_cast<DFPlayerMini_PlayerMini *>(handle))->volume(volume);
        }
    }

    void DFPlayerMini_EQ(void *const handle, const uint8_t eq)
    {
        if (handle != nullptr)
        {
            (static_cast<DFPlayerMini_PlayerMini *>(handle))->EQ(eq);
        }
    }

    void DFPlayerMini_Loop(void *const handle, const int32_t fileNumber)
    {
        if (handle != nullptr)
        {
            (static_cast<DFPlayerMini_PlayerMini *>(handle))->loop(fileNumber);
        }
    }

    void DFPlayerMini_OutputDevice(void *const handle, const uint8_t device)
    {
        if (handle != nullptr)
        {
            (static_cast<DFPlayerMini_PlayerMini *>(handle))->outputDevice(device);
        }
    }

    void DFPlayerMini_Sleep(void *const handle)
    {
        if (handle != nullptr)
        {
            (static_cast<DFPlayerMini_PlayerMini *>(handle))->sleep();
        }
    }

    void DFPlayerMini_Reset(void *const handle)
    {
        if (handle != nullptr)
        {
            (static_cast<DFPlayerMini_PlayerMini *>(handle))->reset();
        }
    }

    void DFPlayerMini_Start(void *const handle)
    {
        if (handle != nullptr)
        {
            (static_cast<DFPlayerMini_PlayerMini *>(handle))->start();
        }
    }

    void DFPlayerMini_Pause(void *const handle)
    {
        if (handle != nullptr)
        {
            (static_cast<DFPlayerMini_PlayerMini *>(handle))->pause();
        }
    }

    void DFPlayerMini_PlayFolder(void *const handle, const uint8_t folderNumber, const uint8_t fileNumber)
    {
        if (handle != nullptr)
        {
            (static_cast<DFPlayerMini_PlayerMini *>(handle))->playFolder(folderNumber, fileNumber);
        }
    }

    void DFPlayerMini_OutputSetting(void *const handle, const bool enable, const uint8_t gain)
    {
        if (handle != nullptr)
        {
            (static_cast<DFPlayerMini_PlayerMini *>(handle))->outputSetting(enable, gain);
        }
    }

    void DFPlayerMini_EnableLoopAll(void *const handle)
    {
        if (handle != nullptr)
        {
            (static_cast<DFPlayerMini_PlayerMini *>(handle))->enableLoopAll();
        }
    }

    void DFPlayerMini_DisableLoopAll(void *const handle)
    {
        if (handle != nullptr)
        {
            (static_cast<DFPlayerMini_PlayerMini *>(handle))->disableLoopAll();
        }
    }

    void DFPlayerMini_PlayMp3Folder(void *const handle, const int32_t fileNumber)
    {
        if (handle != nullptr)
        {
            (static_cast<DFPlayerMini_PlayerMini *>(handle))->playMp3Folder(fileNumber);
        }
    }

    void DFPlayerMini_Advertise(void *const handle, int fileNumber)
    {
        if (handle != nullptr)
        {
            (static_cast<DFPlayerMini_PlayerMini *>(handle))->advertise(fileNumber);
        }
    }

    void DFPlayerMini_PlayLargeFolder(void *const handle, const uint8_t folderNumber, const uint16_t fileNumber)
    {
        if (handle != nullptr)
        {
            (static_cast<DFPlayerMini_PlayerMini *>(handle))->playLargeFolder(folderNumber, fileNumber);
        }
    }

    void DFPlayerMini_StopAdvertise(void *const handle)
    {
        if (handle != nullptr)
        {
            (static_cast<DFPlayerMini_PlayerMini *>(handle))->stopAdvertise();
        }
    }

    void DFPlayerMini_Stop(void *const handle)
    {
        if (handle != nullptr)
        {
            (static_cast<DFPlayerMini_PlayerMini *>(handle))->stop();
        }
    }

    void DFPlayerMini_LoopFolder(void *const handle, const int32_t folderNumber)
    {
        if (handle != nullptr)
        {
            (static_cast<DFPlayerMini_PlayerMini *>(handle))->loopFolder(folderNumber);
        }
    }

    void DFPlayerMini_RandomAll(void *const handle)
    {
        if (handle != nullptr)
        {
            (static_cast<DFPlayerMini_PlayerMini *>(handle))->randomAll();
        }
    }

    void DFPlayerMini_EnableLoop(void *const handle)
    {
        if (handle != nullptr)
        {
            (static_cast<DFPlayerMini_PlayerMini *>(handle))->enableLoop();
        }
    }

    void DFPlayerMini_DisableLoop(void *const handle)
    {
        if (handle != nullptr)
        {
            (static_cast<DFPlayerMini_PlayerMini *>(handle))->disableLoop();
        }
    }

    void DFPlayerMini_EnableDAC(void *const handle)
    {
        if (handle != nullptr)
        {
            (static_cast<DFPlayerMini_PlayerMini *>(handle))->enableDAC();
        }
    }

    void DFPlayerMini_DisableDAC(void *const handle)
    {
        if (handle != nullptr)
        {
            (static_cast<DFPlayerMini_PlayerMini *>(handle))->disableDAC();
        }
    }

    int32_t DFPlayerMini_ReadState(void *const handle)
    {
        int32_t state = 0U;

        if (handle != nullptr)
        {
            state = (static_cast<DFPlayerMini_PlayerMini *>(handle))->readState();
        }

        return state;
    }

    int32_t DFPlayerMini_ReadVolume(void *const handle)
    {
        int32_t volume = 0U;

        if (handle != nullptr)
        {
            volume = (static_cast<DFPlayerMini_PlayerMini *>(handle))->readVolume();
        }

        return volume;
    }

    int32_t DFPlayerMini_ReadEQ(void *const handle)
    {
        int32_t eq = 0U;

        if (handle != nullptr)
        {
            eq = (static_cast<DFPlayerMini_PlayerMini *>(handle))->readEQ();
        }

        return eq;
    }

    int32_t DFPlayerMini_ReadFileCounts(void *const handle, const uint8_t device)
    {
        int32_t fileCounts = 0U;

        if (handle != nullptr)
        {
            fileCounts = (static_cast<DFPlayerMini_PlayerMini *>(handle))->readFileCounts();
        }

        return fileCounts;
    }

    int32_t DFPlayerMini_ReadCurrentFileNumber(void *const handle, const uint8_t device)
    {
        int32_t currentFileNumber = 0U;

        if (handle != nullptr)
        {
            currentFileNumber = (static_cast<DFPlayerMini_PlayerMini *>(handle))->readCurrentFileNumber(device);
        }

        return currentFileNumber;
    }

    int32_t DFPlayerMini_ReadFileCountsInFolder(void *const handle, const int32_t folderNumber)
    {
        int32_t fileCountsInFolder = 0U;

        if (handle != nullptr)
        {
            fileCountsInFolder = (static_cast<DFPlayerMini_PlayerMini *>(handle))->readFileCountsInFolder(folderNumber);
        }

        return fileCountsInFolder;
    }

    int32_t DFPlayerMini_ReadFolderCounts(void *const handle)
    {
        int32_t folderCounts = 0U;

        if (handle != nullptr)
        {
            folderCounts = (static_cast<DFPlayerMini_PlayerMini *>(handle))->readFolderCounts();
        }

        return folderCounts;
    }
}