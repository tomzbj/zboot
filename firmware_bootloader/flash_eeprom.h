/// @file flash_eeprom.h
/// @brief 
/// @author Zhang Hao, stavrosatic@gmail.com
/// @version R0.1
/// @date 2017-10-12
#ifndef _FLASH_EEPROM_H
#define _FLASH_EEPROM_H

#define EEPROM_Config     FLASH_EEPROM_Config
#define EEPROM_Config     FLASH_EEPROM_Config
#define EEPROM_WriteWord  FLASH_EEPROM_WriteWord
#define EEPROM_ReadWord   FLASH_EEPROM_ReadWord
#define EEPROM_WriteData  FLASH_EEPROM_WriteData
#define EEPROM_ReadData   FLASH_EEPROM_ReadData

void FLASH_EEPROM_Config(unsigned long base_addr, unsigned short page_size);
void FLASH_EEPROM_WriteWord(unsigned short addr, unsigned short data);
unsigned short FLASH_EEPROM_ReadWord(unsigned short addr);

void FLASH_EEPROM_WriteData(unsigned short addr, void* data, int num);
void FLASH_EEPROM_ReadData(unsigned short addr, void* data, int num);

unsigned short FLASH_EEPROM_GetSize(void);
void FLASH_EEPROM_EraseAll(void);

#endif
