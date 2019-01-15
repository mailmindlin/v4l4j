#ifndef __V4L4J_OMX_H
#define __V4L4J_OMX_H

#include <IL/OMX_Core.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct OMXMethods {
	/**
	 * Pointer to OMX_Init()
	 */
	OMX_ERRORTYPE (*init) (void);
	/**
	 * Pointer to OMX_Deinit()
	 */
	OMX_ERRORTYPE (*deinit) (void);
	/**
	 * Pointer to OMX_ComponentNameEnum()
	 */
	OMX_ERRORTYPE (*componentNameEnum) (OMX_OUT OMX_STRING cComponentName, OMX_IN OMX_U32 nNameLength, OMX_IN OMX_U32 nIndex);
	/**
	 * Pointer to OMX_GetHandle()
	 */
	OMX_ERRORTYPE (*getHandle) (OMX_OUT OMX_HANDLETYPE* pHandle, OMX_IN OMX_STRING cComponentName, OMX_IN OMX_PTR pAppData, OMX_IN OMX_CALLBACKTYPE* pCallBacks);
	/**
	 * Pointer to OMX_FreeHandle()
	 */
	OMX_ERRORTYPE (*freeHandle) (OMX_IN OMX_HANDLETYPE hComponent);
	/**
	 * Pointer to OMX_SetupTunnel()
	 */
	OMX_ERRORTYPE (*setupTunnel) (OMX_IN  OMX_HANDLETYPE hOutput, OMX_IN  OMX_U32 nPortOutput, OMX_IN  OMX_HANDLETYPE hInput, OMX_IN  OMX_U32 nPortInput);
	/**
	 * Pointer to OMX_GetContentPipe()
	 */
	OMX_ERRORTYPE (*getContentPipe) (OMX_OUT OMX_HANDLETYPE *hPipe, OMX_IN OMX_STRING szURI);
	/**
	 * Pointer to OMX_GetComponentsOfRole()
	 */
	OMX_ERRORTYPE (*getComponentsOfRole) (OMX_IN OMX_STRING role, OMX_INOUT OMX_U32 *pNumComps, OMX_INOUT OMX_U8 **compNames);
	/**
	 * Pointer to OMX_GetRolesOfComponent()
	 */
	OMX_ERRORTYPE (*getRolesOfComponent) (OMX_IN OMX_STRING compName, OMX_INOUT OMX_U32 *pNumRoles, OMX_OUT OMX_U8 **roles);
} OMXMethods;

/**
 * Load libomxil and get pointers to its methods.
 * By the time this method has returned, OMX_Init() and any other similar methods should have been called.
 * @return struct of method pointers on success, NULL on failure.
 */
OMXMethods* v4lconvert_omx_init();

/**
 * Release method pointers, and unload libomxil if possible.
 */
void v4lconvert_omx_deinit(OMXMethods* methods);

#ifdef __cplusplus
}
#endif

#endif //__V4L4J_OMX_H
