/*
 * @Author: Naiqian
 * @Date: 2020-11-02 19:16:18
 * @LastEditTime: 2020-11-11 00:47:31
 * @LastEditors: Naiqian
 * @Description: 
 */
#ifndef __ITCP_UTILS_H
#define __ITCP_UTILS_H

#include <cerrno>
#include <cstdio>
#include <ifaddrs.h>
#include <netpacket/packet.h>
#include <vector>
#include <bits/stdc++.h>
#include <core.h>
typedef unsigned short uint16_t;

struct eth_hdr_t{
    uint8_t dst[6];
    uint8_t src[6];
    uint16_t type;
    eth_hdr_t(const mac_t _dst, const mac_t _src, const uint16_t &_type){
        //dst = new uint8_t[6];
        //src = new uint8_t[6];
        memcpy(dst, _dst, 6);
        memcpy(src, _src, 6);
        type = _type;
    }
    eth_hdr_t(const eth_hdr_t & cp){
        //dst = new uint8_t[6];
        //src = new uint8_t[6];
        memcpy(dst, cp.dst, 6);
        memcpy(src, cp.src, 6);
        type = cp.type;
    }
    ~eth_hdr_t(){
        if(dst) delete[] dst;
        if(src) delete[] src;
    }
};

uint16_t change_Endian(uint16_t x);
#endif