/*
 * Copyright (C) 2020 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU Lesser General Public License v2.1 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

#include "includes.h"

TDNF_PLUGIN_INTERFACE _interface = {0};

const char *
TDNFPluginGetVersion(
    )
{
    return "1.0.0";
}

const char *
TDNFPluginGetName(
    )
{
    return "";
}

uint32_t
TDNFPluginInitialize(
    const char *pszConfig,
    PTDNF_PLUGIN_HANDLE *ppHandle
    )
{
    UNUSED(pszConfig);
    uint32_t dwError = 0;
    /* plugin does not expect config */
    if (!ppHandle)
    {
        dwError = 1;
        BAIL_ON_TDNF_ERROR(dwError);
    }
    dwError = TDNFInitRepoGPGCheck();
    BAIL_ON_TDNF_ERROR(dwError);

cleanup:
    return dwError;

error:
    goto cleanup;
}

uint32_t
TDNFRepoGPGCheckEventsNeeded(
    PTDNF_PLUGIN_HANDLE pHandle,
    TDNF_PLUGIN_EVENT_TYPE *pnEvents
    )
{
    uint32_t dwError = 0;
    TDNF_PLUGIN_EVENT_TYPE nEvents = TDNF_PLUGIN_EVENT_TYPE_REPO;
    if (!pHandle || !pnEvents)
    {
        dwError = 1;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    *pnEvents = nEvents;

cleanup:
    return dwError;

error:
    goto cleanup;
}
