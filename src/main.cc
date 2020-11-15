/*
 * @Author: Naiqian
 * @Date: 2020-11-04 17:07:54
 * @LastEditTime: 2020-11-14 23:32:12
 * @LastEditors: Naiqian
 * @Description: 
 */
#include "core.h"
#include "device.h"
#include "packetio.h"
#include "utils.h"
#include "ip.h"
#include "include.h"
//extern int sendFrame(const void* buf, int len, 
//    int ethtype, const void* destmac, int id);
extern iTCP_kernel_t iTCP_kernel;

using namespace std;

int main(){
    /*
    //uint8_t broadcastIP[4] = {192, 168, 0, 1};
    //cout << hex << ptr2ip_t(broadcastIP) << endl;
    uint8_t fullMask[4] = {0xff, 0xff, 0xff, 0xff};
    //addDevice("veth0-3");
    uint8_t buf[8] = {0xff, 0x12, 0x11, 0,0,0,0,0};
    uint8_t nextMac[6] = {0x0e,0xc2,0x63,0x06,0x2a,0x93};
    //sendFrame(buf, 8, 0x0800, broadcastMAC, 0);
    //cout << "!!!!" << dec << sizeof(ipHdr_t) << endl;
    uint8_t dstIP[4] = {10,100,3,2};
    uint8_t srcIP[4] = {10,100,2,1};
    in_addr src, dst, mask;
    src.s_addr = ptr2ip_t(srcIP);
    dst.s_addr = ptr2ip_t(dstIP);
    mask.s_addr = ptr2ip_t(fullMask);
    uint8_t payload[10];
    */
    cout << addAll() << endl;
    //setRoutingTable(dst, mask, nextMac, "veth3-0");
    //cout << "fuck" << endl;
    //sendIPPacket(src, dst, 4, payload, 10, 32);
    sendArp(iTCP_kernel);
    setFrameReceiveCallback(recvFrame);
    /*
    pcap_t** handle;
    char** errbuf;
    handle = new pcap_t*[deviceNum];
    errbuf = new char*[deviceNum];
    const u_char *packet;
    for(int i = 0; i < deviceNum; i++){
        errbuf[i] = new char[BUFSIZ];
        handle[i] = pcap_open_live(iTCP_kernel.currDevice[i].Name.c_str(), BUFSIZ, 1, 1000, errbuf[i]);
    }

    */
    pcap_t *handle;
    char *errbuf;
    errbuf = new char[BUFSIZ];
    handle = pcap_open_live(iTCP_kernel.currDevice[0].Name.c_str(), BUFSIZ, 1, 1000, errbuf);
    const u_char* packet;
    uint8_t t[4] = {10,100,3,2};
    in_addr tmp; tmp.s_addr = ptr2ip_t(t);
    in_addr mask; mask.s_addr = 0xffffffff;
    uint8_t nem[6] = {0xea,0x5a,0xbb,0xd3,0x03,0x68};
    //routingTableop.setRoutingTable(tmp, mask, nem, "veth3-4");
    while(1){
        struct pcap_pkthdr header;
        cerr << "AHFKJDAHSKJDFHASK" << endl;
        //for(int i = 0; i < deviceNum; i++){
        packet = pcap_next(handle, &header);
        iTCP_kernel.eth_callback(packet, header.len, 0);
        //}
    }
    return 0;
}
