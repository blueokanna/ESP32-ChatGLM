#ifndef _CUSTOM_JWT_H_
#define _CUSTOM_JWT_H_

#define SHA256_HASH 32

#include <Crypto.h>
#include <SHA256.h>
#include "Base64URL.h"

class CustomJWT
{
public:
    uint8_t *secret;
    char *alg;
    char *typ;
    char *sign_type;
    size_t maxHeadLen;
    size_t maxPayloadLen;
    size_t maxSigLen;

    char *header = nullptr;
    char *payload = nullptr;
    char *signature = nullptr;
    size_t headerLength;
    size_t payloadLength;
    size_t signatureLength;
    size_t b64HeaderLen;
    size_t b64PayloadLen;
    size_t b64SigLen;

    char *out = nullptr;
    size_t maxOutputLen;
    size_t outputLength;

    /**
     * @brief Construct a new CustomJWT object
     * 
     * @param secret The Hashing key
     * @param maxPayloadLen Maximum expected length of payload
     * @param maxHeadLen Maximum expected length of header. Defaults to 40.
     * @param maxSigLen Maximum expected length of signature. Defaults to 32.
     * @param alg Encryption algorithm used. Defaults to HS256.
     * @param typ Header type. Defaults to JWT.
     */
    CustomJWT(char *secret, 
              size_t maxPayloadLen, 
              size_t maxHeadLen = 40, 
              size_t maxSigLen = 32, 
              char *alg = "HS256", 
              char *typ = "JWT",
              char *sign_type = "SIGN")
    {
        this->secret = (uint8_t *)secret;
        this->alg = alg;
        this->typ = typ;
        this->sign_type = sign_type;
        this->maxHeadLen = maxHeadLen;
        this->maxPayloadLen = maxPayloadLen;
        this->maxSigLen = maxSigLen;
        this->generateSignaturePointer = nullptr;
        this->staticAllocation = false;
    }

    /**
     * @brief Construct a new CustomJWT object providing own char arrays to store JWT output
     * 
     * @param secret The Hashing key
     * @param header Pointer to where the JWT header will be stored
     * @param headLen Size of header array
     * @param payload Pointer to where the JWT payload will be stored
     * @param payloadLen Size of payload array
     * @param signature Pointer to where the JWT signature will be stored
     * @param signatureLen Size of signature array
     * @param out Pointer to where the final output token will be stored
     * @param outLen Maximum output length
     * @param alg Encryption algorithm used. Defaults to HS256.
     * @param typ Header type. Defaults to JWT.
     */
    CustomJWT(char *secret, 
              char *header,
              size_t headLen,
              char *payload,
              size_t payloadLen,
              char *signature, 
              size_t signatureLen,
              char *out,
              size_t outLen,
              char *alg = "HS256", 
              char *typ = "JWT", 
              char *sign_type = "SIGN")
    {
        this->secret = (uint8_t *)secret;
        this->alg = alg;
        this->typ = typ;
        this->sign_type = sign_type;
        this->header = header;
        this->maxHeadLen = headLen;
        this->payload = payload;
        this->maxPayloadLen = payloadLen;
        this->signature = signature;
        this->maxSigLen = signatureLen;
        this->out = out;
        this->maxOutputLen = outLen;
        this->generateSignaturePointer = nullptr;
        this->staticAllocation = true;
        this->memoryAllocationDone = false;
    }

    /**
     * @brief Construct a new CustomJWT object with a custom signature generation function
     * 
     * @param secret The Hashing key
     * @param maxPayloadLen Maximum expected length of payload
     * @param maxHeadLen Maximum expected length of header. Defaults to 40.
     * @param maxSigLen Maximum expected length of signature. Defaults to 32.
     * @param generateSignaturePointer Pointer to hashing function for signature
     * @param alg Encryption algorithm used
     * @param typ Header type. Defaults to JWT.
     */
    CustomJWT(char *secret, 
              size_t maxPayloadLen, 
              size_t maxHeadLen, 
              size_t maxSigLen, 
              char *alg,
              void (*generateSignaturePointer)(char *output, size_t *outputLen, void *secret, size_t secretLen, void *data, size_t dataLen),
              char *typ = "JWT", 
              char *sign_type = "SIGN")
    {
        this->secret = (uint8_t *)secret;
        this->alg = alg;
        this->typ = typ;
        this->sign_type = sign_type;
        this->maxHeadLen = maxHeadLen;
        this->maxPayloadLen = maxPayloadLen;
        this->maxSigLen = maxSigLen;
        this->generateSignaturePointer = generateSignaturePointer;
        this->staticAllocation = false;
    }

