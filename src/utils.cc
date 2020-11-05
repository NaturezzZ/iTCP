/*
 * @Author: Naiqian
 * @Date: 2020-11-04 22:35:40
 * @LastEditTime: 2020-11-04 22:39:33
 * @LastEditors: Naiqian
 * @Description: 
 */
#include "utils.h"

uint16_t change_Endian(uint16_t x){
    uint16_t mask = 0xff;
    uint16_t result = ((x & mask)<<8) + ((x & (mask<<8))>>8);
    return result;
}