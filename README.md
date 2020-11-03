
libcurl和libevent实现http和https服务端，libcurl实现客户端，使用前需先安装libcurl和libevent。

ubuntu平台测试通过，arm平台交叉编译后也可以使用。

## https_client_upload
`g++ test_https.cpp curl_https.cpp  -o test_https -lcurl`