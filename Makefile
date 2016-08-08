all: select_server client
	
select_server:
	g++ -o select_server select.cpp
client:
	g++ -o client  client.cpp
clean:
	rm ./select_server ./client
