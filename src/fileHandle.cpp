#include <sstream>
#include <thread>
#include <chrono>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <optional>
#include <QThread>
#include "fileHandle.hpp"

namespace fs = std::filesystem;
using namespace std::literals::chrono_literals;

#ifndef HTTP_FORBIDDEN
#define HTTP_FORBIDDEN 403
#endif

#ifdef __WIN32
    #include <winsock2.h>
#endif

FileHandler::FileHandler(std::shared_ptr<SecCodeGenerator> secCodeGenerator, std::string ipAddr, uint16_t port)
    : secCodeGenerator_(secCodeGenerator)
    , ipAddr_(ipAddr)
    , port_(port)
{
    std::cout << "init " << __FUNCTION__ << "\n";
}

FileHandler::~FileHandler()
{
    evhttp_free(http_);
    // event_free(sig_int_);
    event_base_free(base_);

    #ifdef __WIN32
        WSACleanup();
    #endif
}

void FileHandler::init()
{
    base_ = event_base_new();
    http_ = evhttp_new(base_);
    evhttp_bind_socket(http_, ipAddr_.c_str(), port_);

    // sig_int_ = evsignal_new(base_, SIGINT, FileHandler::signal_cb, base_);
    // event_add(sig_int_, NULL);
    
    evhttp_set_cb(http_, "/upload",         FileHandler::upload_handler, this);
    evhttp_set_cb(http_, "/download",       FileHandler::download_handler, this);
    evhttp_set_cb(http_, "/getCacheDir",    FileHandler::getCacheDir_handler, this);

    event_base_dispatch(base_);
}

void FileHandler::printRequestHeaders(struct evhttp_request *req)
{
    struct evkeyvalq *hds = (struct evkeyvalq*)evhttp_request_get_input_headers(req);
    for (struct evkeyval *header = hds->tqh_first;
        header; header = header->next.tqe_next)
    {
        std::cout << "key: " << header->key << " value: " << header->value << "\n";
    }
}

std::string FileHandler::getfileNameFromContentDisposition(const char *contentDisposition)
{
    std::string fileName;
    if (contentDisposition)
    {
        std::string disposition(contentDisposition);
        size_t start = disposition.find("fileName=\"");
        if (start != std::string::npos)
        {
            start += 10;
            size_t end = disposition.find("\"", start);
            if (end != std::string::npos)
            {
                fileName = disposition.substr(start, end - start);
            }
        }
    }
    return fileName;
}

std::string FileHandler::getCurPath()
{
    return secCodeGenerator_->getCurPath();
}

std::string FileHandler::getSecCode()
{
    return secCodeGenerator_->getSecCode();
}

std::string FileHandler::getRequestBuffer(struct evhttp_request *req)
{
    auto input_buffer = evhttp_request_get_input_buffer(req);
    size_t buffer_length = evbuffer_get_length(input_buffer);
    std::vector<char> buffer(buffer_length);
    evbuffer_copyout(input_buffer, buffer.data(), buffer_length);
    std::string requestBody(buffer.begin(), buffer.end());
    return requestBody;
}

std::optional<Json::Value> FileHandler::parseJsonString(const std::string& jsonStr)
{
    Json::Value root;
    Json::CharReaderBuilder reader;
    std::string errs;
    std::istringstream s(jsonStr);

    if (!Json::parseFromStream(reader, s, &root, &errs)) {
        std::cerr << "Failed to parse JSON: " << errs << "\n";
        return {};
    }
    return root;
}

bool FileHandler::isSecCodeRight(const Json::Value& jsonObj)
{
    std::string security_code = jsonObj.get("secCode", "").asString();
    std::cout << "Security Code: " << security_code << "\n";
    const auto secCode = this->getSecCode();

    if (secCode.empty() or secCode != security_code) {
        std::cerr << "Invalid security code\n";
        return false;
    }
    return true;
}

void FileHandler::signal_cb(evutil_socket_t fd, short event, void *arg)
{
    #ifndef __WIN32
    printf("%s signal received\n", strsignal(fd));
    #endif
    event_base_loopbreak((event_base*)(arg));
}

std::string FileHandler::getBoundary(const char* contentType)
{
    std::string contentTypeStr(contentType);
    std::string::size_type pos = contentTypeStr.find("boundary=");
    if (pos != std::string::npos) {
        std::string boundary = contentTypeStr.substr(pos + 9);
        if (boundary.front() == '\"' && boundary.back() == '\"')
            boundary = boundary.substr(1, boundary.size() - 2);
        return boundary;
    }
    return "";
}

