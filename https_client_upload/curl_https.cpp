#include "curl_https.h"
#include <sys/stat.h>
#include "curl/curl.h"

void curlGlobalInit() {
    curl_global_init(CURL_GLOBAL_ALL);
}

void curlGlobalCleanup() {
    curl_global_cleanup();
}

inline size_t writeStringResponse(void *buffer, size_t size, size_t nmemb, void *userp) {
    ((std::string *)userp)->append((char *)buffer, 0, size * nmemb);
    return size * nmemb;
}

size_t writeGetResponse(void *buffer, size_t size, size_t nmemb, void *userp) {
    return writeStringResponse(buffer, size, nmemb, userp);
}

int curlGet(const std::string &url, std::string &response, int isCA) {
    CURL *curl = curl_easy_init();
    if (!curl) {
        return -1;
    }
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeGetResponse);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, CURL_CONNECTION_TIMEOUT_MS); // ibcurl存在毫秒超时bug,如果设备小于1000ms立即返回失败
    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, CURL_TIMEOUT_MS); // TIMEOUT

    if (isCA == 0) {
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);
    } else {
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1);
        curl_easy_setopt(curl, CURLOPT_CAINFO, HTTPS_CA_VERIFY_PEM);
    }

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        std::cout << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
    }
    curl_easy_cleanup(curl);
    return res;
}

size_t writePostHeaderResponse(void *buffer, size_t size, size_t nmemb, void *userp) {
    return writeStringResponse(buffer, size, nmemb, userp);
}

size_t writePostBodyResponse(void *buffer, size_t size, size_t nmemb, void *userp) {
    return writeStringResponse(buffer, size, nmemb, userp);
}

int curlPost(const std::string &url, std::string body, std::string &respHeader, std::string &respBody, int isCA) {
    // set header
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type:application/json;charset=UTF-8");
    CURL *curl = curl_easy_init();
    if (!curl) {
        return -1;
    }
    // set curl to send request
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POST, 1);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    // set curl's handler of response
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, writePostHeaderResponse);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writePostBodyResponse);
    curl_easy_setopt(curl, CURLOPT_WRITEHEADER, &respHeader);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &respBody);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, CURL_CONNECTION_TIMEOUT_MS); //libcurl存在毫秒超时bug,如果设备小于1000ms立即返回失败
    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, CURL_TIMEOUT_MS); //设置超时时间
    if (isCA == 0) { // set not verify peer and host
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);
    } else {
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1);
        curl_easy_setopt(curl, CURLOPT_CAINFO, HTTPS_CA_VERIFY_PEM);
    }
    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        std::cout << "post curl_easy_perform() error: " << curl_easy_strerror(res) << std::endl;
    }
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    return res;
}

size_t wirteGetFile(void *buffer, size_t size, size_t nmemb, void *userp) {
    FtpFile *out = (FtpFile *)userp;
    if (out && !out->stream) {
        out->stream = fopen(out->filename, "wb");
        if (!out->stream) {
            return -1;
        }
    }
    return fwrite((char *)buffer, size, nmemb, out->stream);
}

int curlFtpDownload(const std::string &url, std::string &desFilepath, int isCA) {
    CURL *curl = curl_easy_init();
    if (!curl) {
        return -1;
    }
    FtpFile ftpF = {desFilepath.c_str(), NULL};

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, wirteGetFile);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ftpF);
    if (isCA == 0) { // set not verify peer and host
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);
    } else {
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1);
        curl_easy_setopt(curl, CURLOPT_CAINFO, HTTPS_CA_VERIFY_PEM);
    }
    CURLcode statusCode = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    if (statusCode != 0) {
        std::cout << "https get file curl_easy_preform() error: "
                  << curl_easy_strerror(statusCode) << std::endl;;
    }
    if (ftpF.stream) {
        fclose(ftpF.stream);
    }
    return statusCode;
}

size_t readFtpUpload(void *ptr, size_t size, size_t nmemb, void *userdata)
{
  FILE *readhere = (FILE *)userdata;
  curl_off_t nread;

  /* copy as much data as possible into the 'ptr' buffer, but no more than
     'size' * 'nmemb' bytes! */
  size_t retcode = fread(ptr, size, nmemb, readhere);

  nread = (curl_off_t)retcode;

  fprintf(stderr, "*** We read %" CURL_FORMAT_CURL_OFF_T
          " bytes from file\n", nread);
  return retcode;
}

int curlFtpUpload(const std::string &url, const std::string &srcFilepath,
                  const std::string &loadingFilename, const std::string &loadedFilename, int isCA) {
    struct stat fileInfo;
    if (stat(srcFilepath.c_str(), &fileInfo)) {
        std::cout << "Couldn't open: " << srcFilepath << std::endl;
        return -1;
    }
    FILE *fp = fopen(srcFilepath.c_str(), "rb");
    CURL *curl = curl_easy_init(); // get a curl handle
    if (!curl) {
        return -2;
    }
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1); // enable uploading
    curl_easy_setopt(curl, CURLOPT_PUT, 1);
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str()); // specify target
    if (isCA == 0) { // set not verify peer and host
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);
    } else {
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1);
        curl_easy_setopt(curl, CURLOPT_CAINFO, HTTPS_CA_VERIFY_PEM);
    }

    // set post head
    struct curl_slist *headerlist = NULL;
    std::string buf = "RNFR ";
    buf.append(loadingFilename);
    headerlist = curl_slist_append(headerlist, buf.c_str());
    buf = "RFTO ";
    buf.append(loadedFilename);
    headerlist = curl_slist_append(headerlist, buf.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTQUOTE, headerlist);
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, readFtpUpload);
    curl_easy_setopt(curl, CURLOPT_READDATA, fp);
    curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, (curl_off_t)fileInfo.st_size);

    CURLcode statusCode = curl_easy_perform(curl);
    if (statusCode != CURLE_OK) {
        std::cout << "ftp upload failed: " << curl_easy_strerror(statusCode);
    } else {
        /* now extract transfer info */
        curl_off_t speedUpload, totalTime;
        curl_easy_getinfo(curl, CURLINFO_SPEED_UPLOAD_T, &speedUpload);
        curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME_T, &totalTime);

        fprintf(stderr, "Speed: %" CURL_FORMAT_CURL_OFF_T " bytes/sec during %"
                CURL_FORMAT_CURL_OFF_T ".%06ld seconds\n",
                speedUpload,
                (totalTime / 1000000), (long)(totalTime % 1000000));
    }
    curl_slist_free_all(headerlist);
    curl_easy_cleanup(curl);
    fclose(fp);
    return statusCode;
}

int curlFtpUploadWithPost(const std::string &url, const std::string &srcFilepath,
                  const std::string &loadingFilename, const std::string &loadedFilename, int isCA) {
    if (url.empty() || srcFilepath.empty()) {
        return -1;
    }
    FILE *file = fopen(srcFilepath.c_str(), "rb");
    if (!file) {
        std::cout << __FUNCTION__ << " open file failed: " << srcFilepath << std::endl;
        return -1;
    }
    struct stat fileInfo;
    stat(srcFilepath.c_str(), &fileInfo);
    char *fileData = NULL;
    fileData = (char *)malloc(fileInfo.st_size);
    if (!fileData) {
        return -1;
    }
    int readCount = fread(fileData, 1, fileInfo.st_size, file);
    if (readCount != fileInfo.st_size) {
        fclose(file);
        free(fileData);
        return -1;
    }
}
