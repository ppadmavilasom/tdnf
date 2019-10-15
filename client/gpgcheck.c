/*
 * Copyright (C) 2015 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU Lesser General Public License v2.1 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

/*
 * Module   : gpgcheck.c
 *
 * Abstract :
 *
 *            tdnfclientlib
 *
 *            client library
 *
 * Authors  : Priyesh Padmavilasom (ppadmavilasom@vmware.com)
 */

#include "includes.h"

static
uint32_t
_ImportKeys(
    PTDNF pTdnf,
    rpmts pSigVerifyTS,
    const char *pszRepo,
    const char* pszFile
    );

uint32_t
_ReadGPGKey(
   PTDNF pTdnf,
   const char *pszRepo,
   const char* pszKeyUrl,
   char** ppszKeyData
   );

uint32_t
_CollectAllReposWithGPGKeys(
    PTDNF pTdnf,
    PTDNF_PKG_INFO pPkgInfos,
    PKEYVALUE *ppRepos
    )
{
    uint32_t dwError = 0;
    PTDNF_PKG_INFO pPkgInfo = NULL;

    if (!pTdnf || !pPkgInfos || !ppRepos)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    for (pPkgInfo = pPkgInfos; pPkgInfo; pPkgInfo = pPkgInfo->pNext)
    {
        fprintf(stdout, "repo: %s\n", pPkgInfo->pszRepoName);
    }
    
cleanup:
    return dwError;

error:
    goto cleanup;
}
    

uint32_t
TDNFAddGPGKeysToImport(
    PTDNF pTdnf,
    PTDNF_SOLVED_PKG_INFO pSolvedPkgInfo
    )
{
    uint32_t dwError = 0;
    int i = 0;
    const int ITEMS = 4;
    PTDNF_PKG_INFO ppPkgInfos[] = {NULL, NULL, NULL, NULL};
    PKEYVALUE pRepos = NULL;

    if (!pTdnf || !pSolvedPkgInfo)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    ppPkgInfos[0] = pSolvedPkgInfo->pPkgsToInstall;
    ppPkgInfos[1] = pSolvedPkgInfo->pPkgsToUpgrade;
    ppPkgInfos[2] = pSolvedPkgInfo->pPkgsToDowngrade;
    ppPkgInfos[3] = pSolvedPkgInfo->pPkgsToReinstall;

    for (i = 0; i < ITEMS; ++i)
    {
        if (ppPkgInfos[i] == NULL)
        {
            continue;
        }
        dwError = _CollectAllReposWithGPGKeys(pTdnf, ppPkgInfos[i], &pRepos);
        BAIL_ON_TDNF_ERROR(dwError);
    }

cleanup:
    return dwError;

error:
    goto cleanup;
}

uint32_t
TDNFImportGPGKeys(
    PTDNF pTdnf,
    rpmts pSigVerifyTS,
    const char *pszRepo,
    const char* pszUrlKeyFile,
    const char* pszPkgFile
    )
{
    uint32_t dwError = 0;
    char* pszKeyData = NULL;

    if(!pTdnf || !pSigVerifyTS || IsNullOrEmptyString(pszRepo) ||
       IsNullOrEmptyString(pszUrlKeyFile) || !pszPkgFile)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    /*
     * TODO: only import keys if the verify transaction
     * did not already have keys
    */
    dwError = _ImportKeys(pTdnf, pSigVerifyTS, pszRepo, pszUrlKeyFile);
    BAIL_ON_TDNF_ERROR(dwError);

cleanup:
    TDNF_SAFE_FREE_MEMORY(pszKeyData);
    return dwError;

error:
    goto cleanup;
}

uint32_t
TDNFParseScheme(
    const char* pszKeyUrl,
    char** ppszScheme)
{
    uint32_t dwError = 0;
    char* pszScheme = NULL;
    const char* pszTmpStr = NULL;
    int nLen = 0;
    int i = 0;
    if(IsNullOrEmptyString(pszKeyUrl) || !ppszScheme)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }
    pszTmpStr = strchr(pszKeyUrl, ':');
    if(pszTmpStr == NULL)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }
    nLen = pszTmpStr - pszKeyUrl;
    dwError = TDNFAllocateMemory(
                  nLen + 1,
                  sizeof(char),
                  (void**)&pszScheme);
    BAIL_ON_TDNF_ERROR(dwError);
    for(i = 0; i < nLen; i ++)
    {
        pszScheme[i] = pszKeyUrl[i];
    }
    *ppszScheme = pszScheme;
cleanup:
    return dwError;

error:
    if(ppszScheme)
    {
        *ppszScheme = NULL;
    }
    TDNF_SAFE_FREE_MEMORY(pszScheme);
    goto cleanup;
}

uint32_t
FileNameFromUri(
    const char* pszKeyUrl,
    char** ppszFile)
{
    uint32_t dwError = 0;
    const char* pszPath = NULL;
    const char* pszFilePrefix = "file://";
    char *pszFile = NULL;

    if(strncmp(pszKeyUrl, pszFilePrefix, strlen(pszFilePrefix)) != 0)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }
    pszPath = pszKeyUrl + strlen(pszFilePrefix);

    if(!pszPath || *pszPath == '\0')
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if (strchr (pszPath, '#') != NULL)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if(*pszPath != '/')
    {
        //skip hostname in the uri.
        pszPath = strchr (pszPath, '/');
        if(pszPath == NULL)
        {
            dwError = ERROR_TDNF_INVALID_PARAMETER;
            BAIL_ON_TDNF_ERROR(dwError);
        }
    }
    dwError = TDNFAllocateString(pszPath, &pszFile);
    BAIL_ON_TDNF_ERROR(dwError);
    *ppszFile = pszFile;

cleanup:
    return dwError;

