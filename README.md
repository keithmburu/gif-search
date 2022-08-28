# GIF Search

An app for searching for gifs and stickers, built using C++, Asio library for networking, and Giphy's REST API.

## Setup 
### For interactive console:

complile: g++ -Iinclude -pthread  main.cpp -o main\
run: ./main

### For server:

compile: g++ -g -Iinclude -L/usr/lib/x86_64-linux-gnu -pthread cgi/cgi.cpp 
             -o cgi/html/gifs.cgi -lcgicc\
sudo apt install apache2\
cp -rf cgi/apache2/ /etc/\
sudo cp -f cgi/html/* /var/www/html\
sudo service apache2 start\
visit ip address at ip addr | grep eth0


## Commands

search <criteria> : Fetch first page of results from Giphy according to given\ 
                    criteria
next : Fetch next page of results\
rank: Show number of results with same rank\
cancel : Halt current search\
exit: Close session
