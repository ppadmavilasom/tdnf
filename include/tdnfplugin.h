/*
 * Copyright (C) 2020 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU Lesser General Public License v2.1 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

#ifndef _TDNF_PLUGIN_H_
#define _TDNF_PLUGIN_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "tdnf.h"

typedef struct _TDNF_PLUGIN_HANDLE_ *PTDNF_PLUGIN_HANDLE;

/* function names */
#define TDNF_FN_NAME_PLUGIN_GET_VERSION       "TDNFPluginGetVersion"
#define TDNF_FN_NAME_PLUGIN_GET_NAME          "TDNFPluginGetName"
#define TDNF_FN_NAME_PLUGIN_LOAD_INTERFACE    "TDNFPluginLoadInterface"
#define TDNF_FN_NAME_PLUGIN_UNLOAD_INTERFACE  "TDNFPluginUnloadInterface"

typedef enum
{
    TDNF_PLUGIN_EVENT_TYPE_NONE = 0x0,
    TDNF_PLUGIN_EVENT_TYPE_REPO = 0x1,
    TDNF_PLUGIN_EVENT_TYPE_ALL  = TDNF_PLUGIN_EVENT_TYPE_REPO
}TDNF_PLUGIN_EVENT_TYPE;

typedef enum
{
    TDNF_PLUGIN_EVENT_STATE_NONE,
    TDNF_PLUGIN_EVENT_STATE_INIT,
    TDNF_PLUGIN_EVENT_STATE_PARSE,
}TDNF_PLUGIN_EVENT_STATE;

typedef enum
{
    TDNF_PLUGIN_EVENT_PHASE_NONE,
    TDNF_PLUGIN_EVENT_PHASE_START,
    TDNF_PLUGIN_EVENT_PHASE_END
}TDNF_PLUGIN_EVENT_PHASE;


/* plugin event layout (32 bit)
 * <   type   >  <  state  >   <  phase  >
 * <  24 bits >  <  6 bits >   <  2 bits >
 * Eg: TDNF_PLUGIN_EVENT_REPO_INIT_START = TDNF_PLUGIN_EVENT_TYPE_REPO  << 8 |
 *                                         TDNF_PLUGIN_EVENT_STATE_INIT << 2 |
 *                                         TDNF_PLUGIN_EVENT_PHASE_START
*/
typedef uint32_t TDNF_PLUGIN_EVENT;

typedef struct _TDNF_PLUGIN_CONTEXT_
{
    TDNF_PLUGIN_EVENT nEvent;
    PTDNF pTdnf;
}TDNF_PLUGIN_CONTEXT, *PTDNF_PLUGIN_CONTEXT;

/* version of the plugin interface */
typedef const char *
(*PFN_TDNF_PLUGIN_GET_VERSION)(
    void
    );

/* name of the plugin interface */
typedef const char *
(*PFN_TDNF_PLUGIN_GET_NAME)(
    void
    );

/*
 * Return a bit mask of events required by a plugin
*/
typedef uint32_t
(*PFN_TDNF_PLUGIN_EVENTS_NEEDED)(
    PTDNF_PLUGIN_HANDLE pHandle,
    TDNF_PLUGIN_EVENT_TYPE *pnEventTypes
    );

/*
 * initialize. handle returned must be saved.
 * handle is used in all api calls.
 * when done, use pFnCloseHandle to close.
*/
typedef uint32_t
(*PFN_TDNF_PLUGIN_INITIALIZE)(
    const char *pszConfigFile,
    PTDNF_PLUGIN_HANDLE *ppHandle
    );

/*
 * Return error description for errors originating from this plugin.
 * Return human readable error strings. Do not include user supplied
 * data in error strings.
*/
typedef uint32_t
(*PFN_TDNF_PLUGIN_GET_ERROR_STRING)(
    uint32_t dwError,
    char **ppszError
    );

/* event callback */
typedef uint32_t
(*PFN_TDNF_PLUGIN_EVENT)(
    PTDNF_PLUGIN_HANDLE pHandle,
    PTDNF_PLUGIN_CONTEXT pContext
    );

/* close handle */
typedef void
(*PFN_TDNF_PLUGIN_CLOSE_HANDLE)(
    PTDNF_PLUGIN_HANDLE pHandle
    );

typedef struct _TDNF_PLUGIN_INTERFACE_
{
    PFN_TDNF_PLUGIN_INITIALIZE          pFnInitialize;
    PFN_TDNF_PLUGIN_EVENTS_NEEDED       pFnEventsNeeded;
    PFN_TDNF_PLUGIN_GET_ERROR_STRING    pFnGetErrorString;
    PFN_TDNF_PLUGIN_EVENT               pFnEvent;
    PFN_TDNF_PLUGIN_CLOSE_HANDLE        pFnCloseHandle;
}TDNF_PLUGIN_INTERFACE, *PTDNF_PLUGIN_INTERFACE;

/*
 * Plugins should implement this function with
 * the exact function name "TDNFPluginLoadInterface"
*/
typedef uint32_t
(*PFN_TDNF_PLUGIN_LOAD_INTERFACE)(
    PTDNF_PLUGIN_INTERFACE pInterface
    );

#ifdef __cplusplus
}
#endif

#endif//TDNF_PLUGIN_H_
