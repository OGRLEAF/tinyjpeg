import ctypes as C

dll = C.CDLL("./build/lib.so")
dll.hello()