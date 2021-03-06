/**
\brief A CoAP resource which indicates the board its running on.
*/

#include "openwsn.h"
#include "rrt.h"
#include "opencoap.h"
#include "openqueue.h"
#include "packetfunctions.h"
#include "openserial.h"
#include "openrandom.h"
#include "board.h"
#include "idmanager.h"

//=========================== defines =========================================

const uint8_t rrt_path0[] = "rt";
#define PAYLOADLEN  2

//=========================== variables =======================================

rrt_vars_t rrt_vars;

uint8_t * tmp_payload;

//=========================== prototypes ======================================

owerror_t     rrt_receive(
   OpenQueueEntry_t* msg,
   coap_header_iht*  coap_header,
   coap_option_iht*  coap_options
);

void          rrt_sendDone(
   OpenQueueEntry_t* msg,
   owerror_t error
);

uint8_t *  getIPFromPayload(
	uint8_t* payload,
	int      ip_to_get 
);

void rotateState();
void sendPacketToRingmaster(char msg);
void sendPacketToMote(char msg, uint8_t mote);

//=========================== public ==========================================

/**
\brief Initialize this module.
*/
void rrt_init() {
   
   // do not run if DAGroot
   if(idmanager_getIsDAGroot()==TRUE) return; 
   
   // prepare the resource descriptor for the /rt path
   rrt_vars.desc.path0len             = sizeof(rrt_path0)-1;
   rrt_vars.desc.path0val             = (uint8_t*)(&rrt_path0);
   rrt_vars.desc.path1len             = 0;
   rrt_vars.desc.path1val             = NULL;
   rrt_vars.desc.componentID          = COMPONENT_RRT;
   rrt_vars.desc.callbackRx           = &rrt_receive;
   rrt_vars.desc.callbackSendDone     = &rrt_sendDone;

   //initialize state
   rrt_vars.STATE                     = 0;
   rrt_vars.last_mssg                 = 'a';
   
   // register with the CoAP module
   opencoap_register(&rrt_vars.desc);
}

//=========================== private =========================================

/**
\brief Called when a CoAP message is received for this resource.

\param[in] msg          The received message. CoAP header and options already
   parsed.
\param[in] coap_header  The CoAP header contained in the message.
\param[in] coap_options The CoAP options contained in the message.

\return Whether the response is prepared successfully.
*/