    /**
     * @brief Construct a new CustomJWT object with a custom signature generation function
     * 
     * @param secret The Hashing key
     * @param header Pointer to where the JWT header will be stored
     * @param headLen Size of header array
     * @param payload Pointer to where the JWT payload will be stored
     * @param payloadLen Size of payload array
     * @param signature Pointer to where the JWT signature will be stored
     * @param signatureLen Size of signature array
     * @param out Pointer to where the final output token will be stored
     * @param outLen Maximum output length
     * @param alg Hashing algorithm used
     * @param generateSignaturePointer Pointer to hashing function for signature
     * @param typ Header type. Defaults to JWT.
     */
    CustomJWT(char *secret, 
              char *header,
              size_t headLen,
              char *payload,
              size_t payloadLen,
              char *signature, 
              size_t signatureLen,
              char *out,
              size_t outLen,
              char *alg,
              void (*generateSignaturePointer)(char *output, size_t *outputLen, void *secret, size_t secretLen, void *data, size_t dataLen),
              char *typ = "JWT", 
              char *sign_type = "SIGN")
    {
        this->secret = (uint8_t *)secret;
        this->alg = alg;
        this->typ = typ;
        this->sign_type = sign_type;
        this->header = header;
        this->maxHeadLen = headLen;
        this->payload = payload;
        this->maxPayloadLen = payloadLen;
        this->signature = signature;
        this->maxSigLen = signatureLen;
        this->out = out;
        this->maxOutputLen = outLen;
        this->generateSignaturePointer = generateSignaturePointer;
        this->staticAllocation = true;
        this->memoryAllocationDone = false;
    }

    /**
     * @brief Allocate memory for internal char pointers
     * @return true: Memory deallocated successfully \n false: Using Static allocation
     */
    bool allocateJWTMemory()
    {
        if(staticAllocation)
            return false;
        b64HeaderLen = (size_t)(4.0 * (maxHeadLen / 3.0)) + 5;
        b64PayloadLen = (size_t)(4.0 * (maxPayloadLen / 3.0)) + 5;
        b64SigLen = (size_t)(4.0 * (maxSigLen / 3.0)) + 5;
        maxOutputLen = (size_t)(b64HeaderLen + b64PayloadLen + b64SigLen + 4);

        header = (char *)malloc(b64HeaderLen * sizeof(char));
        payload = (char *)malloc(b64PayloadLen * sizeof(char));
        signature = (char *)malloc(b64SigLen * sizeof(char));
        out = (char *)malloc(maxOutputLen * sizeof(char));

        memoryAllocationDone = true;
        return true;
    }

    /**
     * @brief Function to generate signature for JWT
     * @param output Pointer to where the final b64 encoded output will be stored
     * @param outputLen Pointer to where the output length will be stored
     * @param secret Pointer to the secret for HMAC encoding
     * @param secretLen Length of secret
     * @param data Pointer to the data from which signature is to be generated
     * @param dataLen Size of data from which signature is to be generated
    */
    void generateSignature(char *output, size_t *outputLen, void *secret, size_t secretLen, void *data, size_t dataLen) {
        uint8_t hashed[SHA256_HASH];
        memset(hashed, 0, SHA256_HASH);
        hmac<SHA256>(hashed, SHA256_HASH, secret, secretLen, data, dataLen);
        Base64URL::base64urlEncode(hashed, SHA256_HASH, output, outputLen);
    }

