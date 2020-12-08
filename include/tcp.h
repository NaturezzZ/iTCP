/*
 * @Author: Naiqian
 * @Date: 2020-12-04 20:52:08
 * @LastEditTime: 2020-12-08 20:51:40
 * @LastEditors: Naiqian
 * @Description: 
 * @FilePath: /iTCP/include/tcp.h
 */

#ifndef __ITCP_TCP_H
#define __ITCP_TCP_H
#include "utils.h"

#define CLOSED 0
#define LISTEN 1
#define SYNSENT 2
#define SYNRCVD 3
#define ESTAB 4
#define FINWAIT1 5
#define FINWAIT2 6
#define CLOSEWAIT 7
#define LASTACK 8
#define CLOSING 9
#define TIMEWAIT 10

bool isSyn(uint16_t flag);
bool isFin(uint16_t flag);
bool isAck(uint16_t flag);
uint16_t setSyn(uint16_t flag);
uint16_t setFin(uint16_t flag);
uint16_t setAck(uint16_t flag);

struct TCB{
    int role;
    int status;

    TCB(int _role, int _status){
        role = _role;
        status = _status;
    }
    TCB(){
        role = -1;
        status = -1;
    }
    void set_state(int state){
        status = state;
    }
    int get_state(){
        return status;
    }
};

struct tuple4{
    uint32_t srcIP;
    uint32_t dstIP;
    uint16_t srcPort;
    uint16_t dstPort;
};

struct tcp_hdr_t{
    uint16_t srcPort;
    uint16_t dstPort;
    uint32_t seq;
    uint32_t ack;
    uint16_t flag;
    uint16_t size;
    uint32_t other;
};

int recvTCPPacket(int fd, void* buf, int len, const tuple4* info,
    uint32_t seq, uint32_t ack, uint16_t flag);
int sendTCPPacket(void* buf, int len, const tuple4* info, 
    uint32_t seq, uint32_t ack, uint16_t flag);
int readIPPacket(int fd);
int recvIPPacket(const void* buf, int len);

#endif