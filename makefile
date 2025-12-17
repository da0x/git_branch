# Generated with github.com/da0x/mmpp
# Binary:
git-select: .obj .obj/main.cpp.o
	g++ -std=c++20 .obj/main.cpp.o -o git-select

.obj:
	mkdir .obj

.obj/main.cpp.o: main.cpp
	g++ -std=c++20 -o .obj/main.cpp.o -c main.cpp

run: git-select
	./git-select

clean:
	rm -rfv .obj

install: git-select
	sudo cp -v git-select /usr/local/bin/

uninstall:
	sudo rm -v /usr/local/bin/git-select
