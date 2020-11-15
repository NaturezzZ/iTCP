/*
 * @Author: Naiqian
 * @Date: 2020-11-14 00:37:06
 * @LastEditTime: 2020-11-14 15:00:57
 * @LastEditors: Naiqian
 * @Description: 
 * @FilePath: /iTCP/src/forward.cc
 */
#include "forward.h"
/* @brief
 *
 */
int forward(const ipv4_t src, const ipv4_t dest, int proto, const void *buf, int len, int ttl){
    in_addr Src; Src.s_addr = src;
    in_addr Dst; Dst.s_addr = dest;
    if (ttl == 0) return -1;
    int ret = sendIPPacket(Src, Dst, proto, buf, len, ttl-1);
    return ret;
}