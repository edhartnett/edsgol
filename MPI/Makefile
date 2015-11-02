# $Id: Makefile,v 1.12 2008/11/02 02:30:27 edhartnett Exp $

# Remember, for Linux set CC=mpicc and CFLAGS='-g -Wall', for frost use CC=mpxlC
CC=mpicc
CFLAGS=-g -Wall
all: goll gol

gol: gol.c
	${CC} ${CFLAGS} ${MPIFLAGS} -o gol gol.c -lm

goll: gol.c
	${CC} ${CFLAGS} ${MPIFLAGS} -DLOGGING -o goll gol.c -lpthread -llmpe -lmpe ${MPILIBS} -lm -lrt 

clean:
	-rm *.o gol goll test_file_type test_mpe

test_file_type: test_file_type.c
	${CC} ${CFLAGS} ${MPIFLAGS} -o test_file_type test_file_type.c ${MPILIBS} -lm

test_mpe: test_mpe.c
	${CC} ${CFLAGS} ${MPIFLAGS} -o test_mpe test_mpe.c -lpthread -llmpe -lmpe ${MPILIBS} 

check: all
	./gol -c 1 -i input/life.pgm -t 10 -s 900 -n 1 -k -f -o > output/test3_1.out
	head -n 11 output/test3_1.out > output/test_1.out
	cmp output/test_1.out output/ref_test.out
	@echo "*** SUCCESS with checkerboard decomposition, MPI types, with output file test!"

	./gol -c 1 -i input/life.pgm -t 10 -s 900 -n 1 -f -o > output/test3_1.out
	head -n 11 output/test3_1.out > output/test_1.out
	cmp output/test_1.out output/ref_test.out
	@echo "*** SUCCESS with row decomposition, MPI types, with output file test!"

	valgrind -q ./gol -c 1 -i input/life.pgm -t 10 -s 900 -n 1 > output/test3_1.out
	head -n 11 output/test3_1.out > output/test_1.out
	cmp output/test_1.out output/ref_test.out
	@echo "*** SUCCESS with valgrind!"

	valgrind -q ./gol -c 1 -i input/life.pgm -t 10 -s 900 -n 1 -k > output/test3_1.out
	head -n 11 output/test3_1.out > output/test_1.out
	cmp output/test_1.out output/ref_test.out
	@echo "*** SUCCESS with valgrind again!"

	./gol -c 1 -i input/life.pgm -t 10 -s 900 -n 1 -k -o > output/test3_1.out
	head -n 11 output/test3_1.out > output/test_1.out
	cmp output/test_1.out output/ref_test.out
	@echo "*** SUCCESS with output file test!"

	-mpiexec -n 1 ./gol -c 1 -k -i input/life.pgm -t 10 -s 900 > output/test2_k.out
	head -n 11 output/test2_k.out > output/test_k.out
	cmp output/test_k.out output/ref_test.out
	@echo "*** SUCCESS with serial checkerboard decomposition!"

	-mpiexec -n 1 ./gol -c 1 -fk -n 1 -i input/life.pgm -t 10 -s 900 > output/test2_k4.out
	head -n 11 output/test2_k4.out > output/test_k4.out
	cmp output/test_k4.out output/ref_test.out
	@echo "*** SUCCESS with n=1 checkerboard decomposition with MPI types!"

	-mpiexec -n 4 ./gol -c 1 -k -n 4 -i input/life.pgm -t 10 -s 900 > output/test2_k4.out
	head -n 11 output/test2_k4.out > output/test_k4.out
	cmp output/test_k4.out output/ref_test.out
	@echo "*** SUCCESS with n=4 checkerboard decomposition!"

	-mpiexec -n 4 ./gol -c 1 -fk -n 4 -i input/life.pgm -t 10 -s 900 > output/test2_k4.out
	head -n 11 output/test2_k4.out > output/test_k4.out
	cmp output/test_k4.out output/ref_test.out
	@echo "*** SUCCESS with n=4 checkerboard decomposition with MPI types!"

	-mpiexec -n 4 ./gol -c 1 -fok -n 4 -i input/life.pgm -t 10 -s 900 > output/test2_o_k4.out
	head -n 11 output/test2_o_k4.out > output/test_o_k4.out
	cmp output/test_o_k4.out output/ref_test.out
	@echo "*** SUCCESS with n=4 checkerboard decomposition with MPI types and output!"

	-mpiexec -n 25 ./gol -c 1 -k -n 25 -i input/life.pgm -t 10 -s 900 > output/test2_k25.out
	head -n 11 output/test2_k25.out > output/test_k25.out
	cmp output/test_k25.out output/ref_test.out
	@echo "*** SUCCESS with n=25 checkerboard decomposition!"

	-mpiexec -n 36 ./gol -c 1 -k -n 36 -i input/life.pgm -t 10 -s 900 > output/test2_k36.out
	head -n 11 output/test2_k36.out > output/test_k36.out
	cmp output/test_k36.out output/ref_test.out
	@echo "*** SUCCESS with n=36 checkerboard decomposition!"

	-mpiexec -n 1 ./gol -c 1 -i input/life.pgm -t 10 -s 900 > output/test2.out
	head -n 11 output/test2.out > output/test.out
	cmp output/test.out output/ref_test.out
	@echo "*** SUCCESS with serial row decomposition!"

	-mpiexec -n 2 ./gol -c 1 -i input/life.pgm -t 10 -s 900 -n 2 > output/test2_2.out
	head -n 11 output/test2_2.out > output/test_2.out
	cmp output/test_2.out output/ref_test.out
	@echo "*** SUCCESS with n=2 row decomposition!"

	-mpiexec -n 3 ./gol -c 1 -i input/life.pgm -t 10 -s 900 -n 3 > output/test2_3.out
	head -n 11 output/test2_3.out > output/test_3.out
	cmp output/test_3.out output/ref_test.out
	@echo "*** SUCCESS with n=3 row decomposition!"

	-mpiexec -n 4 ./gol -c 1 -i input/life.pgm -t 10 -s 900 -n 4 > output/test2_4.out
	head -n 11 output/test2_4.out > output/test_4.out
	cmp output/test_4.out output/ref_test.out
	@echo "*** SUCCESS with n=4 row decomposition!"

	-mpiexec -n 9 ./gol -c 1 -i input/life.pgm -t 10 -s 900 -n 9 > output/test2_9.out
	head -n 11 output/test2_9.out > output/test_9.out
	cmp output/test_9.out output/ref_test.out
	@echo "*** SUCCESS with n=9 row decomposition!"