bool FileHandler::parseMultipart(evhttp_request *req, void* arg)
{
    struct evbuffer *inputBuffer = evhttp_request_get_input_buffer(req);
    size_t len = evbuffer_get_length(inputBuffer);
    char *data = static_cast<char *>(malloc(len));
    evbuffer_remove(inputBuffer, data, len);

    std::string body(data, len);  // Ensure binary-safe string creation
    free(data);

    std::cout << "-------------head--------------\n";
    std::cout << body.substr(0, 100) << "\n"; // Print only the first 100 characters for debugging
    std::cout << "-------------tail--------------\n";

    const char *contentType = evhttp_find_header(evhttp_request_get_input_headers(req), "Content-Type");
    if (!contentType)
    {
        evhttp_send_reply(req, HTTP_BADREQUEST, "Invalid Content-Type", nullptr);
        return false;
    }

    std::string boundary = getBoundary(contentType);
    if (boundary.empty())
    {
        evhttp_send_reply(req, HTTP_BADREQUEST, "Invalid boundary", nullptr);
        return false;
    }

    const std::string delimiter = "--" + boundary;
    const std::string endDelimiter = delimiter + "--";
    
    std::string fileName;
    FileHandler* fileHandler = static_cast<FileHandler*>(arg);

    size_t pos = 0, endPos = 0;
    while ((pos = body.find(delimiter, endPos)) != std::string::npos)
    {
        pos += delimiter.size();
        if (body.compare(pos, 2, "\r\n") == 0)
            pos += 2;

        endPos = body.find(delimiter, pos);
        if (endPos == std::string::npos)
            break;

        std::string part = body.substr(pos, endPos - pos);
        if (part.empty())
            continue;

        size_t headerEnd = part.find("\r\n\r\n");
        if (headerEnd == std::string::npos)
            continue;

        std::string headers = part.substr(0, headerEnd);
        std::string content = part.substr(headerEnd + 4, endPos - headerEnd - 4);

        if (headers.find("Content-Disposition: form-data; name=\"metadata\"") != std::string::npos)
        {
            std::cout << "Metadata: " << content << "\n";
            auto optJsonRoot = fileHandler->parseJsonString(content);
            if (!optJsonRoot)
            {
                evhttp_send_reply(req, HTTP_BADREQUEST, "Invalid JSON metadata", nullptr);
                return false;
            }
            else
            {
                auto jsonRoot = optJsonRoot.value();
                fileName = jsonRoot.get("fileName", "").asString();
                auto sessionId = jsonRoot.get("sessionId", "").asString();
                auto secCode = jsonRoot.get("secCode", "").asString();

                if (fileName.empty() or sessionId.empty()
                    or secCode.empty() or secCode != fileHandler->getSecCode())
                {
                    evhttp_send_reply(req, HTTP_BADREQUEST, "Invalid file info[3]", nullptr);
                    return false;
                }
            }
        } 
        else if (headers.find("Content-Disposition: form-data; name=\"file\"; fileName=") != std::string::npos)
        {
            std::cout << "File content size: " << content.size() << "\n";

            #ifdef __WIN32
            auto filePath = fileHandler->getCurPath() + "\\" + fileName;
            #else
            auto filePath = fileHandler->getCurPath() + "/" + fileName;
            #endif

            std::cout << "fileName: " << filePath << "\n";
            std::ofstream outFile(filePath, std::ios::binary | std::ios::app);
            outFile.write(content.c_str(), content.size());
            outFile.close();
        }
    }

    evhttp_send_reply(req, HTTP_OK, "File uploaded successfully", nullptr);
    return true;
}

void FileHandler::upload_handler(evhttp_request *req, void *arg)
{
    if (evhttp_request_get_command(req) != EVHTTP_REQ_POST)
    {
        evhttp_send_error(req, HTTP_BADREQUEST, "Invalid request method");
        return;
    }

    FileHandler* fileHandler = static_cast<FileHandler*>(arg);
    if (fileHandler->parseMultipart(req, arg))
    {
        std::cout << "File uploaded successfully" << std::endl;
        evhttp_send_reply(req, HTTP_OK, "File uploaded successfully", nullptr);
    }
}

