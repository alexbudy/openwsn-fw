#ifndef __REX_H
#define __REX_H



/**
\addtogroup AppUdp
\{
\addtogroup rT
\{
*/

#include "opencoap.h"

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
