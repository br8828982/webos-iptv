#include <SDL2/SDL.h>
#include <gst/gst.h>
#include <iostream>
#include "playlist.h"

GstElement *pipeline = nullptr;
std::vector<Channel> channels;
int currentIndex = 0;
bool isRunning = true;

// Injects Akamai Cookies and Headers into the video player
static void source_setup_cb(GstElement *playbin, GstElement *source, gpointer user_data) {
    Channel* currentChannel = static_cast<Channel*>(user_data);
    
    if (g_object_class_find_property(G_OBJECT_GET_CLASS(source), "extra-headers")) {
        GstStructure *extra_headers = gst_structure_new_empty("extra-headers");
        for (const auto& header : currentChannel->headers) {
            gst_structure_set(extra_headers, header.first.c_str(), G_TYPE_STRING, header.second.c_str(), NULL);
        }
        g_object_set(source, "extra-headers", extra_headers, NULL);
        gst_structure_free(extra_headers);
    }
}

void playCurrentChannel() {
    if (channels.empty()) return;

    if (pipeline) {
        gst_element_set_state(pipeline, GST_STATE_NULL);
        gst_object_unref(pipeline);
    }

    pipeline = gst_element_factory_make("playbin", "player");
    g_object_set(G_OBJECT(pipeline), "uri", channels[currentIndex].url.c_str(), NULL);

    g_signal_connect(pipeline, "source-setup", G_CALLBACK(source_setup_cb), &channels[currentIndex]);

    gst_element_set_state(pipeline, GST_STATE_PLAYING);
    std::cout << "Playing: " << channels[currentIndex].name << std::endl;
}

void handleInput(SDL_Event& event) {
    if (event.type == SDL_KEYDOWN) {
        switch (event.key.keysym.sym) {
            case SDLK_ESCAPE: // LG Back Button
            case SDLK_q:
                isRunning = false; break;
            case SDLK_UP: // LG Channel Up
                currentIndex = (currentIndex + 1) % channels.size();
                playCurrentChannel(); break;
            case SDLK_DOWN: // LG Channel Down
                currentIndex = (currentIndex - 1 + channels.size()) % channels.size();
                playCurrentChannel(); break;
        }
    }
}

int main(int argc, char* argv[]) {
    gst_init(&argc, &argv);
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window* window = SDL_CreateWindow("IPTV", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1920, 1080, SDL_WINDOW_FULLSCREEN);
    
    std::cout << "Fetching dynamic playlist..." << std::endl;
    std::string m3uData = fetchPlaylistFromUrl("https://kodi-1fx.pages.dev/kodi_playlist.m3u");
    channels = parseKodiPlaylistFromString(m3uData);

    if(channels.empty()) {
        std::cout << "Failed to load playlist. Exiting." << std::endl;
        isRunning = false;
    } else {
        playCurrentChannel();
    }

    SDL_Event event;
    while (isRunning) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) isRunning = false;
            else handleInput(event);
        }
        SDL_Delay(16); // Prevent CPU maxing
    }

    if (pipeline) {
        gst_element_set_state(pipeline, GST_STATE_NULL);
        gst_object_unref(pipeline);
    }
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
