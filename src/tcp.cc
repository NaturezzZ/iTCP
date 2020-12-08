/*
 * @Author: Naiqian
 * @Date: 2020-12-06 15:06:49
 * @LastEditTime: 2020-12-09 02:59:41
 * @LastEditors: Naiqian
 * @Description: 
 * @FilePath: /iTCP/src/tcp.cc
 */
#include "tcp.h"
#include "utils.h"
#include "ip.h"
#include "core.h"
#include <map>
//class mysocket_t;
using namespace std;
extern iTCP_kernel_t iTCP_kernel;

bool isSyn(uint16_t flag){
    return(flag & (1<<9));
}
bool isFin(uint16_t flag){
    return(flag & (1<<8));
}
bool isAck(uint16_t flag){
    return(flag & (1<<12));
}
uint16_t setSyn(uint16_t flag){
    return(flag | (1<<9));
}
uint16_t setFin(uint16_t flag){
    return(flag | (1<<8));
}
uint16_t setAck(uint16_t flag){
    return(flag | (1<<12));
}


int recvIPPacket(const void* buf, int len){
    
    uint8_t *ipHdr = new uint8_t[sizeof(ipHdr_t)];
    uint8_t *ipPayload = new uint8_t[len - sizeof(ipHdr_t)];
    memcpy(ipHdr, buf, sizeof(ipHdr_t));
    memcpy(ipPayload, buf+sizeof(ipHdr_t), len - sizeof(ipHdr_t));
    ipv4_t ipSrc = ((ipHdr_t*)ipHdr)->src;
    ipv4_t ipDst = ((ipHdr_t*)ipHdr)->dst;
    uint16_t portSrc = ((tcp_hdr_t*)ipPayload)->srcPort;
    uint16_t portDst = ((tcp_hdr_t*)ipPayload)->dstPort;
    uint16_t flag = ((tcp_hdr_t*)ipPayload)->flag;
    uint32_t seq = ((tcp_hdr_t*)ipPayload)->seq;
    uint32_t ack = ((tcp_hdr_t*)ipPayload)->ack;
    uint8_t *TCPPayload = (uint8_t*)ipPayload+sizeof(tcp_hdr_t);
    int TCPPayloadLen = len - sizeof(ipHdr_t) - sizeof(tcp_hdr_t);
    //cerr << "???? " << hex<< portSrc << endl;
    tuple4 * info = new tuple4;
    info->srcIP = ipSrc; info->dstIP = ipDst;
    info->srcPort = portSrc; info->dstPort = portDst;
    
    sockaddr* ipaddr = new sockaddr;
    ipaddr->sa_family = AF_INET;
    ((tuple4 *)(ipaddr->sa_data))->srcIP = ipDst;
    ((tuple4 *)(ipaddr->sa_data))->srcPort = portDst;
    ((tuple4 *)(ipaddr->sa_data))->dstIP = ipSrc;
    ((tuple4 *)(ipaddr->sa_data))->dstPort = portSrc;

    // memcpy((void*)(ipaddr->sa_data), (void*)info, sizeof(tuple4));
    
    int connfd = 100;
    // std::map<int, mysocket_t>::iterator it;
    // for(connfd = 100; connfd < 200; connfd++){
    //     if(iTCP_kernel.socket_set.find(connfd)==iTCP_kernel.socket_set.end()) continue;
    //     it = (iTCP_kernel.socket_set.find(connfd));
    //     sockaddr* addr = (*it).second.address;
    //     socklen_t len = (*it).second.address_len;
    //     if(memcmp((*it).second.address, ipaddr, len) == 0) break;
    // }
    //fprintf(stderr, "abcde\n");
    //std::map<int, BlockingQueue< std::pair<uint8_t[2000], int> > >::iterator it = iTCP_kernel.socket_queue.begin();
    if(isSyn(flag)&&(!isAck(flag))){
        std::map<int, mysocket_t*>::iterator it = iTCP_kernel.socket_set.begin();
        for(it; it!= iTCP_kernel.socket_set.end(); it++){
            if((*it).second->tcb->status != LISTEN) continue;
            if((((tuple4 *)((*it).second->address->sa_data))->srcIP == info->dstIP) && (((tuple4 *)((*it).second->address->sa_data))->srcPort == info->dstPort)){
                connfd = (*it).first;
                break;
            }
        }
        memcpy((*it).second->address->sa_data, ipaddr->sa_data, sizeof(tuple4));
    }
    else{
        std::map<int, mysocket_t*>::iterator it = iTCP_kernel.socket_set.begin();
        for(it; it != iTCP_kernel.socket_set.end(); it++){
            //cerr << "*****" << endl;
            //fprintf(stderr, "port%d\n", h2n(portSrc));
            //fprintf(stderr, "port%d\n", h2n(portDst));
            //fprintf(stderr, "11port%d\n", h2n(((tuple4*)(*it).second->address->sa_data)->srcPort));
            //fprintf(stderr, "11port%d\n", h2n(((tuple4*)(*it).second->address->sa_data)->dstPort));
            //cerr << "*****" << endl;
            if(memcmp((*it).second->address->sa_data, ipaddr->sa_data, sizeof(tuple4)) == 0){
                connfd = (*it).first;
                //fprintf(stderr, "abcde%d\n", connfd);
                break;
            }
        }
    }
    uint8_t *tmp = new uint8_t[2000];
    memcpy(tmp, buf, len);
    std::pair<uint8_t*, int>* p = new std::pair<uint8_t*, int>(tmp, len);
    (*(iTCP_kernel.socket_queue.find(connfd))).second->push(std::ref(*p));
    iTCP_kernel.unSolvedSocket.push(connfd);
    return connfd;
}

