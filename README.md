complile main: g++ -Iinclude -pthread  main.cpp -o main
compile cgi: g++ -g -Iinclude -L/usr/lib/x86_64-linux-gnu -pthread cgi/cgi.cpp 
             -o cgi/cgi.cgi -lcgicc 
run: ./main

commands:
search <criteria> : Fetch first page of results from Giphy according to given 
                    criteria
next : Fetch next page of results
rank: Show number of results with same rank
cancel : Halt current search
exit: Close session
