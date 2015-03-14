/* @(#) pfcompfp.h 96/12/18 1.6 */
/***************************************************************
** Compile FP routines.
** This file is included from "pf_compile.c"
**
** These routines could be left out of an execute only version.
**
** Author: Darren Gibbs, Phil Burk
** Copyright 1994 3DO, Phil Burk, Larry Polansky, David Rosenboom
**
** The pForth software code is dedicated to the public domain,
** and any third party may reproduce, distribute and modify
** the pForth software code or any derivative works thereof
** without any compensation or license.  The pForth software
** code is provided on an "as is" basis without any warranty
** of any kind, including, without limitation, the implied
** warranties of merchantability and fitness for a particular
** purpose and their equivalents under the laws of any jurisdiction.
**
****************************************************************
**
***************************************************************/


#ifdef PF_SUPPORT_FP
/* Core words */
	CreateDicEntryC( ID_FP_D_TO_F, "D>F", 0 );
	CreateDicEntryC( ID_FP_FSTORE, "F!", 0 );
	CreateDicEntryC( ID_FP_FTIMES, "F*", 0 );
	CreateDicEntryC( ID_FP_FPLUS, "F+", 0 );
	CreateDicEntryC( ID_FP_FMINUS, "F-", 0 );
	CreateDicEntryC( ID_FP_FSLASH, "F/", 0 );
	CreateDicEntryC( ID_FP_F_ZERO_LESS_THAN, "F0<", 0 );
	CreateDicEntryC( ID_FP_F_ZERO_EQUALS, "F0=", 0 );
	CreateDicEntryC( ID_FP_F_LESS_THAN, "F<", 0 );
	CreateDicEntryC( ID_FP_F_TO_D, "F>D", 0 );
	CreateDicEntryC( ID_FP_FFETCH, "F@", 0 );
	CreateDicEntryC( ID_FP_FDEPTH, "FDEPTH", 0 );
	CreateDicEntryC( ID_FP_FDROP, "FDROP", 0 );
	CreateDicEntryC( ID_FP_FDUP, "FDUP", 0 );
	CreateDicEntryC( ID_FP_FLITERAL, "FLITERAL", FLAG_IMMEDIATE );
	CreateDicEntryC( ID_FP_FLITERAL_P, "(FLITERAL)", 0 );
	CreateDicEntryC( ID_FP_FLOAT_PLUS, "FLOAT+", 0 );
	CreateDicEntryC( ID_FP_FLOATS, "FLOATS", 0 );
	CreateDicEntryC( ID_FP_FLOOR, "FLOOR", 0 );
	CreateDicEntryC( ID_FP_FMAX, "FMAX", 0 );
	CreateDicEntryC( ID_FP_FMIN, "FMIN", 0 );
	CreateDicEntryC( ID_FP_FNEGATE, "FNEGATE", 0 );
	CreateDicEntryC( ID_FP_FOVER, "FOVER", 0 );
	CreateDicEntryC( ID_FP_FROT, "FROT", 0 );
	CreateDicEntryC( ID_FP_FROUND, "FROUND", 0 );
	CreateDicEntryC( ID_FP_FSWAP, "FSWAP", 0 );
	
/* Extended words */
	CreateDicEntryC( ID_FP_FSTAR_STAR, "F**", 0 );
	CreateDicEntryC( ID_FP_FABS, "FABS", 0 );
	CreateDicEntryC( ID_FP_FACOS, "FACOS", 0 );
	CreateDicEntryC( ID_FP_FACOSH, "FACOSH", 0 );
	CreateDicEntryC( ID_FP_FALOG, "FALOG", 0 );
	CreateDicEntryC( ID_FP_FASIN, "FASIN", 0 );
	CreateDicEntryC( ID_FP_FASINH, "FASINH", 0 );
	CreateDicEntryC( ID_FP_FATAN, "FATAN", 0 );
	CreateDicEntryC( ID_FP_FATAN2, "FATAN2", 0 );
	CreateDicEntryC( ID_FP_FATANH, "FATANH", 0 );
	CreateDicEntryC( ID_FP_FCOS, "FCOS", 0 );
	CreateDicEntryC( ID_FP_FCOSH, "FCOSH", 0 );
	CreateDicEntryC( ID_FP_FLN, "FLN", 0 );
	CreateDicEntryC( ID_FP_FLNP1, "FLNP1", 0 );
	CreateDicEntryC( ID_FP_FLOG, "FLOG", 0 );
	CreateDicEntryC( ID_FP_FSIN, "FSIN", 0 );
	CreateDicEntryC( ID_FP_FSINCOS, "FSINCOS", 0 );
	CreateDicEntryC( ID_FP_FSINH, "FSINH", 0 );
	CreateDicEntryC( ID_FP_FSQRT, "FSQRT", 0 );
	CreateDicEntryC( ID_FP_FTAN, "FTAN", 0 );
	CreateDicEntryC( ID_FP_FTANH, "FTANH", 0 );
	CreateDicEntryC( ID_FP_FPICK, "FPICK", 0 );

#endif
