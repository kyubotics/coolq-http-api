#include "./crypt.h"

#include <bcrypt.h>

#include "utils/md5.h"

using namespace std;

string hmac_sha1_hex(const string &key, const string &msg) {
    // see https://stackoverflow.com/a/22155681/2400463

    stringstream hmac_hex_ss;

    #define NT_SUCCESS(Status) (((NTSTATUS)(Status)) >= 0)
    #define STATUS_UNSUCCESSFUL ((NTSTATUS)0xC0000001L)

    BCRYPT_ALG_HANDLE hAlg = NULL;
    BCRYPT_HASH_HANDLE hHash = NULL;
    NTSTATUS status = STATUS_UNSUCCESSFUL;
    DWORD cbData = 0, cbHash = 0, cbHashObject = 0;
    PBYTE pbHashObject = NULL;
    PBYTE pbHash = NULL;

    //open an algorithm handle
    if (!NT_SUCCESS(status = BCryptOpenAlgorithmProvider(
        &hAlg,
        BCRYPT_SHA1_ALGORITHM,
        NULL,
        BCRYPT_ALG_HANDLE_HMAC_FLAG))) {
        goto cleanup;
    }

    //calculate the size of the buffer to hold the hash object
    if (!NT_SUCCESS(status = BCryptGetProperty(
        hAlg,
        BCRYPT_OBJECT_LENGTH,
        (PBYTE) &cbHashObject,
        sizeof(DWORD),
        &cbData,
        0))) {
        goto cleanup;
    }

    //allocate the hash object on the heap
    pbHashObject = (PBYTE) HeapAlloc(GetProcessHeap(), 0, cbHashObject);
    if (NULL == pbHashObject) {
        goto cleanup;
    }

    //calculate the length of the hash
    if (!NT_SUCCESS(status = BCryptGetProperty(
        hAlg,
        BCRYPT_HASH_LENGTH,
        (PBYTE) &cbHash,
        sizeof(DWORD),
        &cbData,
        0))) {
        goto cleanup;
    }

    //allocate the hash buffer on the heap
    pbHash = (PBYTE) HeapAlloc(GetProcessHeap(), 0, cbHash);
    if (NULL == pbHash) {
        goto cleanup;
    }

    //create a hash
    if (!NT_SUCCESS(status = BCryptCreateHash(
        hAlg,
        &hHash,
        pbHashObject,
        cbHashObject,
        reinterpret_cast<PBYTE>(const_cast<char *>(key.data())),
        key.size(),
        0))) {
        goto cleanup;
    }

    //hash some data
    if (!NT_SUCCESS(status = BCryptHashData(
        hHash,
        reinterpret_cast<PBYTE>(const_cast<char *>(msg.data())),
        msg.size(),
        0))) {
        goto cleanup;
    }

    //close the hash
    if (!NT_SUCCESS(status = BCryptFinishHash(
        hHash,
        pbHash,
        cbHash,
        0))) {
        goto cleanup;
    }

    // Output to the hex string.
    for (DWORD i = 0; i < cbHash; i++) {
        hmac_hex_ss << hex << setfill('0') << setw(2) << static_cast<unsigned int>(pbHash[i]);
    }

cleanup:
    if (hAlg) {
        BCryptCloseAlgorithmProvider(hAlg, 0);
    }
    if (hHash) {
        BCryptDestroyHash(hHash);
    }
    if (pbHashObject) {
        HeapFree(GetProcessHeap(), 0, pbHashObject);
    }
    if (pbHash) {
        HeapFree(GetProcessHeap(), 0, pbHash);
    }

    #undef NT_SUCCESS
    #undef STATUS_UNSUCCESSFUL

    return hmac_hex_ss.str();
}

string md5_hash_hex(const string &input) {
    return MD5(input).toStr();
}