check_all: all
	mpiexec -n 1 ./gol -c 1 -i input/life.pgm -t 1000 -s 900 > output/test_1000.out
	cmp output/test_1000.out output/ref_test_1000.out
	@echo "*** SUCCESS with serial row decomposition!"
	mpiexec -n 9 ./gol -c 1 -i input/life.pgm -t 1000 -s 900 -n 9 > output/test9_1000.out
	cmp output/test9_1000.out output/ref_test_1000.out
	@echo "*** SUCCESS with n=9 row decomposition!"
	mpiexec -n 1 ./gol -k -c 1 -i input/life.pgm -t 1000 -s 900 > output/test_k_1000.out
	cmp output/test_k_1000.out output/ref_test_1000.out
	@echo "*** SUCCESS with serial checkerboard decompostion!"
	mpiexec -n 4 ./gol -c 1 -k -i input/life.pgm -t 1000 -s 900 -n 4 > output/test4_1000.out
	cmp output/test4_1000.out output/ref_test_1000.out
	@echo "*** SUCCESS with n=4 checkboard decomposition!"
	mpiexec -n 9 ./gol -c 1 -k -i input/life.pgm -t 1000 -s 900 -n 9 > output/test9_1000.out
	cmp output/test9_1000.out output/ref_test_1000.out
	@echo "*** SUCCESS with n=9 checkboard decomposition!"
	mpiexec -n 25 ./gol -c 1 -k -i input/life.pgm -t 1000 -s 900 -n 25 > output/test25_1000.out
	cmp output/test25_1000.out output/ref_test_1000.out
	@echo "*** SUCCESS with n=25 checkboard decomposition!"
	mpiexec -n 36 ./gol -c 1 -k -i input/life.pgm -t 1000 -s 900 -n 36 > output/test36_1000.out
	cmp output/test36_1000.out output/ref_test_1000.out
	@echo "*** SUCCESS with n=36 checkboard decomposition!"

