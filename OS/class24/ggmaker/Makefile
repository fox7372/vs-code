TITLE := $(shell python3 -c "import json; print(json.load(open('script.json'))['title'])")

game: script.json ggmaker.py
	python3 ggmaker.py script.json

run:
	./start.sh "$(TITLE)"

clean:
	rm -rf "$(TITLE)" ggfs

# FUSE-based visual novel filesystem
FUSE_CFLAGS  ?= $(shell pkg-config --cflags fuse 2>/dev/null)
FUSE_LDFLAGS ?= $(shell pkg-config --libs fuse 2>/dev/null || echo -lfuse)

ggfs: ggfs.c cJSON.h cJSON.c
	$(CC) -D_FILE_OFFSET_BITS=64 -Wall -O2 $(FUSE_CFLAGS) -o $@ ggfs.c cJSON.c $(FUSE_LDFLAGS)

fuse: ggfs
	mkdir -p /tmp/ggfs
	sudo ./ggfs script.json /tmp/ggfs

