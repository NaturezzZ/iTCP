/*
 * @Author: Naiqian
 * @Date: 2020-11-02 19:03:09
 * @LastEditTime: 2020-11-05 00:56:11
 * @LastEditors: Naiqian
 * @Description: 
 */
#include "packetio.h"
#include "device.h"
#include "utils.h"
extern iTCP_kernel_t iTCP_kernel;
extern uint16_t change_Endian(uint16_t x);
//std::pair<uint8_t*, uint32_t> 
//using namespace std;
std::pair<uint8_t*, uint32_t> constructFrame(const void* buf, int len, int ethtype, const void* destmac, int id){
   unsigned frame_len = len + sizeof(eth_hdr_t) + 4;
   //cout << "*****" << frame_len << endl;
   auto &device = iTCP_kernel.getDevice(id);
   uint8_t* frameBuffer = new uint8_t[frame_len];
   uint8_t* ethHdr = new uint8_t[sizeof(eth_hdr_t)];
   uint8_t* checkSum = new uint8_t[4];

   memset(frameBuffer, 0, sizeof(frameBuffer));
   memset(ethHdr, 0, 14);
   memset(checkSum, 0, 4);

   memcpy(ethHdr, destmac, 6);
   memcpy(ethHdr+6, device.Mac, 6);
   *(uint16_t*)(ethHdr+12) = (uint16_t) change_Endian(ethtype);
   
   memcpy(frameBuffer, ethHdr, sizeof(eth_hdr_t));
   memcpy(frameBuffer+sizeof(eth_hdr_t), buf, len);
   memcpy(frameBuffer+sizeof(eth_hdr_t)+len, checkSum, 4);

   std::cerr << "Constructing frame on " << device.Name << std::endl;
   return std::make_pair(frameBuffer, frame_len);
}

int send_frame(const void *buf, int len, int ethtype, const void *destmac,int id){
   std::pair<uint8_t*, uint32_t> sentFrame = constructFrame(buf, len, ethtype, destmac, id);
   int sentResult = iTCP_kernel.getDevice(id).sendFrame(sentFrame.first, sentFrame.second);
   if(sentFrame.first) delete[] sentFrame.first;
   return sentResult;
}

int setFrameReceiveCallback(frameReceiveCallback callback){
   iTCP_kernel.getCallback() = callback;
   return 0;
}