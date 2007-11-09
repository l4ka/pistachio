/*********************************************************************
 *                
 * Copyright (C) 2007,  Karlsruhe University
 *                
 * File path:     glue/v4-x86/config.h
 * Description:   
 *                
 * @LICENSE@
 *                
 * $Id:$
 *                
 ********************************************************************/
#define INC_ARCH_SA(x)             <arch/__ARCH__/__SUBARCH__/x>
#define INC_GLUE_SA(x)             <glue/__API__-__ARCH__/__SUBARCH__/x>

#include INC_GLUE_SA(config.h)
