#include "norflash.h"
#include "quadspi.h"
#define WRITE_ENABLE_COMMAND 0X06
#define WRITE_DISABLE_COMMAND 0X04
#define READ_STATUS_REGISTER_1_COMMAND 0X05
#define WRITE_STATUS_REGISTER_1_COMMAND 0X01
#define READ_STATUS_REGISTER_2_COMMAND 0X35
#define WRITE_STATUS_REGISTER_2_COMMAND 0X31
#define READ_STATUS_REGISTER_3_COMMAND 0X15
#define WRITE_STATUS_REGISTER_3_COMMAND 0X11
#define SECTOR_ERASE_COMMAND 0X20
#define PAGE_PROGRAM_COMMAND 0X32
#define READ_DATA_COMMAND 0Xeb
static void writeEnable()
{
    MX_QUADSPI_Command(WRITE_ENABLE_COMMAND, 0, 0, 0, 0, (0x01 << 0) | (0x02 << 2) | (0x00 << 4) | (0x00 << 6) | (0x00 << 8) | (0x00 << 10));
}

static void writeDisable()
{
    MX_QUADSPI_Command(WRITE_DISABLE_COMMAND, 0, 0, 0, 0, (0x01 << 0) | (0x02 << 2) | (0x00 << 4) | (0x00 << 6) | (0x00 << 8) | (0x00 << 10));
}

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

static void writeStatusRegister(uint8_t id, uint8_t value)
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
    writeDisable();
}

void norflashInit()
{
    MX_QUADSPI_Init();
    uint8_t value = readStatusRegister(2);
    if (0 == (value & 0x02))
    {
        value |= 0x02;
        writeStatusRegister(1, value);                //激活qspi模式
    }
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

void norflashSectorErase(uint32_t address)
{
    writeEnable();
    MX_QUADSPI_Command(SECTOR_ERASE_COMMAND, address, 0, 0, 0, (0x01 << 0) | (0x02 << 2) | (0x01 << 4) | (0x00 << 6) | (0x00 << 8) | (0x00 << 10));
    waitBusy();
    writeDisable();
}

void norflashWriteData(uint32_t address, uint8_t *data, uint32_t size)
{
    writeEnable();
    MX_QUADSPI_Command(PAGE_PROGRAM_COMMAND, address, 0, 0, size, (0x01 << 0) | (0x02 << 2) | (0x01 << 4) | (0x00 << 6) | (0x03 << 8) | (0x00 << 10));
    MX_QUADSPI_Transmit(data);
    waitBusy();
    writeDisable();
}

void norflashReadData(uint32_t address, uint8_t *data, uint32_t size)
{
    MX_QUADSPI_Command(READ_DATA_COMMAND, address, 0, 1, size, (0x01 << 0) | (0x02 << 2) | (0x03 << 4) | (0x03 << 6) | (0x03 << 8) | (0x04 << 10));
    MX_QUADSPI_Receive(data);
}

void norflashMemoryMapped()
{
    MX_QUADSPI_MemoryMapped(READ_DATA_COMMAND, 0, 1, (0x01 << 0) | (0x02 << 2) | (0x03 << 4) | (0x03 << 6) | (0x03 << 8) | (0x04 << 10));
}