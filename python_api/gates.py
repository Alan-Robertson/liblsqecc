from ctypes import POINTER, Structure, Union, c_byte, c_int8, c_int32, c_int64, c_int, c_char 
from symbol import Symbol
from enum_wrapper import Enum, EnumDefaultType

# Matching layer for types defined in `gates.hpp` 
BoolType = c_int8 
GateType = c_int8
QubitNumType = c_int32 
ArbitraryPrecisionInteger = c_int64 # Yes, this in fact not a bigint 

# Possibly grab this from the lib directly to be better supported
# Map symbol strings to symbols
single_qubit_gate_opcodes = list(
    map(Symbol, 
    ('X', 'Y', 'Z', 'S', 'T', 'H', 'SDg', 'TDg')
))
SingleQubitGateType = c_int8
SingleQubitGateEnum = Enum(single_qubit_gate_opcodes, c_type=SingleQubitGateType)
    

CNOTEnum_vals = list(
    map(Symbol, 
        ('ZX_WITH_MBM_CONTROL_FIRST',
         'ZX_WITH_MBM_TARGET_FIRST',
         'BELL_BASED')
))
CNOTEnumType = EnumDefaultType 
CNOTEnum = Enum(CNOTEnum_vals, c_type=CNOTEnumType)

CNOTAncillaPlacementEnum_vals = list(
    map(Symbol,
        ('ANCILLA_FREE_PLACEMENT',
         'ANCILLA_NEXT_TO_CONTROL',
         'ANCILLA_NEXT_TO_TARGET')
))
CNOTAncillaPlacementEnumType = EnumDefaultType
CNOTAncillaPlacementEnum = Enum(CNOTAncillaPlacementEnum_vals, c_type=CNOTAncillaPlacementEnumType)
# Expose symbols to module scope

class BasicSingleQubitGateType(Structure):
    _fields_ = [
        ('gate_type', GateType),
        ('target_qubit', QubitNumType),
    ]

class FractionType(Structure):
    '''
       Provides a basis for pi fractions needed by Gridsynth 
    ''' 
    _fields_ = [
        ('numerator', ArbitraryPrecisionInteger),
        ('denominator', ArbitraryPrecisionInteger),
        ('is_negative', BoolType),
    ]

class RZGateType(Structure):
    '''
        These gates support Rz rotations
    '''
    _fields_ = [
        ('target_qubit', QubitNumType),
        ('pi_fraction', FractionType)
        ]

class SingleQubitGateType(Union):
    '''
        Union of single qubit gate types
    '''
    _fields_ = [
        ('clifford', BasicSingleQubitGateType),
        ('rz', RZGateType)
    ]
    
class ControlledGateType(Structure):
    '''
        Base case of two qubit gates 
    ''' 
    _fields_ = [
        ('control_qubit', QubitNumType),
        ('target_gate', SingleQubitGateType),
        ('cnot_type', CNOTEnumType),
        ('cnot_ancillae_placement', CNOTAncillaPlacementEnumType),
    ]


class Reset(Structure):
    _fields = [
        ('register_name', POINTER(c_char)),
        ('target_qubit', QubitNumType)
        ]

# Constructor for single qubit gates
for sym in single_qubit_gate_opcodes: 
    locals()[str(sym)] = lambda target_qubit: SingleQubitGateType(
        BasicSingleQubitGateType(
            SingleQubitGateEnum[sym],
            QubitNumType(target_qubit)
        )
    )    
# RZ Constructor
def RZ(target_qubit, numerator, denominator): 
    gate = SingleQubitGateType()
    gate.rz = RZGateType(
                target_qubit,
                FractionType(
                    numerator,
                    denominator,
                    0, # Marked as an unused field in gates.hpp
                )
            )
    return gate

# Constructor for two qubit gates
CNOT = lambda ctrl, targ: ControlledGateType(
        ctrl,
        X(targ),
        CNOTEnum['ZX_WITH_MBM_CONTROL_FIRST'],
        CNOTAncillaPlacementEnum['ANCILLA_NEXT_TO_CONTROL']
    )

CZ = lambda ctrl, targ: ControlledGateType(
        ctrl,
        Z(targ),
        CNOTEnum['ZX_WITH_MBM_CONTROL_FIRST'],
        CNOTAncillaPlacementEnum['ANCILLA_NEXT_TO_CONTROL']
    )

CRZ = lambda ctrl, targ, numerator, denominator, cnot_type, anc_type  : ControlledGateType(
        ctrl,
        RZ(targ),
        CNOTEnum['ZX_WITH_MBM_CONTROL_FIRST'],
        CNOTAncillaPlacementEnum['ANCILLA_NEXT_TO_CONTROL']
    )
