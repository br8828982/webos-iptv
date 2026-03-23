#ifndef PLAYLIST_H
#define PLAYLIST_H

#include <string>
#include <vector>
#include <map>

struct Channel {
    std::string name;
    std::string url;
    std::map<std::string, std::string> headers;
};

std::string fetchPlaylistFromUrl(const std::string& url);
std::vector<Channel> parseKodiPlaylistFromString(const std::string& m3uContent);

#endif
