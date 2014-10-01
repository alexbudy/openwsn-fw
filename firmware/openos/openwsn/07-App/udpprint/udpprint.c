#include "openwsn.h"
#include "udpprint.h"
#include "openqueue.h"
#include "openserial.h"

//=========================== defines  =======================================
const uint8_t rrt_path1[] = "rt";

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

void udpprint_init() {
}

void udpprint_receive(OpenQueueEntry_t* msg) {
    OpenQueueEntry_t* pkt;
    uint8_t numOptions;
    owerror_t outcome;

   
    pkt = openqueue_getFreePacketBuffer(COMPONENT_RRT);

    pkt->creator   = COMPONENT_RRT;
    pkt->owner      = COMPONENT_RRT;
    pkt->l4_protocol  = IANA_UDP;

    packetfunctions_reserveHeaderSize(pkt, 9);
    //pkt->payload[0] = msg->payload[5];
    //pkt->payload[1] = msg->payload[6];
    pkt->payload[2] = msg->l4_payload[0];
    pkt->payload[3] = msg->l4_payload[1];
    //pkt->payload[2] = msg->l3_destinationAdd.type;
    pkt->payload[2] = msg->l3_destinationAdd.addr_128b[15];
    pkt->payload[3] = msg->l3_sourceAdd.addr_128b[15];
    //pkt->payload[3] = msg->l3_destinationdAdd.type;
    //pkt->payload[6] = msg->l3_destinationAdd[2];
    //pkt->payload[7] = msg->l3_destinationAdd[3];

    numOptions = 0;
    // location-path option
    packetfunctions_reserveHeaderSize(pkt,sizeof(rrt_path1)-1);
    memcpy(&pkt->payload[0],&rrt_path1,sizeof(rrt_path1)-1);
    packetfunctions_reserveHeaderSize(pkt,1);
    pkt->payload[0]                  = (COAP_OPTION_NUM_URIPATH) << 4 |
       sizeof(rrt_path1)-1;
    numOptions++;
    // content-type option
    packetfunctions_reserveHeaderSize(pkt,2);
    pkt->payload[0]                  = COAP_OPTION_NUM_CONTENTFORMAT << 4 |
       1;
    pkt->payload[1]                  = COAP_MEDTYPE_APPOCTETSTREAM;
    numOptions++;

   //metada
   pkt->l4_destination_port   = WKP_UDP_RINGMASTER; //5683
   pkt->l4_sourcePortORicmpv6Type   = WKP_UDP_RINGMASTER;
   pkt->l3_destinationAdd.type = ADDR_128B;
   memcpy(&pkt->l3_destinationAdd.addr_128b[0], &ipAddr_localhost, 16);
   //send
   outcome = openudp_send(pkt);
  

   if (outcome == E_FAIL) {
     openqueue_freePacketBuffer(pkt);
   }

   openqueue_freePacketBuffer(msg);
}

void udpprint_sendDone(OpenQueueEntry_t* msg, owerror_t error) {
   openserial_printError(
      COMPONENT_UDPPRINT,
      //ERR_UNEXPECTED_SENDDONE,
      ERR_NEIGHBORS_FULL,
      (errorparameter_t)0,
      (errorparameter_t)0
   );
   openqueue_freePacketBuffer(msg);
}

bool udpprint_debugPrint() {
   return FALSE;
}

//=========================== private =========================================
