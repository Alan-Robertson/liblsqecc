class __Symbol:
    '''
        This is just a wrapper type to avoid ugly strings
    ''' 
    def __init__(self, val : str):
        self.val = val
        self.__id = hash(self.val)
    def __hash__(self):
       return self.__id 
    def __eq__(self, other): 
       return hash(self) == hash(other) 
    def __is__(self, other):
       return self.__eq__(other)
    def __repr__(self):
        return self.val
    def __str__(self):
        return self.__repr__() 
    def __bytes__(self):
        return self.val.encode('ascii')

# Dodgy global dict tracking current singletons
__registered = dict() 

def __SINGLETON(val):
    '''
        Tracks objects and ensures that they're singletons on instantiation
    '''
    global __registered
    obj = __registered.get(val, None)
    if obj is None:
        __registered[val] = val
        obj = val
    return obj

# Overload and hide the symbol constructor
Symbol = lambda x : __SINGLETON(__Symbol(x)) 
