/*
 * Copyright (C) 2020 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU Lesser General Public License v2.1 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

#include "includes.h"
#include <gpgme.h>

static
uint32_t
_TDNFVerifyResult(
    gpgme_ctx_t pContext
    )
{
    uint32_t dwError = 0;
    gpgme_verify_result_t pResult = NULL;
    gpgme_signature_t pSig = NULL;

    /* pContext release will free pResult. do not free otherwise */
    pResult = gpgme_op_verify_result(pContext);
    if (!pResult)
    {
        dwError = ERROR_TDNF_GPG_ERROR;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    for(pSig = pResult->signatures; pSig; pSig = pSig->next)
    {
        if (pSig->status)
        {
            fprintf(stderr, "repo md signature check: %s\n", gpgme_strerror (pSig->status));
            dwError = ERROR_TDNF_GPG_ERROR;
            break;
        }
    }
cleanup:
    return dwError;

error:
    goto cleanup;
}

uint32_t
TDNFInitRepoGPGCheck(
    )
{
    uint32_t dwError = 0;
    const char *pszVersion = NULL;

    pszVersion = gpgme_check_version(NULL);
    if (!pszVersion)
    {
        dwError = ERROR_TDNF_GPG_VERSION_FAILED;
        BAIL_ON_TDNF_ERROR(dwError);
    }

error:
    return dwError;
}

uint32_t
TDNFVerifyRepoMDSignature(
    const char *pszRepoMD,
    const char *pszRepoMDSig
    )
{
    uint32_t dwError = 0;
    FILE *fpRepoMD = NULL;
    FILE *fpRepoMDSig = NULL;
    gpgme_error_t nGPGError = 0;
    gpgme_ctx_t pContext = NULL;
    gpgme_protocol_t protocol = GPGME_PROTOCOL_OpenPGP;
    gpgme_data_t dataSig = NULL;
    gpgme_data_t dataText = NULL;

    if (IsNullOrEmptyString(pszRepoMD) || IsNullOrEmptyString(pszRepoMDSig))
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    nGPGError = gpgme_new(&pContext);
    if (nGPGError)
    {
        dwError = ERROR_TDNF_GPG_ERROR;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    gpgme_set_protocol (pContext, protocol);

    fpRepoMDSig = fopen(pszRepoMDSig, "rb");
    if (!fpRepoMDSig)
    {
        dwError = ERROR_TDNF_GPG_ERROR;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = gpgme_data_new_from_stream (&dataSig, fpRepoMDSig);
    if (dwError)
    {
        dwError = ERROR_TDNF_GPG_ERROR;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    fpRepoMD = fopen(pszRepoMD, "rb");
    if (!fpRepoMD)
    {
        dwError = ERROR_TDNF_GPG_ERROR;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = gpgme_data_new_from_stream(&dataText, fpRepoMD);
    if (dwError)
    {
        dwError = ERROR_TDNF_GPG_ERROR;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = gpgme_op_verify(pContext, dataSig, dataText, NULL);
    if (dwError)
    {
        fprintf(stderr, "gpg verify failed: %s\n", gpgme_strerror(dwError));
        dwError = ERROR_TDNF_GPG_ERROR;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = _TDNFVerifyResult(pContext);
    BAIL_ON_TDNF_ERROR(dwError);

cleanup:
    if (dataText)
    {
        gpgme_data_release(dataText);
    }
    if (dataSig)
    {
        gpgme_data_release(dataSig);
    }
    if (fpRepoMD)
    {
        fclose(fpRepoMD);
    }
    if (fpRepoMDSig)
    {
        fclose(fpRepoMDSig);
    }
    if (pContext)
    {
        gpgme_release(pContext);
    }
    return dwError;

error:
    goto cleanup;
}
