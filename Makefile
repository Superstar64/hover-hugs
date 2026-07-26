
all: \
  dist/bin/hugs\
  dist/bin/runhugs\
  dist/bin/ffihugs\
  dist/lib/hugs/packages/hugsbase/hugsbase.cabal\
  dist/lib/hugs/include/HsFFI.h

dist/bin/:
	mkdir -p dist/bin

SRC := $(shell find src \( -name "*.c" -o -name "*.h" -o -name "*.y" \))

dist/bin/%: $(SRC) | dist/bin/
	make -C src ${@:dist/bin/%=%}
	cp src/${@:dist/bin/%=%} $@

dist/lib/hugs/packages/:
	rm -rf dist/lib/hugs/packages
	mkdir -p dist/lib/hugs/packages

HUGSBASE := $(shell find libraries/hugsbase -type f)

dist/lib/hugs/packages/hugsbase/hugsbase.cabal: $(HUGSBASE) | dist/lib/hugs/packages/
	rm -rf dist/lib/hugs/packages/hugsbase
	cp -R libraries/hugsbase dist/lib/hugs/packages
	touch $@

dist/lib/hugs/include/:
	rm -rf dist/lib/hugs/include
	mkdir -p dist/lib/hugs/include

dist/lib/hugs/include/HsFFI.h: src/HsFFI.h | dist/lib/hugs/include/
	cp $< $@