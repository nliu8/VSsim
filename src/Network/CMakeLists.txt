#Sends the -std=c99 flag to the gcc compiler
add_definitions(-std=c99)
#This tells CMake to fib.c and name it fibonacci
add_executable(fibo fib.c)
