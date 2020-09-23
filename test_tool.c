//Консольная утилита, которая
//позволяет загружать в/скачивать из Dropbox произвольный двоичный файл
//(произвольного формата и размера) с помощью формирования HTTP-запросов
//к REST API, т.е. не используя оберточных библиотек
//
//вызов утилиты выглядит следующим образом:
//     testtool login password put/get src_path dst_path
//
//После вызова put соответствующий файл src_path загружается на
//сервер по пути dst_path.
//После вызова get файл, находящийся на сервере под именем src_path
//скачивается и сохраняется по локальному пути dst_path.

//Авторизация производится вручную через сгенерированный access token 

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
    
    printf("%s", "Введите access token: ");
    scanf_s("%69s", token, sizeof(token));

    char auth_bearer[128] = "Authorization: Bearer ";
    strcat_s(auth_bearer, 128, token);


    /*          ОСНОВАНАЯ ЛОГИКА              */
    if (strcmp(argv[3], "put") == 0)
    {
        FILE* file_handler;
        errno_t err;
        err = fopen_s(&file_handler, argv[4], "rb");
        if (err == 0)
        {
            printf("%s", "Файл открыт успешно");

            //определим размер файла 
            long int file_size = filesize(file_handler);

            //Выделим буфер для считывания файла
            void* buffer = malloc(file_size);
            fread(buffer, sizeof(char), file_size, file_handler);//считали файл в буфер 
            fclose(file_handler); //закроем файл 


            CURL* curl_handle = curl_easy_init();//начинаем сессию 


            char path_drbx[256] = "\0";
            // char file_name[64];
            strcat_s(path_drbx, 256, "Dropbox-API-Arg: {\"path\": \"");
            strcat_s(path_drbx, 256, argv[5]);
            strcat_s(path_drbx, 256, "\",\"mode\": \"add\",\"autorename\": true,\"mute\": false,\"strict_conflict\": false}");

            if (curl_handle)
            {
                /*          Блок формирования HTTP запроса на отправку двоичного файла            */

                curl_easy_setopt(curl_handle, CURLOPT_POST, 1L); // устанавливаем вид запроса POST 
                curl_easy_setopt(curl_handle, CURLOPT_URL, "https://content.dropboxapi.com/2/files/upload");// задаем  url адрес
                curl_easy_setopt(curl_handle, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);

                //установим заголовки 
                struct curl_slist* list = NULL;
                list = curl_slist_append(list, auth_bearer);
                list = curl_slist_append(list, "Transfer-Encoding: chunked");
                list = curl_slist_append(list, path_drbx);
                list = curl_slist_append(list, "Content-Type: application/octet-stream");

                curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, list);
                curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, buffer);//данные из буфера для отправки на сервер 
                curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDSIZE, file_size);
                curl_easy_setopt(curl_handle, CURLOPT_VERBOSE, 1L);//вывод отчетной информации 

                CURLcode res = curl_easy_perform(curl_handle);
                free(buffer);
            }

        }
        else
        {
            printf("%s", "Не удалось найти файл");
        }

    }
    else if (strcmp(argv[3], "get") == 0)
    {
        CURL* curl_handle = curl_easy_init();//начинаем сессию 

        char path_drbx[256] = "\0";
        strcat_s(path_drbx, 256, "Dropbox-API-Arg: {\"path\": \"");
        strcat_s(path_drbx, 256, argv[4]);
        strcat_s(path_drbx, 256, "\"}");

        /*          Блок формирования HTTP запроса на получение двоичного файла            */
        if (curl_handle)
        {
            curl_easy_setopt(curl_handle, CURLOPT_HTTPGET, 1L); // устанавливаем вид запроса GET 
            curl_easy_setopt(curl_handle, CURLOPT_URL, "https://content.dropboxapi.com/2/files/download");// задаем  url адрес

            //установим заголовки 
            struct curl_slist* list = NULL;
            list = curl_slist_append(list, auth_bearer);
            list = curl_slist_append(list, path_drbx);
            curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, list);
            curl_easy_setopt(curl_handle, CURLOPT_VERBOSE, 1L);//вывод отчетной информации 

            //откроем файл для записи получаемых данных 
            FILE* new_file;
            fopen_s(&new_file, argv[5], "wb");
            curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_data);
            curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, new_file);

            //отправить запрос 
            CURLcode res = curl_easy_perform(curl_handle);
            fclose(new_file);
        }
    }
    else
        printf("%s\n", "Введен некоректный запрос");
        
    
    return 0;
}


