#include <event2/event.h>
#include <event2/http.h>
#include <event2/buffer.h>
#include <event2/util.h>
#include <event2/keyvalq_struct.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <curl/curl.h>

struct MemoryStruct {
    char *memory;
    size_t size;
};

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;

    char *ptr = realloc(mem->memory, mem->size + realsize + 1);
    if(ptr == NULL) {
        printf("not enough memory (realloc returned NULL)\n");
        return 0;
    }

    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}

char *perform_query(const char *data) {
    CURL *curl;
    CURLcode res;
    struct MemoryStruct chunk;

    chunk.memory = malloc(1);
    chunk.size = 0;

    curl_global_init(CURL_GLOBAL_ALL);

    curl = curl_easy_init();
    if(curl) {
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Authorization: Bearer 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");
        headers = curl_slist_append(headers, "Content-Type: application/json");

        curl_easy_setopt(curl, CURLOPT_URL, "https://api-inference.huggingface.co/models/mistralai/Mistral-7B-Instruct-v0.3");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

        res = curl_easy_perform(curl);

        if(res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        } else {
            printf("%lu bytes retrieved\n", (unsigned long)chunk.size);
        }

        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
    }

    curl_global_cleanup();

    return chunk.memory;
}

void handle_request(struct evhttp_request *req, void *arg) {
    struct evbuffer *evb = evbuffer_new();

    struct evhttp_uri *decoded = evhttp_uri_parse(evhttp_request_get_uri(req));
    const char *query = evhttp_uri_get_query(decoded);

    struct evkeyvalq params;
    evhttp_parse_query_str(query, &params);
    const char *text_input = evhttp_find_header(&params, "text_input");

    if (text_input == NULL) {
        evhttp_add_header(evhttp_request_get_output_headers(req), "Access-Control-Allow-Origin", "*");
        evbuffer_add_printf(evb, "Missing parameters\n");
        evhttp_send_reply(req, HTTP_BADREQUEST, "Bad Request", evb);
        evbuffer_free(evb);
        return;
    }

    char data[2048];
    snprintf(data, sizeof(data), "{\"inputs\": \"%s\"}", text_input);
    char *response_text = perform_query(data);

    // Escape special characters in response_text
    char *escaped_response_text = malloc(strlen(response_text) * 2 + 1); // Allocate enough space
    char *src = response_text;
    char *dest = escaped_response_text;
    while (*src) {
        if (*src == '"' || *src == '\\') {
            *dest++ = '\\';
        }
        *dest++ = *src++;
    }
    *dest = '\0';

    // Print response_text for debugging
    printf("Response text: %s\n", response_text);

    char json_response[2048];
    snprintf(json_response, sizeof(json_response), "{\"response\": \"%s\"}", escaped_response_text);

    // Print json_response for debugging
    printf("JSON Response: %s\n", json_response);

    evhttp_add_header(evhttp_request_get_output_headers(req), "Access-Control-Allow-Origin", "*");
    evhttp_add_header(evhttp_request_get_output_headers(req), "Content-Type", "application/json");

    evbuffer_add_printf(evb, "%s", json_response);

    free(response_text);
    free(escaped_response_text);
    evhttp_send_reply(req, HTTP_OK, "OK", evb);
    evbuffer_free(evb);
}

int main() {
    struct event_base *base = event_base_new();
    struct evhttp *http = evhttp_new(base);
    evhttp_bind_socket(http, "0.0.0.0", 8080);
    evhttp_set_gencb(http, handle_request, NULL);

    event_base_dispatch(base);

    evhttp_free(http);
    event_base_free(base);
    return 0;
}