int readIPPacket(int fd){
    //fprintf(stderr, "readFd:%d\n", fd);
    uint8_t* buf = new uint8_t[2000];
    int len;
    std::pair<uint8_t*, int> tmp = (*(iTCP_kernel.socket_queue.find(fd))).second->pop();
    len = tmp.second;
    memcpy(buf, tmp.first, len);
    
    uint8_t *ipHdr = new uint8_t[sizeof(ipHdr_t)];
    uint8_t *ipPayload = new uint8_t[len - sizeof(ipHdr_t)];
    memcpy(ipHdr, buf, sizeof(ipHdr_t));
    memcpy(ipPayload, buf+sizeof(ipHdr_t), len - sizeof(ipHdr_t));
    ipv4_t ipSrc = ((ipHdr_t*)ipHdr)->src;
    ipv4_t ipDst = ((ipHdr_t*)ipHdr)->dst;
    uint16_t portSrc = ((tcp_hdr_t*)ipPayload)->srcPort;
    uint16_t portDst = ((tcp_hdr_t*)ipPayload)->dstPort;
    uint16_t flag = ((tcp_hdr_t*)ipPayload)->flag;
    uint32_t seq = ((tcp_hdr_t*)ipPayload)->seq;
    uint32_t ack = ((tcp_hdr_t*)ipPayload)->ack;
    seq = htonl(seq); ack = htonl(ack);
    uint8_t *TCPPayload = (uint8_t*)ipPayload+sizeof(tcp_hdr_t);
    int TCPPayloadLen = len - sizeof(ipHdr_t) - sizeof(tcp_hdr_t);

    tuple4 * info = new tuple4;
    info->srcIP = ipSrc; info->dstIP = ipDst;
    info->srcPort = portSrc; info->dstPort = portDst;
    
    sockaddr* ipaddr = new sockaddr;
    ipaddr->sa_family = AF_INET;
    //ipaddr->sa_len = 0;
    memcpy((void*)(ipaddr->sa_data), (void*)info, sizeof(tuple4));
    
    int connfd = fd;
    std::map<int, mysocket_t*>::iterator it = (iTCP_kernel.socket_set.find(connfd));

    if(((*it).second)->tcb->status == SYNSENT && ((*it).second)->tcb->role == TCP_SERVER){
        ((tuple4*)((*it).second->address->sa_data))->dstIP = info->srcIP;
        ((tuple4*)((*it).second->address->sa_data))->srcIP = info->dstIP;
        ((tuple4*)((*it).second->address->sa_data))->srcPort = info->dstPort;
        ((tuple4*)((*it).second->address->sa_data))->dstPort = info->srcPort;
    }
    
    // for(connfd = 100; connfd < 200; connfd++){
    //     if(iTCP_kernel.socket_set.find(connfd)==iTCP_kernel.socket_set.end()) continue;
    //     it = (iTCP_kernel.socket_set.find(connfd));
    //     sockaddr* addr = (*it).second.address;
    //     socklen_t len = (*it).second.address_len;
    //     if(memcmp((*it).second.address, ipaddr, len) == 0) break;
    // }
    //connfd has the same 4-tuple as recv packet
    //fprintf(stderr, "syn:%d,ack:%d\n", isSyn(flag), isAck(flag));
    if(isSyn(flag)&&isAck(flag)){
        if(((*it).second)->tcb->status == SYNSENT){
            uint8_t* tmp = new uint8_t;
            tuple4* sendinfo = new tuple4;
            sendinfo->dstIP = info->srcIP;
            sendinfo->srcIP = info->dstIP;
            sendinfo->srcPort = info->dstPort;
            sendinfo->dstPort = info->srcPort;

            //send back ack packet
            uint16_t flag = 0;
            flag = setAck(flag);
            sendTCPPacket(tmp, 1, sendinfo, ack, seq+1, flag);
            ((*it).second)->tcb->status = ESTAB;
            
            ((*it).second)->conn_ready = true;
            ((*it).second)->conn_cv.notify_all();
        }

    }
    else if(isSyn(flag)&&(!isAck(flag))){
        //fprintf(stderr, "status:%d\n", ((*it).second)->tcb->status);
        if(((*it).second)->tcb->status == LISTEN){
            uint8_t* tmp = new uint8_t;
            tuple4* sendinfo = new tuple4;
            sendinfo->dstIP = info->srcIP;
            sendinfo->srcIP = info->dstIP;
            sendinfo->srcPort = info->dstPort;
            sendinfo->dstPort = info->srcPort;
            uint16_t flag = 0;
            flag = setSyn(flag); flag = setAck(flag);
            sendTCPPacket(tmp, 1, sendinfo, ack, seq+1, flag);
            ((*it).second)->tcb->status = SYNRCVD;
            
            ((*it).second)->acc_syn_ready = true;
            ((*it).second)->acc_syn_cv.notify_all();
        }
        else if(((*it).second)->tcb->status == SYNSENT){
            uint8_t* tmp = new uint8_t;
            tuple4* sendinfo = new tuple4;
            sendinfo->dstIP = info->srcIP;
            sendinfo->srcIP = info->dstIP;
            sendinfo->srcPort = info->dstPort;
            sendinfo->dstPort = info->srcPort;
            uint16_t flag = 0;
            flag = setAck(flag);
            sendTCPPacket(tmp, 1, sendinfo, ack, seq+1, flag);
            ((*it).second)->tcb->status = SYNRCVD;
        }
    }
    else if(isFin(flag)){
        if(((*it).second)->tcb->status == ESTAB){
            uint8_t* tmp = new uint8_t;
            tuple4* sendinfo = new tuple4;
            sendinfo->dstIP = info->srcIP;
            sendinfo->srcIP = info->dstIP;
            sendinfo->srcPort = info->dstPort;
            sendinfo->dstPort = info->srcPort;
            uint16_t flag = 0;
            flag = setAck(flag);
            sendTCPPacket(tmp, 1, sendinfo, ack, seq+1, flag);
            flag = 0; flag = setFin(flag);
            sendTCPPacket(tmp, 1, sendinfo, ack, seq+1, flag);
            
            ((*it).second)->tcb->status = LASTACK;
        }
        else if(((*it).second)->tcb->status == FINWAIT1){
            uint8_t* tmp = new uint8_t;
            tuple4* sendinfo = new tuple4;
            sendinfo->dstIP = info->srcIP;
            sendinfo->srcIP = info->dstIP;
            sendinfo->srcPort = info->dstPort;
            sendinfo->dstPort = info->srcPort;
            uint16_t flag = 0;
            flag = setAck(flag);
            sendTCPPacket(tmp, 1, sendinfo, ack, seq+1, flag);
            ((*it).second)->tcb->status = CLOSING;
        }
        else if(((*it).second)->tcb->status == FINWAIT2){
            uint8_t* tmp = new uint8_t;
            tuple4* sendinfo = new tuple4;
            sendinfo->dstIP = info->srcIP;
            sendinfo->srcIP = info->dstIP;
            sendinfo->srcPort = info->dstPort;
            sendinfo->dstPort = info->srcPort;
            uint16_t flag = 0;
            flag = setAck(flag);
            sendTCPPacket(tmp, 1, sendinfo, ack, seq+1, flag);
            ((*it).second)->tcb->status = TIMEWAIT;
        }
    }
    else if(isAck(flag)){
        if (((*it).second)->tcb->status == SYNRCVD){
            ((*it).second)->tcb->status = ESTAB;
            ((*it).second)->acc_ack_ready = true;
            ((*it).second)->acc_ack_cv.notify_all();
        }
        else if(((*it).second)->tcb->status == FINWAIT1){
            ((*it).second)->tcb->status = FINWAIT2;
        }
        else if(((*it).second)->tcb->status == CLOSING){
            ((*it).second)->tcb->status = TIMEWAIT;
        }
        else if(((*it).second)->tcb->status == LASTACK){
            ((*it).second)->tcb->status = CLOSED;
        }
        else if(((*it).second)->tcb->status == ESTAB){
            recvTCPPacket(it->first, TCPPayload, TCPPayloadLen, info, seq, ack, flag);
        }
    }
    else{
        recvTCPPacket((*it).first, TCPPayload, TCPPayloadLen, info, seq, ack, flag);
    }
}

