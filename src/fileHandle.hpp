#ifndef __FILE_HANDLE_HPP__
#define __FILE_HANDLE_HPP__

#include <string>
#include <iostream>
#include <fstream>
#include <string.h>
#include <signal.h>
#include <memory>
#include <event2/event.h>
#include <event2/http.h>
#include <event2/buffer.h>
#include <event2/keyvalq_struct.h>
#include <json/json.h>
#include "SecCodeGenerator.hpp"

class FileHandler
{
public:
    FileHandler(std::shared_ptr<SecCodeGenerator> secCodeGenerator, std::string ipAddr, uint16_t port);

    ~FileHandler();

    void init();

    std::string getCurPath();
    std::string getSecCode();

    static void upload_handler(struct evhttp_request *req, void *arg);
    static void download_handler(struct evhttp_request *req, void *arg);
    static void getCacheDir_handler(struct evhttp_request *req, void *arg);
    static void signal_cb(evutil_socket_t fd, short event, void *arg);

private:
    std::string getRequestBuffer(struct evhttp_request *req);
    std::optional<Json::Value> parseJsonString(const std::string& jsonStr);
    std::string getfileNameFromContentDisposition(const char *contentDisposition);
    void printRequestHeaders(struct evhttp_request *req);
    bool isSecCodeRight(const Json::Value& jsonObj);
    std::string getBoundary(const char* contentType);
    bool parseMultipart(evhttp_request *req, void *arg);

    struct event_base* base_;
    struct evhttp *http_;
    struct event *sig_int_;

    std::shared_ptr<SecCodeGenerator> secCodeGenerator_;
    std::string ipAddr_;
    uint16_t port_;
};

#endif//__FILE_HANDLE_HPP__
