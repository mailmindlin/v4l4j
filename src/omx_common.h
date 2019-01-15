

#ifndef __OMX_COMMON_H
#define __OMX_COMMON_H

#include "common.h"
#include "omx.h"


#define OMX_PACKAGE				V4L4J_PACKAGE "impl/omx/"
#define OMX_EXCP				OMX_PACKAGE "OMXException"

#define THROW_OMX_EXCP(e, rc, fmt, ...) \
	do { \
		THROW_EXCEPTION((e), OMX_EXCP, "%08x" (fmt) ": %#08x %s", (rc), __VA_ARGS__, (rc), getOMXErrorDescription((rc))); \
	} while (0)

#endif