.PHONY: all clean

INCLUDES = -I/usr/include -I/home/tvv/local/include
LIBDIR = -L/home/tvv/local/lib -L/usr/lib
suffix = -debug
LIBS = -lallegro${suffix} -lallegro_main${suffix} -lallegro_dialog${suffix} -lallegro_audio${suffix} -lallegro_acodec${suffix} -lallegro_color${suffix} -lallegro_font${suffix} -lallegro_image${suffix} -lallegro_primitives${suffix} -lallegro_ttf${suffix} -licuio -licui18n -licuuc -licudata

#-lallegro_audio${suffix}
#-lallegro_memfile${suffix}
#-lallegro_physfs${suffix}
#-lallegro_video${suffix}

CFLAGS = -std=c99 -Wall -Wextra $(INCLUDES)
TARGET = afv
SOURCES = $(wildcard *.c)
OBJECTS = $(patsubst %.c,%.o,$(SOURCES))

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(LIBDIR) -o $@ $(OBJECTS) $(LIBS)

clean:
	$(RM) $(TARGET)
	$(RM) $(OBJECTS)

