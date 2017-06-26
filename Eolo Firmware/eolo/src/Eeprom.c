
#include "Eeprom.h"

// sacado de https://github.com/martinribelotta/micropython/blob/master/ciaa-nxp/board_ciaa_edu_4337/src/board.c
// ej de uso https://github.com/martinribelotta/micropython/blob/master/ciaa-nxp/testing/testsEEPROM.c

void Board_EEPROM_init(void)
{
    Chip_EEPROM_Init(LPC_EEPROM);
    /* Set Auto Programming mode */
    Chip_EEPROM_SetAutoProg(LPC_EEPROM,EEPROM_AUTOPROG_AFT_1WORDWRITTEN);
}

bool_t Board_EEPROM_writeByte(uint32_t addr,uint8_t value)
{
    if(addr>=(16*1024))
        return false;

    uint32_t addr4 = addr/4;
    uint32_t pageAddr = addr4/EEPROM_PAGE_SIZE;
    uint32_t pageOffset = addr4 - pageAddr*EEPROM_PAGE_SIZE;

    uint32_t *pEepromMem = (uint32_t*)EEPROM_ADDRESS(pageAddr,pageOffset*4);

    // read 4 bytes in auxValue
    uint32_t auxValue = pEepromMem[0];
    uint8_t* pAuxValue = (uint8_t*)&auxValue;

    // modify auxValue with new Byte value
    uint32_t indexInBlock = addr % 4;
    pAuxValue[indexInBlock] = value;

    //write auxValue back in eeprom
    pEepromMem[0] = auxValue;
    Chip_EEPROM_WaitForIntStatus(LPC_EEPROM, EEPROM_INT_ENDOFPROG);
    return true;
}

bool_t Board_EEPROM_clearData(uint32_t addr, size_t dataSize){
    if(addr == NULL || dataSize == 0){
        return false;
    }
    uint32_t i = 0;
    uint32_t lastAddr = addr + (uint32_t) dataSize - 1;
    if(lastAddr >=(16*1024)){
        return false;
    }

    for (; i < dataSize; ++i, ++addr) {
        if(!Board_EEPROM_writeByte(addr, 0)){
            return false;
        }
    }
    return true;
}

bool_t Board_EEPROM_writeData(uint32_t addr,uint8_t* data, size_t dataSize){
    if(addr == NULL || data == NULL || dataSize == 0){
        return false;
    }
    uint32_t i = 0;
    uint32_t lastAddr = addr + (uint32_t) dataSize - 1;
    if(lastAddr >=(16*1024)){
        return false;
    }

    for (; i < dataSize; ++i, ++addr) {
        if(!Board_EEPROM_writeByte(addr, data[i])){
            return false;
        }
    }
    return true;
}

uint8_t Board_EEPROM_readByte(uint32_t addr)
{
    if(addr>=(16*1024))
        return 0;

    uint32_t addr4 = addr/4;
    uint32_t pageAddr = addr4/EEPROM_PAGE_SIZE;
    uint32_t pageOffset = addr4 - pageAddr*EEPROM_PAGE_SIZE;

    uint32_t *pEepromMem = (uint32_t*)EEPROM_ADDRESS(pageAddr,pageOffset*4);

    // read 4 bytes in auxValue
    uint32_t auxValue = pEepromMem[0];
    uint8_t* pAuxValue = (uint8_t*)&auxValue;

    uint32_t indexInBlock = addr % 4;
    return (uint8_t) pAuxValue[indexInBlock];

}

bool_t Board_EEPROM_readData(uint32_t addr,uint8_t* data, size_t dataSize){
    uint32_t lastAddr = addr + (uint32_t) dataSize - 1;
    if(data == NULL || lastAddr >=(16*1024)){
        return false;
    }
    uint32_t i = 0;
    for (; i < dataSize; ++i, ++addr) {
        data[i] = Board_EEPROM_readByte(addr);
    }
    return true;
}