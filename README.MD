COMPILE CLIENT
>>g++ client.cpp -o client -std=c++17 -lws2_32

COMPILE SERVER
g++ server.cpp -o server -std=c++17 -lws2_32

Step 1: Start the Server
.\server

Step 2: Start a Client
.\client