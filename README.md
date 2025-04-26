This project only Works on Linux.

To run on windows,
1) Use WSL.
   (run by gcc -o flights view_flights.c -lpthread)
OR
2) Download and install the pthreads-w32 library (POSIX threads for Windows)
   Github repo: https://github.com/GerHobbelt/pthread-win32
   (run by gcc view_flights.c -o flights -lpthreadGC2)
   
3) To run the project use
   "make run"
   
4) To run the simulation use
   "make simul"



   
