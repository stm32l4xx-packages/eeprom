/*
 * eeprom.h
 *
 *  Created on: Dec 11, 2020
 *      Author: jsilva
 */

#ifndef INC_EEPROM_H_
#define INC_EEPROM_H_

/*
#include "package.h"
#include "logging.h"
*/
#define AT24_DEV_ADDR          (0xA0) // 24AA02 2K EEPROM using i2c Address
                                  // for the 22AA025 variant this can be 0xA0,0xA2,0xA4,0xA6,0xA8,0xAA,0xAC,0xAE
                                  // since LSB is used to to control a WRITE(0) or a READ(1) Operation
#define AT24_DEV_ADDR_WRITE    (0x00) // 24AA02 2K EEPROM using i2c Write bit
#define AT24_DEV_ADDR_READ     (0x01) // 24AA02 2K EEPROM using i2c Read bit

                                  // Information from Microchip Datasheet DS20005202A Part #24AA02UID/24AA025UID
uint8_t read_byte_i2c(uint8_t partition, uint16_t address);
bool write_byte_i2c(uint8_t partition, uint16_t address,uint8_t byte);
bool write_sequential_i2c(uint8_t partition, uint16_t address, uint8_t* data, uint8_t len);
bool read_sequential_i2c(uint8_t partition, uint16_t address, uint8_t* data, uint8_t len);

#endif /* INC_EEPROM_H_ */
