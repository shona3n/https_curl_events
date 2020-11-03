#ifndef CURL_HTTPS__H
#define CURL_HTTPS__H

#include <iostream>
#include <string.h>

#define HTTPS_CA_VERIFY_PEM "ebo_cacert.pem"
#define CURL_CONNECTION_TIMEOUT_MS 10000
#define CURL_TIMEOUT_MS 10000

typedef struct _FtpFile {
    const char *filename;
    FILE *stream;
} FtpFile;

void curlGlobalInit(); // only call one time in a program
void curlGlobalCleanup(); // for clean https_global_init before program done

/*
 * @brief Get request
 * @param url: url to visit
 * @param response: save response from url
 * @param isCA: if http, set to 0, else if https and need to ssl verifypeer, set to 1
 * @return -1: init curl failed;
 *         -2: curl_easy_perform failed;
 *          0: curl_easy_perform request success.
 *         else: curl_easy_perform error status code
*/
int curlGet(const std::string &url, std::string &response, int isCA = 0);

/*
 * @brief Post request
 * @param url: url to visit
 * @param body: post url body
 * @param respHeader: save response header
 * @param respBody: save response body
 * @param isCA: if https and need to ssl verifypeer, set to 1, else set to 0
 * @return -1: init curl failed;
 *         -2: curl_easy_perform failed;
 *          0: curl_easy_perform request success.
 *         else: curl_easy_perform error status code
*/
int curlPost(const std::string &url, std::string body,
             std::string &respHeader, std::string &respBody, int isCA = 0);

int curlFtpDownload(const std::string &url, std::string &desFilepath, int isCA = 0);

// put file
int curlFtpUpload(const std::string &url, const std::string &srcFilepath,
                  const std::string &loadingFilename, const std::string &loadedFilename, int isCA = 0);


// post file
int curlFtpUploadWithPost(const std::string &url, const std::string &srcFilepath,
                  const std::string &loadingFilename, const std::string &loadedFilename, int isCA = 0);

#endif // CURL_HTTPS__H
