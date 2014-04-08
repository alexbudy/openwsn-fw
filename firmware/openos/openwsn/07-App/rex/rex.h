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
  opentimer_id_t  timerId;
	coap_resource_desc_t desc;
} rex_vars_t;

//=========================== prototypes ======================================

void rex_init();

/**
\}
\}
*/

#endif
