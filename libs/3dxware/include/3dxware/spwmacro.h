/*----------------------------------------------------------------------
 *  spwmacro.h -- cpp macros we ALWAYS use.
 *
 *  $Id: spwmacro.h 11091 2015-01-09 11:02:45Z jwick $
 *
 *   We always seem to use the same macros.
 *   This is the place we define them.
 *
 *----------------------------------------------------------------------
 */
/*
 * Copyright notice:
 * Copyright (c) 1998-2015 3Dconnexion. All rights reserved. 
 * 
 * This file and source code are an integral part of the "3Dconnexion
 * Software Developer Kit", including all accompanying documentation,
 * and is protected by intellectual property laws. All use of the
 * 3Dconnexion Software Developer Kit is subject to the License
 * Agreement found in the "LicenseAgreementSDK.txt" file.
 * All rights not expressly granted by 3Dconnexion are reserved.
 */
#ifndef SPWMACRO_H
#define SPWMACRO_H

 
#define SPW_FALSE   (0)
#define SPW_TRUE    (!SPW_FALSE)

#define SPW_MAX(a,b)   (((a)>(b))?(a):(b))
#define SPW_MIN(a,b)   (((a)<(b))?(a):(b))

#define SPW_ABS(a)   (((a)<0)?(-(a)):(a))

#define SPW_SIGN(a)  ((a)>=0?1:-1)

#define SPW_BIND(min,n,max)   (SPW_MIN((max),SPW_MAX((min),(n))))

#define SPW_NUM_ELEMENTS_IN(a)   (sizeof(a)/sizeof((a)[0]))

#define SPW_PI   3.14159265358979324f

#define SPW_DEG_TO_RAD(d)   ((d)*SPW_PI/180.0f)
#define SPW_RAD_TO_DEG(r)   ((r)*180.0f/SPW_PI)

#define SPW_LENGTH_OF(a)   (sizeof(a)/sizeof((a)[0]))

#define SPW_END_OF(a)   (&(a)[SPW_LENGTH_OF(a)-1])

#define SPW_SQ(a)   ((a)*(a))

#define SPW_ABSDIFF(a, b) (fabs((double) (a) - (b)))


#endif
