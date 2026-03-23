#include "playlist.h"
#include <sstream>
#include <algorithm>
#include <iostream>
#include <curl/curl.h>

static inline void trim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) { return !std::isspace(ch); }));
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(), s.end());
}

static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

std::string fetchPlaylistFromUrl(const std::string& url) {
    CURL *curl;
    CURLcode res;
    std::string readBuffer;

    curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L); // Bypasses SSL issues on older webOS
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

        res = curl_easy_perform(curl);
        if(res != CURLE_OK) {
            std::cerr << "curl fetch failed: " << curl_easy_strerror(res) << std::endl;
        }
        curl_easy_cleanup(curl);
    }
    return readBuffer;
}

std::vector<Channel> parseKodiPlaylistFromString(const std::string& m3uContent) {
    std::vector<Channel> channels;
    std::stringstream file(m3uContent);
    std::string line;
    Channel currentChannel;

    while (std::getline(file, line)) {
        trim(line);
        if (line.empty()) continue;

        if (line.rfind("#EXTINF:", 0) == 0) {
            currentChannel = Channel();
            size_t commaPos = line.find_last_of(',');
            if (commaPos != std::string::npos) currentChannel.name = line.substr(commaPos + 1);
        } 
        else if (line.rfind("#KODIPROP:inputstream.adaptive.common_headers=", 0) == 0) {
            std::string headerString = line.substr(46); 
            std::stringstream ss(headerString);
            std::string pair;
            while (std::getline(ss, pair, '&')) {
                size_t equalsPos = pair.find('=');
                if (equalsPos != std::string::npos) {
                    currentChannel.headers[pair.substr(0, equalsPos)] = pair.substr(equalsPos + 1);
                }
            }
        }
        else if (line[0] != '#') {
            currentChannel.url = line;
            channels.push_back(currentChannel);
        }
    }
    return channels;
}
