all:
	gcc ../main.c ../glad_gl.c -I ../inc -Ofast -lglfw -lm -o aigeneratedgame

install:
	cp aigeneratedgame $(DESTDIR)

uninstall:
	rm $(DESTDIR)/aigeneratedgame

clean:
	rm aigeneratedgame