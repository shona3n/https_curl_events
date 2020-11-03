#include <iostream>
#include <string.h>
#include "curl_https.h"

void testGet() {
    curlGlobalInit();
    std::string response;
    int res = curlGet("https://www.baidu.com/", response, 0);
    // int res = curlGet("https://0.0.0.0:8080/upload", response, 0);
    if (res != 0) {
        std::cout << "request error!!" << res << std::endl;
    } else {
        std::cout << "data:" << response << std::endl;
    }
    curlGlobalCleanup();
}

void testPost() {
    curlGlobalInit();
    std::string respHeader;
    std::string respBody;
    int res = curlPost("https://www.baidu.com", "name=daniel&project=curl", respHeader, respBody, 0);
    if (res != 0) {
        std::cout << "request error!!" << res << std::endl;
    } else {
        std::cout << respHeader << std::endl;
        std::cout << "body: " << std::endl << respBody << std::endl;
    }
    curlGlobalCleanup();
}

void testGetFile() {
    std::cout << "******TEST httpGetFile FUNCTION" << std::endl;
    curlGlobalInit();
    std::string filepath = "a.jpg";
    std::string url = "https://timgsa.baidu.com/timg?image&quality=80&size=b9999_10000&sec=1604061839564&di=d89257ddd30e5eee06ca9b1f9301bbf5&imgtype=0&src=http%3A%2F%2Fa0.att.hudong.com%2F56%2F12%2F01300000164151121576126282411.jpg";
    int res = curlFtpDownload(url, filepath, 0);
    if (res != 0) {
        std::cout << "request get file error!!" << res << std::endl;
    }

    curlGlobalCleanup();
    std::cout << "******TEST httpGetFile FUNCTION DONE" << std::endl;
}

void testUploadFile() {
   std::cout << "******TEST curlFtpUpload FUNCTION" << std::endl;
    curlGlobalInit();
    std::string url = "https://172.16.6.47:8080/upload";
    int res = curlFtpUpload(url, "a.jpg", "aing.jpg", "ad.jpg", 0);
    if (res != 0) {
        std::cout << "request upload file error!!" << res << std::endl;
    }
    curlGlobalCleanup();
    std::cout << "******TEST curlFtpUpload FUNCTION DONE" << std::endl;
}

int main() {
    testGet();
    testPost();
    testGetFile();
    testUploadFile();
    return 0;
}
