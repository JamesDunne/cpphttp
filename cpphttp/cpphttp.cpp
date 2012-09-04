// cpphttp.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

int _tmain(int argc, _TCHAR* argv[])
{
    HTTPAPI_VERSION apiVersion = HTTPAPI_VERSION_2;
    auto result = HttpInitialize(apiVersion, HTTP_INITIALIZE_SERVER, 0);
    if (NO_ERROR != result) goto cleanup0;

    wprintf(L"HTTP initialized.\n");

    HANDLE reqQueue = 0;
    result = HttpCreateRequestQueue(apiVersion, NULL, NULL, 0, &reqQueue);
    if (NO_ERROR != result) goto cleanup1;
    wprintf(L"HTTP request queue created.\n");

    HTTP_SERVER_SESSION_ID serverSessionId = 0;
    result = HttpCreateServerSession(apiVersion, &serverSessionId, 0);
    if (NO_ERROR != result) goto cleanup2;
    wprintf(L"HTTP server session created.\n");

    HTTP_URL_GROUP_ID urlGroupId = 0;
    result = HttpCreateUrlGroup(serverSessionId, &urlGroupId, 0);
    if (NO_ERROR != result) goto cleanup3;
    wprintf(L"Url group created.\n");

    result = HttpAddUrlToUrlGroup(urlGroupId, L"http://*:1234/", NULL, 0);
    if (NO_ERROR != result) {
        wprintf(L"HttpAddUrlToUrlGroup failed with result code %lu.\n", result);
        goto cleanup4;
    }
    wprintf(L"Url added to group.\n");

    HTTP_BINDING_INFO serverBinding;
    serverBinding.Flags.Present = 1;
    serverBinding.RequestQueueHandle = reqQueue;
    result = HttpSetUrlGroupProperty(urlGroupId, HttpServerBindingProperty, &serverBinding, sizeof(HTTP_BINDING_INFO));
    if (NO_ERROR != result) {
        wprintf(L"HttpSetUrlGroupProperty failed with result code %lu.\n", result);
        goto cleanup4;
    }

    // Ready to receive HTTP requests.
    {
        HTTP_REQUEST_ID requestId;
        PHTTP_REQUEST   pRequest;
        ULONG           reqBufLen = sizeof(HTTP_REQUEST) + 16384;
        ULONG           bytesRead;

        pRequest = (PHTTP_REQUEST) HeapAlloc(GetProcessHeap(), 0, reqBufLen);
        if (pRequest == NULL)
        {
            wprintf(L"HeapAlloc failed!\n");
            goto cleanup4;
        }

        for (;;)
        {
            HTTP_SET_NULL_ID(&requestId);

            RtlZeroMemory(pRequest, reqBufLen);

            wprintf(L"Receiving request...\n");
            result = HttpReceiveHttpRequest(reqQueue, requestId, 0, pRequest, reqBufLen, &bytesRead, NULL);
            if (NO_ERROR != result)
            {
                wprintf(L"HTTP request error!\n");
                break;
            }
            wprintf(L"HTTP request received!\n");

#if 0
            PHTTP_RESPONSE_V2   pResponse;
            ULONG               bytesSent;
            result = HttpSendHttpResponse(reqQueue, requestId, 0, pResponse, NULL, &bytesSent, NULL, 0, NULL, NULL);
            if (NO_ERROR != result)
            {
                break;
            }
#endif
        }
    }

cleanup4:
    result = HttpCloseUrlGroup(urlGroupId);
    if (result != 0UL)
    {
        return -1;
    }
    wprintf(L"Url group closed.\n");

cleanup3:
    result = HttpCloseServerSession(serverSessionId);
    if (result != 0UL)
    {
        return -1;
    }
    wprintf(L"HTTP server session closed.\n");

cleanup2:
    result = HttpCloseRequestQueue(reqQueue);
    if (result != 0UL)
    {
        return -1;
    }
    wprintf(L"HTTP request queue closed.\n");

cleanup1:
    result = HttpTerminate(HTTP_INITIALIZE_SERVER, 0);
    if (result != 0UL)
    {
        return -1;
    }
    wprintf(L"HTTP terminated.\n");

cleanup0:
    return 0;
}