owerror_t rrt_receive(
      OpenQueueEntry_t* msg,
      coap_header_iht* coap_header,
      coap_option_iht* coap_options
   ) {
   
   OpenQueueEntry_t* pkt;
   uint8_t numOptions;
   owerror_t outcome;
   uint8_t i;
   
   switch (coap_header->Code) {
      case COAP_CODE_REQ_GET:
         rotateState(&rrt_vars.STATE);
         
         //=== reset packet payload (we will reuse this packetBuffer)
         msg->payload                     = &(msg->packet[127]);
         msg->length                      = 0;
         
         //=== prepare  CoAP response
         
	    //ip from
         packetfunctions_reserveHeaderSize(msg,2);
         msg->payload[0] = '0' + rrt_vars.STATE;
         msg->payload[1] = rrt_vars.last_mssg;

         // payload marker
         packetfunctions_reserveHeaderSize(msg,1);
         msg->payload[0] = COAP_PAYLOAD_MARKER;
         
         // set the CoAP header
         coap_header->Code                = COAP_CODE_RESP_CONTENT;

         
         pkt = openqueue_getFreePacketBuffer(COMPONENT_RRT);
         if (pkt == NULL) {
             openserial_printError(COMPONENT_RRT,ERR_BUSY_SENDING,
                                   (errorparameter_t)0,
                                   (errorparameter_t)0);
             openqueue_freePacketBuffer(pkt);
             return;
         }

         pkt->creator   = COMPONENT_RRT;
         pkt->owner      = COMPONENT_RRT;
         pkt->l4_protocol  = IANA_UDP;

         packetfunctions_reserveHeaderSize(pkt, 1);
         pkt->payload[0] = msg; //D stands for DISCOVERY

         numOptions = 0;
         // location-path option
         packetfunctions_reserveHeaderSize(pkt,sizeof(rrt_path0)-1);
         memcpy(&pkt->payload[0],&rrt_path0,sizeof(rrt_path0)-1);
         packetfunctions_reserveHeaderSize(pkt,1);
         pkt->payload[0]                  = (COAP_OPTION_NUM_URIPATH) << 4 |
            sizeof(rrt_path0)-1;
         numOptions++;
         // content-type option
         packetfunctions_reserveHeaderSize(pkt,2);
         pkt->payload[0]                  = COAP_OPTION_NUM_CONTENTFORMAT << 4 |
            1;
         pkt->payload[1]                  = COAP_MEDTYPE_APPOCTETSTREAM;
         numOptions++;

         //metada
         pkt->l4_destination_port   = WKP_UDP_RINGMASTER;
         pkt->l4_sourcePortORicmpv6Type   = WKP_UDP_RINGMASTER;
         pkt->l3_destinationAdd.type = ADDR_128B;
         memcpy(&pkt->l3_destinationAdd.addr_128b[0], &ipAddr_localhost, 16);

         //send
         outcome = openudp_send(pkt);

         if (outcome == E_FAIL) {
           openqueue_freePacketBuffer(pkt);
         }
         outcome                          = E_SUCCESS;
         break;
      case COAP_CODE_REQ_PUT:
          //PUT - receives message from last mote
          //simply save the first char received

          //this is just for debugging purposes
          //openserial_printData((uint8_t*)(msg->payload), msg->length);
          openserial_printError(COMPONENT_RRT, 0x2a,
                                (errorparameter_t)0,
                                (errorparameter_t)0);

          rrt_vars.last_mssg = msg->payload[0];
          outcome                         = E_SUCCESS;

          break;

      case COAP_CODE_REQ_POST:
          tmp_payload = getIPFromPayload(msg->payload, COAP_GET_FROM_IP);

          msg->payload 										= &(msg->packet[127]);
          msg->length											= 0;

          packetfunctions_reserveHeaderSize(msg, 4);
          msg->payload[0] = 'x';
          msg->payload[1] = 'y';
          msg->payload[2] = 'z';
          msg->payload[3] = tmp_payload[0];
          rrt_vars.last_mssg = tmp_payload[0];

          if (tmp_payload[0] == 'P') { //P - perform an action
              //palce holder for doing an action 
              openserial_printError(COMPONENT_RRT,ERR_BUSY_SENDING,
                                    (errorparameter_t)0,
                                    (errorparameter_t)0);
              //send message back to RM saying done
              
          } else if (tmp_payload[0] == 'F') {
              //send a packet to next mote here telling action

          } else  {
              //neither perform an action nor forward a packet, so no actio needed
          }
          pkt = openqueue_getFreePacketBuffer(COMPONENT_RRT);
          if (pkt == NULL) {
              openserial_printError(COMPONENT_RRT,ERR_BUSY_SENDING,
                                    (errorparameter_t)0,
                                    (errorparameter_t)0);
              openqueue_freePacketBuffer(pkt);
              return;
          }

          pkt->creator   = COMPONENT_RRT;
          pkt->owner      = COMPONENT_RRT;
          pkt->l4_protocol  = IANA_UDP;

          packetfunctions_reserveHeaderSize(pkt, 1);
          pkt->payload[0] = 'D';

           numOptions = 0;
           // location-path option
           packetfunctions_reserveHeaderSize(pkt,sizeof(rrt_path0)-1);
           memcpy(&pkt->payload[0],&rrt_path0,sizeof(rrt_path0)-1);
           packetfunctions_reserveHeaderSize(pkt,1);
           pkt->payload[0]                  = (COAP_OPTION_NUM_URIPATH) << 4 |
              sizeof(rrt_path0)-1;
           numOptions++;
           // content-type option
           packetfunctions_reserveHeaderSize(pkt,2);
           pkt->payload[0]                  = COAP_OPTION_NUM_CONTENTFORMAT << 4 |
              1;
           pkt->payload[1]                  = COAP_MEDTYPE_APPOCTETSTREAM;
           numOptions++;

          //metada
          pkt->l4_destination_port   = WKP_UDP_RINGMASTER; 
          pkt->l4_sourcePortORicmpv6Type   = WKP_UDP_RINGMASTER;
          pkt->l3_destinationAdd.type = ADDR_128B;
          memcpy(&pkt->l3_destinationAdd.addr_128b[0], &ipAddr_simMotes, 16);
          //send
          outcome = openudp_send(pkt);
          

          if (outcome == E_FAIL) {
            openqueue_freePacketBuffer(pkt);
          }
          
          // payload marker
          packetfunctions_reserveHeaderSize(msg,1);
          msg->payload[0] = COAP_PAYLOAD_MARKER;

          // set the CoAP header
          coap_header->Code                = COAP_CODE_RESP_CONTENT;

          outcome													= E_SUCCESS;
          break;
      default:
         // return an error message
         outcome = E_FAIL;
   }
   
   return outcome;
}

