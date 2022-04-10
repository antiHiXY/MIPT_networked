# Заходим в папку bin
# Для запуска сервера
```
g++ -std=c++17 ../server.cpp ../socket_tools.cpp -o server && ./server
```
# Для запуска клиента
```
g++ -std=c++17 ../client.cpp ../socket_tools.cpp -o client && ./client
```