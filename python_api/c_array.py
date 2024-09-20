'''
    Wrapper object for Ctype arrays
    Exposes some simple python interfaces
'''

class CArray:
    '''
        CArray
        Python wrapper for Ctype arrays
    '''
    def __init__(self, n_elements : int, arr):
        '''
            Initialiser
            :: n_elements : int :: Number of elements in the array
            :: arr : POINTER(<T>) :: Pointer object to the array
        '''
        self.n_elements = n_elements
        self.arr = arr

    def __iter__(self):
        '''
            Generator style __iter__
            Yields elements of the array in order
        '''
        for i in range(self.n_elements):
            yield self.arr[i]

    def __getitem__(self, idx : int):
        '''
            Returns an item from the array
            :: idx : int :: Index to query
        '''
        if idx > self.n_elements:
            raise IndexError("Index is out of range")
        return self.arr[idx]

    def __len__(self):
        '''
            Returns the length of the array
        '''
        return self.n_elements
