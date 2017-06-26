//
// Created by ivan on 18/06/17.
//

#ifndef CONTROL_EEPROM_H
#define CONTROL_EEPROM_H

#include "stdint.h"
#include "chip.h"
#include "stddef.h"
#include "sapi.h"

#include "modem.h"
#include "webservice.h"

#define EEPROM_ADDRESS_CHECK 0
#define EEPROM_ADDRESS_APN 1
#define EEPROM_ADDRESS_APN_USERNAME EEPROM_ADDRESS_APN + MODEM_APN_MAX_SIZE
#define EEPROM_ADDRESS_APN_PASSWORD EEPROM_ADDRESS_APN_USERNAME + MODEM_USERNAME_MAX_SIZE
#define EEPROM_ADDRESS_SERVER_URL EEPROM_ADDRESS_APN_PASSWORD + MODEM_PASSWORD_MAX_SIZE
#define EEPROM_ADDRESS_SERVER_PATH EEPROM_ADDRESS_SERVER_URL + SERVER_URL_MAX_SIZE
#define EEPROM_ADDRESS_SERVER_PORT EEPROM_ADDRESS_SERVER_PATH + SERVER_PATH_MAX_SIZE
#define EEPROM_ADDRESS_STATION_ID EEPROM_ADDRESS_SERVER_PORT + 4

#define EEPROM_CHECK_TOKEN 0xEE

void Board_EEPROM_init(void);
bool_t Board_EEPROM_writeByte(uint32_t addr,uint8_t value);
bool_t Board_EEPROM_writeData(uint32_t addr,uint8_t* data, size_t dataSize);
uint8_t Board_EEPROM_readByte(uint32_t addr);
bool_t Board_EEPROM_readData(uint32_t addr,uint8_t* data, size_t dataSize);
bool_t Board_EEPROM_clearData(uint32_t addr, size_t dataSize);

#endif //CONTROL_EEPROM_H