error:
    TDNF_SAFE_FREE_MEMORY(pszFile);
    if(ppszFile)
    {
        *ppszFile = NULL;
    }
    goto cleanup;
}

static
uint32_t
_ReadKeyFile(
   const char *pszKeyUrl,
   char **ppszKeyData
   )
{
    uint32_t dwError = 0;
    char *pszFile = NULL;
    int nPathIsDir = 0;
    char *pszKeyData = NULL;

    if(IsNullOrEmptyString(pszKeyUrl) || !ppszKeyData)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = FileNameFromUri(pszKeyUrl, &pszFile);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFIsDir(pszFile, &nPathIsDir);
    if(dwError)
    {
        fprintf(
            stderr,
            "Error: Accessing gpgkey at %s\n",
            pszFile);
    }
    BAIL_ON_TDNF_ERROR(dwError);

    if(nPathIsDir)
    {
        dwError = ERROR_TDNF_KEYURL_INVALID;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFFileReadAllText(pszFile, &pszKeyData);
    BAIL_ON_TDNF_ERROR(dwError);

    *ppszKeyData = pszKeyData;

cleanup:
    return dwError;

error:
    TDNF_SAFE_FREE_MEMORY(pszKeyData);
    goto cleanup;
}

uint32_t
_ReadGPGKey(
   PTDNF pTdnf,
   const char *pszRepo,
   const char* pszKeyUrl,
   char** ppszKeyData
   )
{
    uint32_t dwError = 0;
    int fd = -1;
    char* pszKeyData = NULL;
    char* pszScheme = NULL;
    char* pszFile = NULL;
    char pszTempKeyFile[] = "/tmp/pubkey.XXXXXX";

    if(IsNullOrEmptyString(pszKeyUrl) || !ppszKeyData)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError =  TDNFParseScheme(pszKeyUrl, &pszScheme);
    BAIL_ON_TDNF_ERROR(dwError);

    /*
     * allow only file and https
     * eventually add support for insecure schemes but control with
     * a repo setting and command line override
    */
    if (strcasecmp("file", pszScheme) != 0 &&
        strcasecmp("https", pszScheme) != 0)
    {
        dwError = ERROR_TDNF_KEYURL_UNSUPPORTED;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if(strcasecmp("file", pszScheme) == 0)
    {
        dwError = _ReadKeyFile(pszKeyUrl, &pszKeyData);
        BAIL_ON_TDNF_ERROR(dwError);
    }
    else
    {
        fd = mkstemp(pszTempKeyFile);
        if(fd < 1)
        {
            dwError = ERROR_TDNF_INVALID_PARAMETER;
            BAIL_ON_TDNF_ERROR(dwError);
        }

        dwError = TDNFDownloadFile(pTdnf, pszRepo, pszKeyUrl, pszTempKeyFile, NULL);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFFileReadAllText(pszTempKeyFile, &pszKeyData);
        BAIL_ON_TDNF_ERROR(dwError);
    }

    *ppszKeyData = pszKeyData;

cleanup:
    unlink(pszTempKeyFile);
    TDNF_SAFE_FREE_MEMORY(pszScheme);
    TDNF_SAFE_FREE_MEMORY(pszFile);
    return dwError;

error:
    TDNF_SAFE_FREE_MEMORY(pszKeyData);
    goto cleanup;
}

static
uint32_t
_ImportKey(
    PTDNF pTdnf,
    rpmts pSigVerifyTS,
    const char *pszRepo,
    const char* pszFile
    )
{
    uint32_t dwError = 0;
    pgpArmor nArmor = PGPARMOR_NONE;
    uint8_t* pPkt = NULL;
    size_t nPktLen = 0;
    char* pszKeyData = NULL;
    size_t nCertLen = 0;

    if(!pTdnf || !pSigVerifyTS || IsNullOrEmptyString(pszRepo) ||
       IsNullOrEmptyString(pszFile))
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = _ReadGPGKey(pTdnf, pszRepo, pszFile, &pszKeyData);
    BAIL_ON_TDNF_ERROR(dwError);

    nArmor = pgpParsePkts(pszKeyData, &pPkt, &nPktLen);
    if(nArmor != PGPARMOR_PUBKEY)
    {
        dwError = ERROR_TDNF_INVALID_PUBKEY_FILE;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = pgpPubKeyCertLen(pPkt, nPktLen, &nCertLen);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = rpmtsImportPubkey(pSigVerifyTS, pPkt, nCertLen);
    BAIL_ON_TDNF_ERROR(dwError);
cleanup:
    if (pPkt)
    {
        free(pPkt);
    }
    TDNF_SAFE_FREE_MEMORY(pszKeyData);
    return dwError;
error:
    goto cleanup;
}

static
uint32_t
_ImportKeys(
    PTDNF pTdnf,
    rpmts pSigVerifyTS,
    const char *pszRepo,
    const char* pszKeyUrl
    )
{
    uint32_t dwError = 0;
    int nKeyCount = 0;
    int i = 0;
    const char *pszSeparator = " ";
    char **ppszKeyUrls = NULL;

    dwError = TDNFMakeArrayFromString(pszKeyUrl,
                                      pszSeparator,
                                      &ppszKeyUrls,
                                      &nKeyCount);
    BAIL_ON_TDNF_ERROR(dwError);

    for (i = 0; i < nKeyCount; ++i)
    {
        if (IsNullOrEmptyString(ppszKeyUrls[i]))
        {
            continue;
        }
        dwError = _ImportKey(pTdnf, pSigVerifyTS, pszRepo, ppszKeyUrls[i]);
        BAIL_ON_TDNF_ERROR(dwError);
    }

cleanup:
    TDNFFreeStringArrayWithCount(ppszKeyUrls, nKeyCount);
    return dwError;

error:
    goto cleanup;
}
