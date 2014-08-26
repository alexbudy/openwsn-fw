#ifndef __RRT_H
#define __RRT_H

/**
\addtogroup AppCoAP
\{
\addtogroup rrt
\{
*/

#include "opencoap.h"

//=========================== define ==========================================

#define COAP_GET_FROM_IP 			97
#define COAP_GET_TO_IP 				2
#define COAP_GET_NEXT_IP 			3
#define COAP_GET_MSG	 			4


//=========================== typedef =========================================

//=========================== variables =======================================

typedef struct {
   coap_resource_desc_t desc;
   uint8_t STATE; 
   char last_mssg;
   uint8_t mssg_sent;
} rrt_vars_t;


//=========================== prototypes ======================================

void rrt_init();

/**
\}
\}
*/

#endif
