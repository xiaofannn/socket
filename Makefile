all: select_server client

select_server:
	g++ -o select_server select.cpp
poll_server:
	g++ -o poll_server poll.cpp
client:
	g++ -o client  client.cpp
clean:
	rm ./select_server ./client ./pool_server
