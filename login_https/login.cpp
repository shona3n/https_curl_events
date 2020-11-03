//
// Created by ldw on 2018/11/8.
//
#include "cJSON.h"
#include <curl/curl.h>
#include<string.h>

#define RESPONSE_DATA_LEN 4096

//�������շ�����һ��buffer
typedef struct login_response_data
{
    login_response_data() {
        memset(data, 0, RESPONSE_DATA_LEN);
        data_len = 0;
    }

    char data[RESPONSE_DATA_LEN];
    int data_len;

}response_data_t;


//�����ӷ��������ص����ݣ������ݿ�����arg��
size_t deal_response(void *ptr, size_t n, size_t m, void *arg)
{
    int count = m*n;

    response_data_t *response_data = (response_data_t*)arg;

    memcpy(response_data->data, ptr, count);

    response_data->data_len = count;

    return response_data->data_len;
}

#define POSTDATA "{\"username\":\"gailun\",\"password\":\"123123\",\"driver\":\"yes\"}"

int main()
{


    char *post_str = NULL;

	CURL* curl = NULL;
	CURLcode res;
    response_data_t responseData;//ר��������Ŵӷ��������ص�����
    //��ʼ��curl���
    curl = curl_easy_init();
    if(curl == NULL) {
        return 1;
    }

    //��װһ������Э��
    /*

       ====������˵�Э��====
     http://ip:port/login [json_data]
    {
        username: "gailun",
        password: "123123",
        driver:   "yes"
    }
     *
     *
     * */
    //��1����װһ��json�ַ���
    cJSON *root = cJSON_CreateObject();

    cJSON_AddStringToObject(root, "username", "ldw");
    cJSON_AddStringToObject(root, "password", "123123");
    cJSON_AddStringToObject(root, "driver", "yes");

    post_str = cJSON_Print(root);
    cJSON_Delete(root);
    root = NULL;


    //(2) ��web������ ����http���� ����post���� json�ַ���
    //1 ����curl url
    // curl_easy_setopt(curl, CURLOPT_URL, "https://172.16.1.96:8080/login");
    curl_easy_setopt(curl, CURLOPT_URL, "https://0.0.0.0:8080/login");

    //�ͻ��˺���CA֤����֤ ����https����֤����֤
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, false);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);

    //2 ����post���󿪹�
    curl_easy_setopt(curl, CURLOPT_POST, true);
    //3 ����post����
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_str);

    //4 �趨һ��������������Ӧ�Ļص�����
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, deal_response);

    //5 ���ص���������һ���β�
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseData);

    //6 ���������������,�ȴ�����������Ӧ
    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        return 1;
    }
    curl_easy_cleanup(curl);

    //��3��  ������������Ӧ������ �˿̵�responseData���Ǵӷ�������ȡ������
    /*

      //�ɹ�
    {
        result: "ok",
    }
    //ʧ��
    {
        result: "error",
        reason: "why...."
    }

     *
     * */
    //(4) �������������ص�json�ַ���
    //cJSON *root;
    root = cJSON_Parse(responseData.data);

    cJSON *result = cJSON_GetObjectItem(root, "result");
    if(result && strcmp(result->valuestring, "ok") == 0) {
	    printf("data:%s\n",responseData.data);
        //��½�ɹ�
        return 0;

    }
    else {
        //��½ʧ��
        cJSON* reason = cJSON_GetObjectItem(root, "reason");
        if (reason) {
            //��֪����
           return 1;

        }
        else {
            //δ֪�Ĵ���
          return 1;
        }

        return 1;
    }

    return 0;
}

