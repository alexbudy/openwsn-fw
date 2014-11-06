#ifndef __RRT_H
#define __RRT_H

/**
\addtogroup AppCoAP
\{
\addtogroup rrt
\{
*/

#include "opendefs.h"
#include "opencoap.h"
#include "schedule.h"

//=========================== define ==========================================

//=========================== typedef =========================================

//=========================== variables =======================================

typedef struct {
   coap_resource_desc_t desc;
   uint8_t discovered;
} rrt_vars_t;

//=========================== prototypes ======================================

void rrt_init(void);

void sendCoAPMsg(char actionMsg, uint8_t mote);
/**
\}
\}
*/

#endif