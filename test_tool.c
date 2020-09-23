//���������� �������, �������
//��������� ��������� �/��������� �� Dropbox ������������ �������� ����
//(������������� ������� � �������) � ������� ������������ HTTP-��������
//� REST API, �.�. �� ��������� ���������� ���������
//
//����� ������� �������� ��������� �������:
//     testtool login password put/get src_path dst_path
//
//����� ������ put ��������������� ���� src_path ����������� ��
//������ �� ���� dst_path.
//����� ������ get ����, ����������� �� ������� ��� ������ src_path
//����������� � ����������� �� ���������� ���� dst_path.

//����������� ������������ ������� ����� ��������������� access token 

#include <stdio.h>
#include <locale.h>
#include <string.h>
#include <curl/curl.h>

#pragma comment(lib, "libcurl.lib")


static size_t write_data(void* ptr, size_t size, size_t nmemb, void* stream)
{
    size_t written = fwrite(ptr, size, nmemb, (FILE*)stream);
    return written;
}

long int filesize(FILE* fp)
{
    long int save_pos, size_of_file;

    save_pos = ftell(fp);
    fseek(fp, 0L, SEEK_END);
    size_of_file = ftell(fp);
    fseek(fp, save_pos, SEEK_SET);
    return(size_of_file);
}


int main(int argc, char* argv[]) {
    
    setlocale(LC_ALL, "rus");
    
    char token[128]="\0"; 
    
    printf("%s", "������� access token: ");
    scanf_s("%69s", token, sizeof(token));

    char auth_bearer[128] = "Authorization: Bearer ";
    strcat_s(auth_bearer, 128, token);


    /*          ��������� ������              */
    if (strcmp(argv[3], "put") == 0)
    {
        FILE* file_handler;
        errno_t err;
        err = fopen_s(&file_handler, argv[4], "rb");
        if (err == 0)
        {
            printf("%s", "���� ������ �������");

            //��������� ������ ����� 
            long int file_size = filesize(file_handler);

            //������� ����� ��� ���������� �����
            void* buffer = malloc(file_size);
            fread(buffer, sizeof(char), file_size, file_handler);//������� ���� � ����� 
            fclose(file_handler); //������� ���� 


            CURL* curl_handle = curl_easy_init();//�������� ������ 


            char path_drbx[256] = "\0";
            // char file_name[64];
            strcat_s(path_drbx, 256, "Dropbox-API-Arg: {\"path\": \"");
            strcat_s(path_drbx, 256, argv[5]);
            strcat_s(path_drbx, 256, "\",\"mode\": \"add\",\"autorename\": true,\"mute\": false,\"strict_conflict\": false}");

            if (curl_handle)
            {
                /*          ���� ������������ HTTP ������� �� �������� ��������� �����            */

                curl_easy_setopt(curl_handle, CURLOPT_POST, 1L); // ������������� ��� ������� POST 
                curl_easy_setopt(curl_handle, CURLOPT_URL, "https://content.dropboxapi.com/2/files/upload");// ������  url �����
                curl_easy_setopt(curl_handle, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);

                //��������� ��������� 
                struct curl_slist* list = NULL;
                list = curl_slist_append(list, auth_bearer);
                list = curl_slist_append(list, "Transfer-Encoding: chunked");
                list = curl_slist_append(list, path_drbx);
                list = curl_slist_append(list, "Content-Type: application/octet-stream");

                curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, list);
                curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, buffer);//������ �� ������ ��� �������� �� ������ 
                curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDSIZE, file_size);
                curl_easy_setopt(curl_handle, CURLOPT_VERBOSE, 1L);//����� �������� ���������� 

                CURLcode res = curl_easy_perform(curl_handle);
                free(buffer);
            }

        }
        else
        {
            printf("%s", "�� ������� ����� ����");
        }

    }
    else if (strcmp(argv[3], "get") == 0)
    {
        CURL* curl_handle = curl_easy_init();//�������� ������ 

        char path_drbx[256] = "\0";
        strcat_s(path_drbx, 256, "Dropbox-API-Arg: {\"path\": \"");
        strcat_s(path_drbx, 256, argv[4]);
        strcat_s(path_drbx, 256, "\"}");

        /*          ���� ������������ HTTP ������� �� ��������� ��������� �����            */
        if (curl_handle)
        {
            curl_easy_setopt(curl_handle, CURLOPT_HTTPGET, 1L); // ������������� ��� ������� GET 
            curl_easy_setopt(curl_handle, CURLOPT_URL, "https://content.dropboxapi.com/2/files/download");// ������  url �����

            //��������� ��������� 
            struct curl_slist* list = NULL;
            list = curl_slist_append(list, auth_bearer);
            list = curl_slist_append(list, path_drbx);
            curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, list);
            curl_easy_setopt(curl_handle, CURLOPT_VERBOSE, 1L);//����� �������� ���������� 

            //������� ���� ��� ������ ���������� ������ 
            FILE* new_file;
            fopen_s(&new_file, argv[5], "wb");
            curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_data);
            curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, new_file);

            //��������� ������ 
            CURLcode res = curl_easy_perform(curl_handle);
            fclose(new_file);
        }
    }
    else
        printf("%s\n", "������ ����������� ������");
        
    
    return 0;
}


