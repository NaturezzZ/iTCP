/*
 * @Author: Naiqian
 * @Date: 2020-11-02 19:16:18
 * @LastEditTime: 2020-11-15 12:22:10
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
#include <mutex>
#include <thread>
#include <ctime>
#include <chrono>
#include <unistd.h>

//#include <core.h>

#define L2_IPV4 0x0800
#define L3_MYARP 0x8f
typedef unsigned short uint16_t;
#define ETHER 0
#define IPV4 1
#define IPV6 2
typedef std::string deviceName_t;

//using mac_t = uint8_t*;

typedef uint8_t* mac_t;
typedef uint32_t deviceID_t;
typedef uint32_t ipv4_t;
typedef std::pair<ipv4_t, ipv4_t> rtKey_t;

struct rtValue_t{
    deviceID_t deviceID;
    mac_t dstMAC;
    int ttl;
    rtValue_t(){
        dstMAC = new uint8_t[6];
        deviceID = -1;
        ttl = 0;
    }
    rtValue_t(const rtValue_t & cp){
        deviceID = cp.deviceID;
        dstMAC = new uint8_t[6];
        ttl = cp.ttl;
        memcpy(dstMAC, cp.dstMAC, 6);
    }    
    ~rtValue_t(){
        if(dstMAC) delete[] dstMAC;
    }
};

//#define rtTable_t std::map<rtKey_t, rtValue_t>
typedef std::map<rtKey_t, rtValue_t> rtTable_t;


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

struct ipHdr_t{
    uint32_t part1;
    uint32_t part2;
    uint8_t ttl;
    uint8_t protocol;
    uint16_t hdrCRC;
    uint32_t src;
    uint32_t dst;
};

uint16_t change_Endian(uint16_t x);

#include <condition_variable>
#include <iostream>
#include <mutex>
#include <random>
#include <thread>
#include <vector>

template <typename T>
class BlockingQueue {
    std::mutex              _mutex;
    std::condition_variable _not_full;
    std::condition_variable _not_empty;
    int                     _start;
    int                     _end;
    int                     _capacity;
    std::vector<T>          _vt;

    public:
    BlockingQueue(const BlockingQueue<T>& other) = delete;
    BlockingQueue<T>& operator=(const BlockingQueue<T>& other) = delete;
    BlockingQueue(int capacity) : _capacity(capacity), _vt(capacity + 1), _start(0), _end(0) {}

    bool isempty() {
        return _end == _start;
    }

    bool isfull() {
        return (_start + _capacity - _end) % (_capacity + 1) == 0;
    }

    void push(const T& e) {
        std::unique_lock<std::mutex> lock(_mutex);
        while (isfull()) {
            _not_full.wait(lock);
        }

        _vt[_end++] = e;
        _end %= (_capacity + 1);
        _not_empty.notify_one();
    }

    T pop() {
        std::unique_lock<std::mutex> lock(_mutex);
        while (isempty()) {
            _not_empty.wait(lock);
        }

        auto res = _vt[_start++];
        _start %= (_capacity + 1);
        _not_full.notify_one();
        return res;
    }
};

struct recvPcap{
    uint8_t packet[16*1024];
    uint32_t len;
    deviceID_t ID;
    recvPcap(){
        len = 0;
        ID = 1024;
    }
    recvPcap(const recvPcap & cp){
        len = cp.len;
        memcpy(packet, cp.packet, len);
        ID = cp.ID;
    }
    recvPcap & operator=(const recvPcap &cp){
        len = cp.len;
        memcpy(packet, cp.packet, len);
        ID = cp.ID;
        return *this;
    }
};
#endif