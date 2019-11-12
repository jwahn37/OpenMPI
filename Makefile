
all :
	mpicc hw_prefixsum_1.c -o hw_prefixsum_1
	mpicc hw_prefixsum_2.c -o hw_prefixsum_2
	mpicc hw_prefixsum_3.c -o hw_prefixsum_3
	gcc hw_prefixsum_analyze.c -o hw_prefixsum_analyze
	gcc hw_imgprocess_1.c -o hw_imgprocess_1
	mpicc hw_imgprocess_2.c -o hw_imgprocess_2
clean :
	rm hw_prefixsum_1
	rm hw_prefixsum_2
	rm hw_prefixsum_3
	rm hw_prefixsum_analyze
	rm hw_imgprocess_1
	rm hw_imgprocess_2