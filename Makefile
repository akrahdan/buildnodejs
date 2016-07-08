CC ?= $(which clang)

BUILDTYPE ?= Release
BUILDFILE ?= main.c
OUTFILE ?= thread

UV_DIR ?= uv
UV_BUILD = $(UV_DIR)/out/$(BUILDTYPE)
UV_FLAGS = -fno-omit-frame-pointer

IDIR = $(UV_DIR)/include
CFLAGS = -pthread -fno-omit-frame-pointer -Wall -g

all:
	$(CC) $(CFLAGS) -o $(OUTFILE) $(BUILDFILE) $(UV_BUILD)/libuv.a -I$(IDIR)

.PHONY: libuv.a clean

libuv.a:
	@if [ ! -d $(UV_DIR)/build/gyp ]; then \
		git clone https://chromium.googlesource.com/external/gyp.git $(UV_DIR)/build/gyp; \
	fi
	@cd $(UV_DIR); \
		./gyp_uv.py -f make > /dev/null; \
		BUILDTYPE=$(BUILDTYPE) CFLAGS=$(UV_FLAGS) make -C out -j4

clean:
	@for i in `ls`; do \
		if [ -x $$i ] && [ ! -d $$i ]; then \
			rm $$i; \
		fi; \
	done
	@rm -rf $(UV_DIR)/out
