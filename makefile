all:
	cc assets/ptf.c -lm -Ofast -o assets/ptf
	mkdir -p release
	cc main.c glad_gl.c -I inc -Ofast -lglfw -lm -o release/aigeneratedgame
	strip --strip-unneeded release/aigeneratedgame
	upx --lzma --best release/aigeneratedgame

med:
	cc assets/ptf.c -lm -Ofast -o assets/ptf
	mkdir -p release
	cc -DQUALITY_MEDIUM main.c glad_gl.c -I inc -Ofast -lglfw -lm -o release/aigeneratedgame_medium
	strip --strip-unneeded release/aigeneratedgame_medium
	upx --lzma --best release/aigeneratedgame_medium

low:
	cc assets/ptf.c -lm -Ofast -o assets/ptf
	mkdir -p release
	cc -DQUALITY_LOW main.c glad_gl.c -I inc -Ofast -lglfw -lm -o release/aigeneratedgame_low
	strip --strip-unneeded release/aigeneratedgame_low
	upx --lzma --best release/aigeneratedgame_low

test:
	cc main.c glad_gl.c -I inc -Ofast -lglfw -lm -o /tmp/aigeneratedgame_test
	/tmp/aigeneratedgame_test
	rm /tmp/aigeneratedgame_test

web:
	emcc main.c glad_gl.c -DWEB -O3 --closure 1 -s FILESYSTEM=0 -s USE_GLFW=3 -s ENVIRONMENT=web -s TOTAL_MEMORY=128MB -I inc -o bin/index.html --shell-file t.html

run:
	emrun bin/index.html

clean:
	rm -f -r release
	rm -f assets/ptf
	rm -f bin/index.html
	rm -f bin/index.js
	rm -f bin/index.wasm
