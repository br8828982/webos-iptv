TARGET = lightiptv
SRC_DIR = src
OBJ_DIR = build

# We use SDKTARGETSYSROOT to point strictly to the TV's library folders
CXXFLAGS += -O2 -Wall -std=c++17 -I$(SDKTARGETSYSROOT)/usr/include -I$(SDKTARGETSYSROOT)/usr/include/gstreamer-1.0 -I$(SDKTARGETSYSROOT)/usr/include/glib-2.0 -I$(SDKTARGETSYSROOT)/usr/lib/glib-2.0/include
LDFLAGS += -L$(SDKTARGETSYSROOT)/usr/lib -lSDL2 -lgstreamer-1.0 -lgobject-2.0 -lglib-2.0 -lcurl

SRCS = $(wildcard $(SRC_DIR)/*.cpp)
OBJS = $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRCS))

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) -o $@ $^ $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR) $(TARGET)
