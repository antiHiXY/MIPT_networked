# Для запуска сервера
```
g++ -std=c++17 ./server.cpp ./utilities.cpp -o server && ./server "port number"
```
# Для запуска клиента
```
g++ -std=c++17 ./client.cpp ./utilities.cpp -o client && ./client
```
# Для запуска матчмэйкинг сервера
```
g++ -std=c++17 ./matchmaking_server.cpp ./utilities.cpp -o matchmaking_server && ./matchmaking_server
```
# Выбираеам комнату матчмейкером
```
/start "number of room"
```
# Выбираем комнату клиентом
```
/select "number of room"
```