int sendTCPPacket(void* buf, int len, const tuple4* info, 
    uint32_t seq, uint32_t ack, uint16_t flag){
    tcp_hdr_t* tcpHdr = new tcp_hdr_t;
    ack = ntohl(ack); seq = ntohl(seq);
    memset(tcpHdr, 0, sizeof(tcpHdr));
    tcpHdr->ack = ack;
    tcpHdr->seq = seq;
    tcpHdr->flag = flag;
    tcpHdr->dstPort = info->dstPort;
    tcpHdr->srcPort = info->srcPort;
    uint8_t* payload = new uint8_t[len+sizeof(tcp_hdr_t)];
    memcpy(payload, tcpHdr, sizeof(tcp_hdr_t));
    //fprintf(stderr, "testout:%x", )
    memcpy(payload+sizeof(tcp_hdr_t), buf, len);
    // ipHdr_t* ipHdr = new ipHdr_t;
    // memset(ipHdr, 0, sizeof(ipHdr_t));
    // ipHdr->ttl = 10;
    // ipHdr->protocol = 0x6;
    // ipHdr->src = info->srcIP;
    // ipHdr->dst = info->dstIP;

    in_addr srcin, dstin;
    srcin.s_addr = info->srcIP;
    dstin.s_addr = info->dstIP;
    //fprintf(stderr, "backpacket,src:%x,dst:%x\n", info->srcIP, info->dstIP);
    sendIPPacket(srcin, dstin, 0x6, payload, len+sizeof(tcp_hdr_t), 10);
    return 0;
}

