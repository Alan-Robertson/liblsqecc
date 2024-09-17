from ctypes import POINTER, Structure, Union, c_byte, c_int8, c_int32, c_int64, c_int, c_char 
from symbol import Symbol
from enum_wrapper import Enum, EnumDefaultType


class Grid:
    def __init__(self, height, width):
        self.height = height
        self.width = width
        self.grid = [[RoutingAncilla(i, j) for j in range(self.width)] for i in range(self.height)]      

    def setitem(self, Type, y, x): 
       self.grid[y][x] = Type(y, x) 


class LayoutCell:
    def __init__(self, y, x):
        self.x = x
        self.y = y 
    def __gt__(self, other):
        if self.y != other.y:
            return self.y > other.y
        else:
            return self.x > other.x

    def __lt__(self, other):
        return not self.__gt__(other)

    def __le__(self, other):
        return self.__lt__(other)

    def __ge__(self, other):
        return self.__gt__(other)

    def update(self, Type):
        return Type(self.y, self.x)

def cell_factory(name, symbol, enum_val, **kwargs): 
    '''
        Factory method for cell types
    '''
    return type(str(name), (LayoutCell,), {'symbol':symbol, 'enum_val':enum_val} | kwargs)

cell_types_list = [
        (Symbol('RoutingAncilla'), Symbol('r')),
        (Symbol('LogicalComputationQubit_StandardBorderOrientation'), Symbol('Q')),
        (Symbol('LogicalComputationQubit_RotatedBorderOrientation'), Symbol('T')),
        (Symbol('LogicalComputationQubit_DynamicAllocation'), Symbol('D')),
        (Symbol('AncillaQubitLocation'), Symbol('A')),
        (Symbol('DistillationRegion_0'), Symbol('0')),
        (Symbol('DistillationRegion_1'), Symbol('1')),
        (Symbol('DistillationRegion_2'), Symbol('2')),
        (Symbol('DistillationRegion_3'), Symbol('3')),
        (Symbol('DistillationRegion_4'), Symbol('4')),
        (Symbol('DistillationRegion_5'), Symbol('5')),
        (Symbol('DistillationRegion_6'), Symbol('6')),
        (Symbol('DistillationRegion_7'), Symbol('7')),
        (Symbol('DistillationRegion_8'), Symbol('8')),
        (Symbol('DistillationRegion_9'), Symbol('9')),
        (Symbol('ReservedForMagicState'), Symbol('M')),
        (Symbol('DeadCell'), Symbol('X')),
        (Symbol('PreDistilledYState'), Symbol('Y'))
    ]
# Create an enum of the cells
CellEnum = Enum(map(lambda x: x[0], cell_types_list))
# Promote the enum elements to class constructors
cell_classes = map(lambda x: cell_factory(*x, CellEnum[x[0]]), cell_types_list)

# Inject newly created classes into the module namespace
for cell_type, cell_class in zip(cell_types_list, cell_classes):
    locals()[str(cell_type[0])] = cell_class 
