from ctypes import c_int

EnumDefaultType = c_int

# Enum constructor 
def Enum(vals, c_type=EnumDefaultType): 
    '''
    Maps an interable to a dictionary
    '''
    return {
        symbol:c_type(i) 
        for i, symbol in enumerate(vals)
    }

