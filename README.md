# Broker Server (MULTI / SELECT / POLL / EPOLL)
### Build
Always clean before building:
```
make clean
```
**Server (choose mode)**
```
make server MODE=MULTI
make server MODE=SELECT
make server MODE=POLL
make server MODE=EPOLL
```
**Client**
```
make client
```
### Run

**Start server:**
```
./server
```
**Start client:**
```
./client
```
