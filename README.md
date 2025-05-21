# Simple http server

Based on [C-Web-Server](https://github.com/bloominstituteoftechnology/C-Web-Server/tree/master) from 
[Bloom Institute of Technology](https://github.com/bloominstituteoftechnology) and rewritten in C++.

## build this project

In folder `build/`

run 

```
cmake ..
make
```

and use 
```
./server
```
to run the server and use 
```
./tests
```
to run the tests.


## build chatroom

In folder `chatroom/`


Start and activate the Python virtual environment:

```
./setup_venv.sh
source .venv/bin/activate
```

Install dependencies for chat app:

in `chatroom/build/` folder

```
conan install .. --build=missing
```
run cmake:
```
cmake .. -DCMAKE_TOOLCHAIN_FILE=build/Release/generators/conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Release
```