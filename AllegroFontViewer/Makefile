ALLEGRO_SUFFIX = -debug
ALLEGRO_MODS = allegro$(ALLEGRO_SUFFIX)-5 allegro_main$(ALLEGRO_SUFFIX)-5 \
               allegro_dialog$(ALLEGRO_SUFFIX)-5 \
               allegro_acodec$(ALLEGRO_SUFFIX)-5 \
               allegro_color$(ALLEGRO_SUFFIX)-5 \
               allegro_font$(ALLEGRO_SUFFIX)-5 \
               allegro_image$(ALLEGRO_SUFFIX)-5 \
               allegro_primitives$(ALLEGRO_SUFFIX)-5 \
               allegro_ttf$(ALLEGRO_SUFFIX)-5
ALLEGRO_LIBS = $(shell pkg-config --libs $(ALLEGRO_MODS))
ALLEGRO_CFLAGS = $(shell pkg-config --cflags $(ALLEGRO_MODS))

ICU_MODS = icu-io icu-i18n icu-uc
ICU_CFLAGS = $(shell pkg-config --cflags $(ICU_MODS))
ICU_LIBS = $(shell pkg-config --libs $(ICU_MODS))

CFLAGS ?= -Wall -Wextra -std=c99 $(ALLEGRO_CFLAGS) $(ICU_CFLAGS)
LDFLAGS ?=
LIBS ?= $(ALLEGRO_LIBS) $(ICU_LIBS)

BUILD_DIR = build
TARGET = $(BUILD_DIR)/afv
SOURCES = $(wildcard *.c)
OBJECTS = $(patsubst %.c,%.o,$(SOURCES))

all: $(TARGET)

devel: CFLAGS += -g
devel: all

$(TARGET): $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $(OBJECTS) $(LIBS)

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<

clean:
	$(RM) $(TARGET) $(OBJECTS)

.PHONY: all clean
