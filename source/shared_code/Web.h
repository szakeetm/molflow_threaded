#include <curl/curl.h>
#include <string>
#include <tuple>

size_t AppendDataToStringCurlCallback(void *ptr, size_t size, size_t nmemb, void *vstring);

std::tuple<CURLcode,std::string> DownloadString(std::string url);
CURLcode SendHTTPPostRequest(std::string hostname, std::string payload);
CURLcode DownloadFile(std::string url,std::string fileName);