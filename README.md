# Broker Server (MULTI / SELECT / POLL / EPOLL)
### Build
Always clean before building:
```
make clean
```
**Server (choose mode)**
```
make server MODE=MULTI     // mode server multithread
make server MODE=SELECT    // mode select
make server MODE=POLL      // mode poll
make server MODE=EPOLL      // mode epoll
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