void FileHandler::download_handler(struct evhttp_request *req, void *arg)
{
    std::cout << __FUNCTION__ << "\n";
    FileHandler* fileHandler = static_cast<FileHandler*>(arg);

    std::filesystem::path localPath(fileHandler->getCurPath());
    auto requestBody = fileHandler->getRequestBuffer(req);
    std::cout << "Request Body: " << requestBody << "\n";

    auto optJsonRoot = fileHandler->parseJsonString(requestBody);
    if (not optJsonRoot)
    {
        evhttp_send_reply(req, HTTP_BADREQUEST, "Invalid JSON", nullptr);
        return;
    }

    if (not fileHandler->isSecCodeRight(optJsonRoot.value()))
    {
        evhttp_send_reply(req, HTTP_FORBIDDEN, "Invalid Security Code", nullptr);
        return;
    }

    std::string fileName = optJsonRoot->get("fileName", "").asString();
    std::cout << "path: " << localPath << ",fileName: " << fileName << "\n";

    std::ifstream file(localPath / fileName, std::ios::binary);

    if (!file.is_open())
    {
        std::cerr << "file not found!\n";
        evhttp_send_reply(req, HTTP_NOTFOUND, "File not found", nullptr);
        return;
    }

    const size_t CHUNK_SIZE = 4096;
    std::vector<char> buffer(CHUNK_SIZE);

    struct evbuffer* evb = evbuffer_new();
    evhttp_send_reply_start(req, HTTP_OK, "OK");

    uint64_t totalSize = 0;
    uint64_t lastDownloadSize = 0;


    while (file.good())
    {
        if (totalSize > lastDownloadSize + 1024*1024*100)
        {
            // std::this_thread::sleep_for(200ms);
            QThread::msleep(400);
            lastDownloadSize = totalSize;
            std::cout << "current sended size: " << lastDownloadSize << "\n";
            std::cout << "buffer size: " << buffer.size() << "\n";
        }

        file.read(buffer.data(), CHUNK_SIZE);
        auto bytesRead = file.gcount();
        if (bytesRead > 0)
        {
            totalSize += bytesRead;
            evbuffer_add(evb, buffer.data(), bytesRead);
            evhttp_send_reply_chunk(req, evb);
            evbuffer_drain(evb, evbuffer_get_length(evb));
            // std::cout << "send chunk size: " << bytesRead << "\n";
        }
    }

    file.close();
    std::cout << fileName << " sended, total send size: " << totalSize << "\n";
    evhttp_send_reply_end(req);
    evbuffer_free(evb);
}

void FileHandler::getCacheDir_handler(struct evhttp_request *req, void *arg)
{
    std::cout << __FUNCTION__ << "\n";
    FileHandler* fileHandler = static_cast<FileHandler*>(arg);
    auto requestBody = fileHandler->getRequestBuffer(req);
    std::cout << "Request Body: " << requestBody << "\n";

    auto optJsonRoot = fileHandler->parseJsonString(requestBody);
    if (not optJsonRoot)
    {
        evhttp_send_reply(req, HTTP_BADREQUEST, "Invalid JSON", nullptr);
        return;
    }

    if (not fileHandler->isSecCodeRight(optJsonRoot.value()))
    {
        evhttp_send_reply(req, HTTP_FORBIDDEN, "Invalid Security Code", nullptr);
        return;
    }

    std::string directory_path = fileHandler->getCurPath();
    std::cout << "curPath:" << directory_path << "\n";
    if (!fs::exists(directory_path) || !fs::is_directory(directory_path))
    {
        std::cerr << "Directory not found\n";
        evhttp_send_reply(req, HTTP_NOTFOUND, "Directory not found", nullptr);
        return;
    }

    Json::Value response;
    response["dir"] = directory_path;
    Json::Value children(Json::arrayValue);

    for (const auto &entry : fs::directory_iterator(directory_path))
    {
        if (not entry.is_regular_file())
            continue;
        Json::Value child;
        auto fileAbsoluteName = entry.path().string();

        child["fileSize"] = 0;
        child["fileType"] = "directory";

        #ifdef __WIN32
        child["fileName"] = fileAbsoluteName.substr(fileAbsoluteName.find_last_of("\\") + 1);
        #else
        child["fileName"] = fileAbsoluteName.substr(fileAbsoluteName.find_last_of("/") + 1);
        #endif

        if (not entry.is_directory())
        {
            child["fileType"] = "file";
            child["fileSize"] = (std::uint64_t)entry.file_size();
        }
        children.append(child);
    }

    response["children"] = children;

    Json::StreamWriterBuilder writer;
    std::string response_body = Json::writeString(writer, response);

    struct evbuffer *response_buffer = evbuffer_new();
    evbuffer_add(response_buffer, response_body.c_str(), response_body.size());
    evhttp_send_reply(req, HTTP_OK, "OK", response_buffer);
    evbuffer_free(response_buffer);
}