//send packet to ringmaster
void sendPacketToRingmaster(char msg) {
     OpenQueueEntry_t* pkt2;
     uint8_t numOpts;
     owerror_t outcome2;
     /*

     pkt2 = openqueue_getFreePacketBuffer(COMPONENT_RRT);
     if (pkt2 == NULL) {
         openserial_printError(COMPONENT_RRT,ERR_BUSY_SENDING,
                               (errorparameter_t)0,
                               (errorparameter_t)0);
         openqueue_freePacketBuffer(pkt2);
         return;
     }

     pkt->creator   = COMPONENT_RRT;
     pkt->owner      = COMPONENT_RRT;
     pkt->l4_protocol  = IANA_UDP;

     packetfunctions_reserveHeaderSize(pkt, 1);
     pkt->payload[0] = msg; //D stands for DISCOVERY

     numOptions = 0;
     // location-path option
     packetfunctions_reserveHeaderSize(pkt,sizeof(rrt_path0)-1);
     memcpy(&pkt->payload[0],&rrt_path0,sizeof(rrt_path0)-1);
     packetfunctions_reserveHeaderSize(pkt,1);
     pkt->payload[0]                  = (COAP_OPTION_NUM_URIPATH) << 4 |
        sizeof(rrt_path0)-1;
     numOptions++;
     // content-type option
     packetfunctions_reserveHeaderSize(pkt,2);
     pkt->payload[0]                  = COAP_OPTION_NUM_CONTENTFORMAT << 4 |
        1;
     pkt->payload[1]                  = COAP_MEDTYPE_APPOCTETSTREAM;
     numOptions++;

     //metada
     pkt->l4_destination_port   = WKP_UDP_RINGMASTER;
     pkt->l4_sourcePortORicmpv6Type   = WKP_UDP_RINGMASTER;
     pkt->l3_destinationAdd.type = ADDR_128B;
     memcpy(&pkt->l3_destinationAdd.addr_128b[0], &ipAddr_localhost, 16);

     //send
     outcome = openudp_send(pkt);

     if (outcome == E_FAIL) {
       openqueue_freePacketBuffer(pkt);
     }
    */
}

void sendPacketToMote(char msg, uint8_t mote) {

}

//add more states as needed
//TODO
void rotateState(uint8_t * state) {
    
   switch(*state) {
        case 0:
            *state = 1;
            break;
        case 1:
            *state = 2;
            break;
        case 2:
            *state = 0;
            break;
        default:
            *state = 0;
            break;
   } 

}

uint8_t * getIPFromPayload(uint8_t* payload, int msg) {
	uint8_t * newPtr = payload;
	switch (msg) {
		case COAP_GET_FROM_IP:
			newPtr = newPtr + 0;
			break;
		case COAP_GET_TO_IP:
			newPtr = newPtr + 1;
			break;
		case COAP_GET_NEXT_IP:
			newPtr = newPtr + 2;
			break;
		case COAP_GET_MSG:
			newPtr = newPtr + 3;
			break;
		default:
			break;
	}

	return newPtr;
}

/**
\brief The stack indicates that the packet was sent.

\param[in] msg The CoAP message just sent.
\param[in] error The outcome of sending it.
*/
void rrt_sendDone(OpenQueueEntry_t* msg, owerror_t error) {
   openqueue_freePacketBuffer(msg);
}
