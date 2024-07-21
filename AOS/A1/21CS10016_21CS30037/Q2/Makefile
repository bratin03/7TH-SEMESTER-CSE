obj-m += 21CS10016_21CS30037.o

all:
	sudo make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) modules

install:
	sudo insmod 21CS10016_21CS30037.ko

uninstall:
	sudo rmmod 21CS10016_21CS30037

tests: test_1 test_2 test_3 test_4 test_5 test_6 test_7 test_8 test_9

test_1: ./TESTS/test_1.cpp
	g++ ./TESTS/test_1.cpp -o ./TESTS/test_1.out
	cd TESTS && ./test_1.sh 1000

test_2: ./TESTS/test_2.cpp
	g++ ./TESTS/test_2.cpp -o ./TESTS/test_2.out
	cd TESTS && ./test_2.out

test_3: ./TESTS/test_3.cpp
	g++ ./TESTS/test_3.cpp -o ./TESTS/test_3.out
	cd TESTS && ./test_3.out 1000

test_4: ./TESTS/test_4.cpp
	g++ ./TESTS/test_4.cpp -o ./TESTS/test_4.out
	cd TESTS && ./test_4.out

test_5: ./TESTS/test_5.cpp
	g++ ./TESTS/test_5.cpp -o ./TESTS/test_5.out
	cd TESTS && ./test_5.out

test_6: ./TESTS/test_6.cpp
	g++ ./TESTS/test_6.cpp -o ./TESTS/test_6.out
	cd TESTS && ./test_6.out

test_7: ./TESTS/test_7.cpp
	g++ ./TESTS/test_7.cpp -o ./TESTS/test_7.out
	cd TESTS && ./test_7.out

test_8: ./TESTS/test_8.cpp
	g++ ./TESTS/test_8.cpp -o ./TESTS/test_8.out
	cd TESTS && ./test_8.out

test_9: ./TESTS/test_9.cpp
	g++ ./TESTS/test_9.cpp -o ./TESTS/test_9.out
	cd TESTS && ./test_9.out

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) clean
	rm -f ./TESTS/test_*.out