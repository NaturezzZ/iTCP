/*
 * @Author: Naiqian
 * @Date: 2020-12-04 13:27:38
 * @LastEditTime: 2020-12-09 04:25:04
 * @LastEditors: Naiqian
 * @Description: 
 * @FilePath: /iTCP/src/socket.cc
 */
#include "core.h"
#include "socket.h"
#include "tcp.h"
#include <map>
/**
 * @see [POSIX.1-2017:socket](http://pubs.opengroup.org/onlinepubs/
 * 9699919799/functions/socket.html)
 */
extern iTCP_kernel_t iTCP_kernel;
int __wrap_socket(int domain, int type, int protocol){
    return iTCP_kernel.socket_create(domain, type, protocol);
}

/**
 * @see [POSIX.1-2017:bind](http://pubs.opengroup.org/onlinepubs/
 * 9699919799/functions/bind.html)
 */
 int __wrap_bind(int socket, const struct sockaddr *address,
    socklen_t address_len){
    return iTCP_kernel.socket_bind(socket, address, address_len);
}

/**
 * @see [POSIX.1-2017:listen](http://pubs.opengroup.org/onlinepubs/
 * 9699919799/functions/listen.html)
 */
int __wrap_listen(int socket, int backlog){
    if(backlog != 1) return -1;
    return iTCP_kernel.socket_listen(socket);
}

/**
 * @see [POSIX.1-2017:connect](http://pubs.opengroup.org/onlinepubs/
 * 9699919799/functions/connect.html)
 */
int __wrap_connect(int socket, const struct sockaddr *address,
    socklen_t address_len){
    
    uint16_t flag = 0;
    flag = setSyn(flag);
    uint8_t* tmp = new uint8_t[1];
    tuple4* info = new tuple4;
    memcpy((void*)info, (void*)(address->sa_data), sizeof(tuple4));
    
    std::map<int, mysocket_t*>::iterator it = iTCP_kernel.socket_set.find(socket);
    (*it).second->tcb->role = TCP_CLIENT;
    (*it).second->tcb->status = SYNSENT;

    memcpy((void*)((*it).second->address), address, address_len);
    memcpy((void*)((*it).second->address->sa_data), address->sa_data, sizeof(tuple4));
    //fprintf(stderr, "<<??>>\n");
    int starttime = clock();

    std::unique_lock<std::mutex> lck((*it).second->conn_mtx);
    sendTCPPacket(tmp, 1, info, 0, 0, flag);
    //fprintf(stderr, "%x,%x,%x,%x\n", ((tuple4*)((*it).second->address->sa_data))->srcIP, ((tuple4*)((*it).second->address->sa_data))->srcPort, ((tuple4*)((*it).second->address->sa_data))->dstIP, ((tuple4*)((*it).second->address->sa_data))->srcPort);
    while( !( (*it).second->conn_ready ) ){
        (*it).second->conn_cv.wait(lck);
    }

    fprintf(stderr, "[INFO] Connection established on sender\n");
    return 0;
}

/**
 * @see [POSIX.1-2017:accept](http://pubs.opengroup.org/onlinepubs/
 * 9699919799/functions/accept.html)
 */

//build a socket
//wait for a syn
//wait for a ack

int __wrap_accept(int socket, struct sockaddr *address,
    socklen_t *address_len){
    std::map<int, mysocket_t*>::iterator it = iTCP_kernel.socket_set.find(socket);
    int connfd = 100;
    for(connfd; connfd < 200; connfd++){
        if(iTCP_kernel.socket_set.find(connfd)==iTCP_kernel.socket_set.end()) break;
    }
    
    mysocket_t* connsock = new mysocket_t( (*(iTCP_kernel.socket_set.find(socket))).second);
    memcpy((tuple4*)(connsock->address->sa_data), ((tuple4*)(*(iTCP_kernel.socket_set.find(socket))).second->address->sa_data), sizeof(tuple4));
    //fprintf(stderr, "debug:%x", ((tuple4*)(connsock->address->sa_data))->srcPort);
    BlockingQueue<std::pair<uint8_t*, int>> *tmp = new BlockingQueue<std::pair<uint8_t*, int>>(2000);
    iTCP_kernel.socket_set.insert(std::pair<int, mysocket_t*>(connfd, connsock));
    iTCP_kernel.socket_queue.insert(std::pair<int, BlockingQueue<std::pair<uint8_t*, int>>*>(connfd, tmp));
    
    // std::map<int, mysocket_t>::iterator it = iTCP_kernel.socket_set.find(connfd);
    it = iTCP_kernel.socket_set.find(connfd);
    std::map<int, mysocket_t*>::iterator oldit = iTCP_kernel.socket_set.find(socket);
    
    (*oldit).second->tcb->status = CLOSED;
    (*it).second->tcb->status = LISTEN;

    std::unique_lock<std::mutex> lck((*it).second->acc_syn_mtx);
    while( !( (*it).second->acc_syn_ready ) ){
        (*it).second->acc_syn_cv.wait(lck);
    }
    std::unique_lock<std::mutex> lck1((*it).second->acc_ack_mtx);
    while( !( (*it).second->acc_ack_ready ) ){
        (*it).second->acc_ack_cv.wait(lck1);
    }
    //fprintf(stderr, "p2 finished\n");
    (*oldit).second->tcb->status = LISTEN;
    return connfd;
}

