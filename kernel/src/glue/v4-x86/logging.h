/*********************************************************************
 *                
 * Copyright (C) 2008-2010,  Karlsruhe University
 *                
 * File path:     glue/v4-x86/logging.h
 * Description:   
 *                
 * @LICENSE@
 *                
 * $Id:$
 *                
 ********************************************************************/
#ifndef __GLUE__V4_X86__LOGGING_H__
#define __GLUE__V4_X86__LOGGING_H__

#if defined(CONFIG_X_EVT_LOGGING)
#include INC_GLUE_SA(logging.h)
#else
#define LOG_PMC(src, dst)
#endif

#endif /* !__GLUE__V4_X86__LOGGING_H__ */
