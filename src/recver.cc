/*
 * @Author: Naiqian
 * @Date: 2020-11-14 00:02:22
 * @LastEditTime: 2020-12-09 03:44:45
 * @LastEditors: Naiqian
 * @Description: 
 * @FilePath: /iTCP/src/recver.cc
 */

#include "include.h"
#include "ip.h"
#include "utils.h"
#include "tcp.h"
#include "pthread.h"
#include "socket.h"
extern iTCP_kernel_t iTCP_kernel;
extern opRoutingTable routingTableop;
using namespace std;

void recvPcapPacket(BlockingQueue<recvPcap> &q, int id){
    //fprintf(stderr, "????\n");
    pcap_t *handle;
    char *errbuf;
    errbuf = new char[BUFSIZ];
    handle = pcap_open_live(iTCP_kernel.currDevice[id].Name.c_str(), BUFSIZ, 1, 1000, errbuf);
    while(1){
        struct pcap_pkthdr header;
        const u_char *packet;
        //fprintf(stderr, "????\n");
        packet = pcap_next(handle, &header);
        //fprintf(stderr, "fuck\n");
        recvPcap ret;
        memcpy(ret.packet, packet, header.len);
        ret.len = header.len;
        ret.ID = id;
        q.push(ret);
    }
    pcap_close(handle);
}

void periodSendMyArp(uint32_t interval){
    clock_t start = clock();
    int cnt = 0;
    while(cnt<2){
        cnt +=1;
        routingTableop.dumpRoutingTable();
        sendArp(iTCP_kernel);
        sleep(interval);
    }
}

void processIPPacket(){
    while(1){
        int fd = iTCP_kernel.unSolvedSocket.pop();
        readIPPacket(fd);
    }
    return;
}

void work(){
    sleep(10);
    fprintf(stderr, "[INFO] Start testing\n");
    std::thread processip(processIPPacket);
    processip.detach();

    int fd = __wrap_socket(AF_INET, SOCK_STREAM, 0);
    //fprintf(stderr, "%d", fd);
    sockaddr * sock = new sockaddr;
    uint8_t srcipp[4] = {10,100,2,2};
    
    ((tuple4*)(sock->sa_data))->srcIP = ptr2ip_t(srcipp);
    ((tuple4*)(sock->sa_data))->srcPort = h2n((uint16_t)10000);
    //fprintf(stderr, "checkbind:%d\n", ((tuple4*)(sock->sa_data))->srcPort);
    __wrap_bind(fd, sock, sizeof(sockaddr));
    uint32_t* tmp = new uint32_t;
    *tmp = sizeof(sockaddr);
    int connfd;
    connfd = __wrap_accept(fd, sock, tmp);
    uint8_t *readfile1 = new uint8_t[1001];
    __wrap_read(connfd, (void*)readfile1, 2002);
    fprintf(stderr, "[TEST]%x\n", *((uint32_t*)readfile1));
    __wrap_close(connfd);
}

int main(){
    int deviceNum = addAll();
    setFrameReceiveCallback(recvFrame);
    //IPPacketReceiveCallback
    setIPPacketReceiveCallback(recvIPPacket);
    
    //setIPPacketReceiveCallback()
    
    BlockingQueue<recvPcap> q(1024);
    //fprintf(stderr, "%d\n", deviceNum);
    for(int i = 0; i < deviceNum; i++){
        //fprintf(stderr, "%d\n", deviceNum);
        std::thread t(recvPcapPacket, std::ref(q), i);
        
        //fprintf(stderr, "%d\n", t.get_id());
        t.detach();
    }
    std::thread t(periodSendMyArp, (uint32_t)3);
    
    
    std::thread th_work(work);

    while(1){
        recvPcap tmp = q.pop();
        iTCP_kernel.getEthCallback()(tmp.packet, tmp.len, tmp.ID);
    }
    
    t.detach();
    th_work.detach();
    // std::thread processip(processIPPacket);
    // processip.detach();

    
    // int fd = __wrap_socket(AF_INET, SOCK_STREAM, 0);
    // fprintf(stderr, "%d", fd);
    // while(1)

    return 0;
}