import ctypes
lib = ctypes.cdll.LoadLibrary('../build/liblsqecclib.so')

def create_libsqecclib_grid(layout):
    '''
        Call the appropriate api function from the shared library
    '''
    obj = lib.api_create_grid(layout.height, layout.width, layout.cells()) 
    return obj


def create_router():
    '''
        Call the appropriate api function from the shared library
    '''
    obj = lib.api_create_router() 
    return obj


def create_circuit(gates):  
    '''
    '''
    obj

