#ifndef __REX_H
#define __REX_H

#include "opencoap.h"

/**
\addtogroup AppUdp
\{
\addtogroup rT
\{
*/

//=========================== define ==========================================

//=========================== typedef =========================================

//=========================== variables =======================================

typedef struct {
   coap_resource_desc_t desc;
   opentimer_id_t timerId;
} rex_vars_t;

//=========================== prototypes ======================================

void rex_init();

/**
\}
\}
*/

#endif