/**
 * @see [POSIX.1-2017:read](http://pubs.opengroup.org/onlinepubs/
 * 9699919799/functions/read.html)
 */
ssize_t __wrap_read(int fildes, void *buf, size_t nbyte){
    std::map<int, mysocket_t*>::iterator it = iTCP_kernel.socket_set.find(fildes);
    //fprintf(stderr, "??????????\n");
    int res =  (*it).second->myread(buf, nbyte);
    fprintf(stderr, "[INFO] read completed\n");
    return res;
}

/**
 * @see [POSIX.1-2017:write](http://pubs.opengroup.org/onlinepubs/
 * 9699919799/functions/write.html)
 */
ssize_t __wrap_write(int fildes, const void *buf, size_t nbyte){
    std::map<int, mysocket_t*>::iterator it = iTCP_kernel.socket_set.find(fildes);
    
    it->second->mywrite(buf, nbyte);
    (*it).second->data_mtx.lock();
    //fprintf(stderr, "qwe,%d\n", it->second->writep);
    
    int Len = 1000;
    if(it->second->writep+Len > it->second->sizep) Len = (it->second->sizep) - (it->second->writep);
    uint32_t seq = it->second->writep;
    //it->second->writep += Len;
    uint32_t ack = 0;
    //fprintf(stderr, "qwe,%d\n", it->second->writep);
    //seq = htonl(seq);
    //ack = htonl(ack);
    tuple4 *info = new tuple4;
    memcpy(info, (*it).second->address->sa_data, sizeof(tuple4));
    
    sendTCPPacket(it->second->databuf+it->second->writep, Len, info, seq, ack, (uint16_t)0);
    
    it->second->writep += Len;
    (*it).second->data_mtx.unlock();
    while(it->second->writep != it->second->sizep){
        //fprintf(stderr, "wp,sp:%d,%d\n", it->second->writep, it->second->sizep);
    }
    fprintf(stderr, "[INFO] write completed\n");
    return nbyte;
}

/**
 * @see [POSIX.1-2017:close](http://pubs.opengroup.org/onlinepubs/
 * 9699919799/functions/close.html)
 */
int __wrap_close(int fildes){
    std::map<int, mysocket_t*>::iterator it = iTCP_kernel.socket_set.find(fildes);
    
    if(it->second->tcb->role == TCP_SERVER){
        while(1){
            //fprintf(stderr, "%d\n", it->second->tcb->status);
            if(it->second->tcb->status == CLOSED) {
                fprintf(stderr, "fildes closed\n");
                return 0;
            }
        }
    }
    else{
        //fprintf(stderr, "wuwuwuwu\n");
        tuple4* info = new tuple4;
        info = (tuple4*)(it->second->address->sa_data);
        uint16_t flag = 0;

        flag = setFin(flag);
        uint8_t *tmp = new uint8_t;
        
        sendTCPPacket(tmp, 1, info, 0, 0, flag);
        it->second->tcb->status = FINWAIT1;
        while(1){
            //fprintf(stderr, "%d\n", it->second->tcb->status);
            if(it->second->tcb->status == TIMEWAIT) {
                fprintf(stderr, "fildes closed\n");
                return 0;
            }
        }
        return 0;
    }
}

/** 
 * @see [POSIX.1-2017:getaddrinfo](http://pubs.opengroup.org/onlinepubs/
 * 9699919799/functions/getaddrinfo.html)
 */
int __wrap_getaddrinfo(const char *node, const char *service,
    const struct addrinfo *hints,
    struct addrinfo **res){
    int cnt = 0;
    std::map<int, mysocket_t*>::iterator it = iTCP_kernel.socket_set.begin();
    if (memcmp( (char*)(&(((tuple4*)(it->second->address->sa_data))->srcPort)), service, 2 )==0 &&
        memcmp((char*)(&((tuple4*)(it->second->address->sa_data))->srcIP), node, 4) == 0){
            res[cnt] = (addrinfo*)(it->second->address);
            cnt += 1;
            it++;
    }
    return 0;
}