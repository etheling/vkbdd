all : vkbdd vkeyboard

vkbdd: src/vkbdd.c
	gcc -w src/vkbdd.c -o ./vkbdd

vkeyboard: src/vkeyboard.c
	gcc -w src/vkeyboard.c -o ./vkeyboard -lncurses

install:
	sudo cp -f ./vkbdd /usr/local/sbin
	sudo cp -f ./vkeyboard /usr/local/bin
	-sudo systemctl stop vkbdd
	-sudo systemctl disable vkbdd
	sudo cp -f systemd/vkbdd.service /lib/systemd/system
	sudo systemctl enable vkbdd
	sudo systemctl start vkbdd

## enable all warnings
vkbdd-test: src/vkbdd.c
	gcc -Wall -Wextra src/vkbdd.c -o ./a.out

vkeyboard-test: src/vkeyboard.c
	gcc -Wall -Wextra src/vkeyboard.c -o ./b.out

clean :
	rm -f vkbdd vkeyboard a.out b.out src/*.o