    /**
     * @brief Encode given string into JWT. Final output stored in out. 
     * 
     * @param string Data to be encoded
     * @return true: Encoding success
     * @return false: Memory not allocated
     */
    bool encodeJWT(char *string)
    {
        if(!memoryAllocationDone && !staticAllocation)
            return false;

        if(!staticAllocation)
        {
            memset(header, 0, sizeof(char) * b64HeaderLen);
            memset(payload, 0, sizeof(char) * b64PayloadLen);
            memset(signature, 0, sizeof(char) * b64SigLen);
            memset(out, 0, sizeof(char) * maxOutputLen);
        }
        else
        {
            memset(header, 0, sizeof(char) * maxHeadLen);
            memset(payload, 0, sizeof(char) * maxPayloadLen);
            memset(signature, 0, sizeof(char) * maxSigLen);
            memset(out, 0, sizeof(char) * maxOutputLen);
        }

        char headerJSON[maxHeadLen];
        sprintf(headerJSON, "{\"alg\":\"%s\",\"typ\":\"%s\",\"sign_type\":\"%s\"}", alg, typ, sign_type);
        Base64URL::base64urlEncode(headerJSON, strlen(headerJSON), header, &headerLength);
        Base64URL::base64urlEncode(string, strlen(string), payload, &payloadLength);
        
        char toHash[payloadLength + headerLength + 3];
        memset(toHash, 0, payloadLength + headerLength + 3);
        sprintf(toHash, "%s.%s", header, payload);
        
        if(this->generateSignaturePointer == nullptr)
            generateSignature(signature, &signatureLength, secret, strlen((char *)secret), toHash, strlen(toHash));
        else
            generateSignaturePointer(signature, &signatureLength, secret, strlen((char *)secret), toHash, strlen(toHash));
        
        sprintf(out, "%s.%s", toHash, signature);
        outputLength = strlen(out);
        return true;
    }

    /**
     * @brief Validate and decode JWT. Result stored int header, payload and signature.
     * 
     * @param string JWT to be decoded
     * @returns Code 0: Decode success \n Code 1: Memory not allocated \n Code 2: Invalid JWT \n Code 3: Signature Mismatch
     */
    int decodeJWT(char *string)
    {
        if(!memoryAllocationDone && !staticAllocation)
            return 1;
        
        const char* delimiter = ".";
        char* b64Head;
        char* b64Payload;
        char* b64Signature;

        
        b64Head = strtok(string, delimiter);
        if(b64Head == 0)
            return 2;
        b64Payload = strtok(0, delimiter);
        if(b64Payload == 0)
            return 2;
        b64Signature = strtok(0, delimiter);
        if(b64Signature == 0)
            return 2;
        if(strtok(0, delimiter) != 0)
            return 2;
        
        char toCheckHash[strlen(b64Head) + strlen(b64Payload) + 3];
        memset(toCheckHash, 0, sizeof(toCheckHash));

        char testSignature[strlen(b64Signature)];
        memset(testSignature, 0, sizeof(testSignature));
        size_t testSignatureLength;

        sprintf(toCheckHash, "%s.%s", b64Head, b64Payload);

        if(generateSignaturePointer == nullptr)
            generateSignature(testSignature, &testSignatureLength, secret, strlen((char*)secret), toCheckHash, strlen(toCheckHash));
        else
            generateSignaturePointer(testSignature, &testSignatureLength, secret, strlen((char*)secret), toCheckHash, strlen(toCheckHash));

        if(testSignatureLength != strlen(b64Signature)){
            return 3;
        }

        if(strncmp(b64Signature, testSignature, testSignatureLength) != 0) {
            return 3;
        }
        
        if(!staticAllocation)
        {
            memset(header, 0, sizeof(char) * b64HeaderLen);
            memset(payload, 0, sizeof(char) * b64PayloadLen);
            memset(signature, 0, sizeof(char) * b64SigLen);
        }
        else
        {
            memset(header, 0, sizeof(char) * maxHeadLen);
            memset(payload, 0, sizeof(char) * maxPayloadLen);
            memset(signature, 0, sizeof(char) * maxSigLen);
        }
        
        Base64URL::base64urlDecode(b64Head, strlen(b64Head), header, &headerLength);
        Base64URL::base64urlDecode(b64Payload, strlen(b64Payload), payload, &payloadLength);
        sprintf(signature, "%s", b64Signature);
        signatureLength = strlen(signature);
        
        return 0;
    }

    /**
     * @brief Clear all allocated memory
     * @return true: Memory deallocated successfully \n false: Using Static allocation
     */
    bool clear()
    {
        if(staticAllocation)
            return false;
        if(header != nullptr)
        {
            free(header);
            header = nullptr;
        }
        if(payload != nullptr)
        {
            free(payload);
            payload = nullptr;
        }
        if(signature != nullptr)
        {
            free(signature);
            signature = nullptr;
        }
        if(out != nullptr)
        {
            free(out);
            out = nullptr;
        }
        memoryAllocationDone = false;
        return true;
    }

private:
    bool memoryAllocationDone = false;
    bool staticAllocation = false;
    void (*generateSignaturePointer)(char *output, size_t *outputLen, void *secret, size_t secretLen, void *data, size_t dataLen);
};

#endif //_CUSTOM_JWT_H_