homework:
	mpiexec -n 1 ./gol -c 1000 -i input/life.pgm -t 10000 -s 900 > output/hw_1000.out
	cmp output/hw_1000.out output/ref_hw_1000.out
	@echo "*** SUCCESS with serial row decomposition!"
	mpiexec -n 1 ./gol -k -c 1000 -i input/life.pgm -t 10000 -s 900 > output/hw_k_1000.out
	cmp output/hw_k_1000.out output/ref_hw_1000.out
	@echo "*** SUCCESS with serial checkerboard decompostion!"
	mpiexec -n 4 ./gol -c 1000 -k -i input/life.pgm -t 10000 -s 900 -n 4 > output/hw_4_1000.out
	cmp output/hw_4_1000.out output/ref_hw_1000.out
	@echo "*** SUCCESS with n=4 checkboard decomposition!"
	mpiexec -n 9 ./gol -c 1000 -k -i input/life.pgm -t 10000 -s 900 -n 9 > output/hw_9_1000.out
	cmp output/hw_9_1000.out output/ref_hw_1000.out
	@echo "*** SUCCESS with n=9 checkboard decomposition!"
	mpiexec -n 25 ./gol -c 1000 -k -i input/life.pgm -t 10000 -s 900 -n 25 > output/hw_25_1000.out
	cmp output/hw_25_1000.out output/ref_hw_1000.out
	@echo "*** SUCCESS with n=25 checkboard decomposition!"
	mpiexec -n 36 ./gol -c 1000 -k -i input/life.pgm -t 10000 -s 900 -n 36 > output/hw_36_1000.out
	cmp output/hw_36_1000.out output/ref_hw_1000.out
	@echo "*** SUCCESS with n=36 checkboard decomposition!"
	mpiexec -n 4 ./gol -c 1000 -i input/life.pgm -t 10000 -s 900 -n 4 > output/hw_4_1000.out
	cmp output/hw_4_1000.out output/ref_hw_1000.out
	@echo "*** SUCCESS with n=4 row decomposition!"
	mpiexec -n 9 ./gol -c 1000 -i input/life.pgm -t 10000 -s 900 -n 9 > output/hw_9_1000.out
	cmp output/hw_9_1000.out output/ref_hw_1000.out
	@echo "*** SUCCESS with n=9 row decomposition!"
	mpiexec -n 16 ./gol -c 1000 -i input/life.pgm -t 10000 -s 900 -n 16 > output/hw_16_1000.out
	cmp output/hw_16_1000.out output/ref_hw_1000.out
	@echo "*** SUCCESS with n=16 row decomposition!"
	mpiexec -n 36 ./gol -c 1000 -i input/life.pgm -t 10000 -s 900 -n 36 > output/hw_36_1000.out
	cmp output/hw_36_1000.out output/ref_hw_1000.out
	@echo "*** SUCCESS with n=36 row decomposition!"

benchmark: all
	-mpiexec -n 4 ./gol -k -n 4 -t 100 -s 900 -ph > output/bm.out
	-mpiexec -n 4 ./gol -k -n 4 -t 100 -s 16 -p >> output/bm.out
	-mpiexec -n 9 ./gol -k -n 9 -t 100 -s 900 -p >> output/bm.out
	-mpiexec -n 9 ./gol -k -n 9 -t 100 -s 81 -p >> output/bm.out
	-mpiexec -n 25 ./gol -k -n 25 -t 100 -s 900 -p >> output/bm.out
	-mpiexec -n 25 ./gol -k -n 25 -t 100 -s 100 -p >> output/bm.out
	-mpiexec -n 36 ./gol -k -n 36 -t 100 -s 900 -p >> output/bm.out
	-mpiexec -n 4 ./gol -n 4 -t 100 -s 900 -p >> output/bm.out
	-mpiexec -n 4 ./gol -n 4 -t 100 -s 16 -p >> output/bm.out
	-mpiexec -n 36 ./gol -n 36 -t 100 -s 900 -p >> output/bm.out
	-mpiexec -n 512 ./gol -n 512 -t 100 -s 1024 -p >> output/bm.out

