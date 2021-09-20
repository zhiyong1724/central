#include "norflash.h"
#include "quadspi.h"
#define ENABLE_RESET_COMMAND 0x66
#define RESET_COMMAND 0x99
#define WRITE_ENABLE_COMMAND 0x06
#define WRITE_DISABLE_COMMAND 0x04
#define READ_STATUS_REGISTER_1_COMMAND 0x05
#define WRITE_STATUS_REGISTER_1_COMMAND 0x01
#define READ_STATUS_REGISTER_2_COMMAND 0x35
#define WRITE_STATUS_REGISTER_2_COMMAND 0x31
#define READ_STATUS_REGISTER_3_COMMAND 0x15
#define WRITE_STATUS_REGISTER_3_COMMAND 0x11
#define SECTOR_ERASE_COMMAND 0x20
#define PAGE_PROGRAM_COMMAND 0x02
#define READ_DATA_COMMAND 0xeb

static uint8_t readStatusRegister(uint8_t id)
{
    uint8_t value = 0;
    uint32_t command = 0;
    switch (id)
    {
    case 1:
        command = READ_STATUS_REGISTER_1_COMMAND;
        break;
    case 2:
        command = READ_STATUS_REGISTER_2_COMMAND;
        break;
    case 3:
        command = READ_STATUS_REGISTER_3_COMMAND;
        break;
    default:
        break;
    }
    MX_QUADSPI_Command(command, 0, 0, 0, 1, (0x01 << 0) | (0x02 << 2) | (0x00 << 4) | (0x00 << 6) | (0x01 << 8) | (0x00 << 10));
    MX_QUADSPI_Receive(&value);
    return value;
}

static void waitBusy()
{
    while (1)
    {
        if (0 == (readStatusRegister(1) & 0x01))
        {
            break;
        }
    }
}

static void writeEnable()
{
    MX_QUADSPI_Command(WRITE_ENABLE_COMMAND, 0, 0, 0, 0, (0x01 << 0) | (0x02 << 2) | (0x00 << 4) | (0x00 << 6) | (0x00 << 8) | (0x00 << 10));
}

/* static void writeStatusRegister(uint8_t id, uint8_t value)
{
    writeEnable();
    uint32_t command = 0;
    switch (id)
    {
    case 1:
        command = WRITE_STATUS_REGISTER_1_COMMAND;
        break;
    case 2:
        command = WRITE_STATUS_REGISTER_2_COMMAND;
        break;
    case 3:
        command = WRITE_STATUS_REGISTER_3_COMMAND;
        break;
    default:
        break;
    }
    MX_QUADSPI_Command(command, 0, 0, 0, 1, (0x01 << 0) | (0x02 << 2) | (0x00 << 4) | (0x00 << 6) | (0x01 << 8) | (0x00 << 10));
    MX_QUADSPI_Transmit(&value);
} */

static void reset()
{
    MX_QUADSPI_Command(ENABLE_RESET_COMMAND, 0, 0, 0, 0, (0x01 << 0) | (0x02 << 2) | (0x00 << 4) | (0x00 << 6) | (0x00 << 8) | (0x00 << 10));
    MX_QUADSPI_Command(RESET_COMMAND, 0, 0, 0, 0, (0x01 << 0) | (0x02 << 2) | (0x00 << 4) | (0x00 << 6) | (0x00 << 8) | (0x00 << 10));
}

void norflashInit()
{
    MX_QUADSPI_Init();  
    MX_Disable_Qspi();
    reset();  
    HAL_Delay(100);
}

void norflashSectorErase(uint32_t address)
{
    MX_Disable_Qspi();
    writeEnable();
    waitBusy();
    MX_QUADSPI_Command(SECTOR_ERASE_COMMAND, address, 0, 0, 0, (0x01 << 0) | (0x02 << 2) | (0x01 << 4) | (0x00 << 6) | (0x00 << 8) | (0x00 << 10));
}

void norflashWriteData(uint32_t address, uint8_t *data, uint32_t size)
{
    MX_Disable_Qspi();
    writeEnable();
    waitBusy();
    MX_QUADSPI_Command(PAGE_PROGRAM_COMMAND, address, 0, 0, size, (0x01 << 0) | (0x02 << 2) | (0x01 << 4) | (0x00 << 6) | (0x01 << 8) | (0x00 << 10));
    MX_QUADSPI_Transmit(data);
}

void norflashReadData(uint32_t address, uint8_t *data, uint32_t size)
{
    MX_Enable_Qspi();
    waitBusy();
    MX_QUADSPI_Command(READ_DATA_COMMAND, address, 0, 1, size, (0x01 << 0) | (0x02 << 2) | (0x03 << 4) | (0x03 << 6) | (0x03 << 8) | (0x04 << 10));
    MX_QUADSPI_Receive(data);
}

void norflashMemoryMapped()
{
    MX_Enable_Qspi();
    waitBusy();
    MX_QUADSPI_MemoryMapped(READ_DATA_COMMAND, 0, 1, (0x01 << 0) | (0x02 << 2) | (0x03 << 4) | (0x03 << 6) | (0x03 << 8) | (0x04 << 10));
}