int recvTCPPacket(int fd, void* buf, int len, const tuple4* info,
    uint32_t seq, uint32_t ack, uint16_t flag){
    //fprintf(stderr, "getpacket?\n");
    map<int, mysocket_t*>::iterator it = (iTCP_kernel.socket_set.find(fd));
    (*it).second->data_mtx.lock();
    // ack = ntohl(ack);
    // seq = ntohl(seq);

    if(isAck(flag)){
        if(it->second->role == TCP_SERVER){
            (*it).second->data_mtx.unlock();
            return 0;
        }
        //fprintf(stderr, "whathappened?\n");
        int leng = 1000; int sizep = (*it).second->sizep;
        if(ack >= sizep){
            it->second->writep = ack;
            (*it).second->data_mtx.unlock();
            return 0;
        }
        if(sizep - ack < leng) leng = sizep - ack;
        
        tuple4* ninfo = new tuple4;
        ninfo->srcIP = info->dstIP;
        ninfo->srcPort = info->dstPort;
        ninfo->dstIP = info->srcIP;
        ninfo->dstPort = info->srcPort;
        it->second->writep = ack;
        sendTCPPacket((*it).second->databuf+ack, leng, ninfo, ack, seq+len, (uint16_t)0);
    }
    else{
        memcpy((*it).second->databuf+seq, buf, len);
        //fprintf(stderr, "qwerty:%d,%d,%d\n", seq , len , (*it).second->sizep);
        if(seq + len > (*it).second->sizep){
            (*it).second->sizep = seq+len;
            (*it).second->read_cv.notify_all();
        }
        uint16_t flag = 0;
        flag = setAck(flag);
        tuple4* ninfo = new tuple4;
        ninfo->srcIP = info->dstIP;
        ninfo->srcPort = info->dstPort;
        ninfo->dstIP = info->srcIP;
        ninfo->dstPort = info->srcPort;
        uint8_t *tmp = new uint8_t;
        sendTCPPacket(tmp, 1, ninfo, ack, seq+len, flag);
        //fprintf(stderr, "aftersend\n");
    }
    (*it).second->data_mtx.unlock();
    
    //fprintf(stderr, "[HINT] sip:%x,dip:%x,seq:%d,ack:%d\n", info->srcIP, info->dstIP, seq, ack);
    return 0;
}