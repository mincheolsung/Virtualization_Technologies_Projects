# VT_Project3

## build HermitCore
1. On the repo ({YOUR-PATH}/VT_Project3/)
2. `git submodule init`
3. `git submodule update`
4. `cd VT_Project3_Hermitcore`
5. `git submodule init`
6. `git submodule update`
7. `mkdir build && cd build`
8. `cmake ..`
9. `make && sudo make install`

## install sysv_ipc for IPC
1. `cd VT_Project3/sysv_ipc-1.0.0`
2. `sudo python setup.py install`

## run example application
1. `cd VT_Project3/VT_project3_Hermitcore/usr/tests`
2. `sudo make test`


# Design
### int put(char \*key, void \*value, size_t value_len)

Behavior:
- Put() copies key to **buffer_key** which is **contiguous buffer** allocated by init_buffer() in kernel/main.c
- Put() copies value to **buffer_value** which is **contiguous buffer**.
- Then, uhyve_send() sends addresses of buffer_key and buffer_value to uhyve.
- Uhyve creates shared memory and copy data in buffer_key and buffer_value into the shared memory.
- Uhyve calls proxy.py and proxy.py upload data in the shared memory to AWS S3.

### int get(char \*key, void \*value, size_t \*value_len)

Behavior:
- get() passes addresses of buffer_key and buffer_value to uhyve (uhyve_send()).
- Uhyve creates shared memory and calls proxy.py.
- Proxy.ph download data with the key and stores on the shared buffer.
- Uhyve copy data(value) from the shared memory and stores in buffer_value.
- get() copies data in buffer_value to value which is user buffer.
