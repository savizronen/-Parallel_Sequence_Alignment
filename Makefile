build:
	mpicxx -fopenmp -c main.c -o main.o
	mpicxx -fopenmp -c fileUtil.c -o fileUtil.o
	mpicxx -fopenmp -c functions.c -o functions.o 
	mpicxx -fopenmp -c mpiUtill.c -o mpiUtill.o 
	nvcc -I./inc -c cudaFunctions.cu -o cudaFunctions.o
	mpicxx -fopenmp -o exec main.o functions.o fileUtil.o mpiUtill.o cudaFunctions.o  /usr/local/cuda-11.8/lib64/libcudart_static.a -ldl -lrt


run:
	mpiexec -np 4 ./exec < input.txt

clean:
	rm -f *.o ./exec

