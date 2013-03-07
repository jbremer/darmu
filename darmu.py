from ctypes import cdll, POINTER, Structure, create_string_buffer
from ctypes import c_char, c_int32, c_uint32, c_char_p


class _Darmu(Structure):
    _fields_ = [
        ('padding', c_char * 0x200)
    ]


class Darmu:
    def __init__(self, image, stack_size):
        self.image = image
        self.stack = create_string_buffer(stack_size)

        self.darmu = _Darmu()

        _lib.darmu_init(self.darmu, self.image, self.stack)

    def set_argv_env(self, argv, env):
        # TODO initialize the contents of the stack
        pass

    def entry_point(self, address):
        _lib.darmu_register_set(self.darmu, 0b1111, address)

    def mapping(self, raw, raw_size, virtual, virtual_size):
        _lib.darmu_mapping_add(self.darmu,
                               raw,
                               raw_size,
                               virtual,
                               virtual_size)

    def single_step(self):
        ret = _lib.darmu_single_step(self.darmu)
        return ret == 0


def _set_func(name, restype, *argtypes):
    getattr(_lib, name).restype = restype
    getattr(_lib, name).argtypes = argtypes

_lib = cdll.LoadLibrary('libdarmu.so')
_set_func('darmu_init', c_int32, POINTER(_Darmu), c_char_p, c_char_p)
_set_func('darmu_mapping_add', c_int32, POINTER(_Darmu), c_uint32, c_uint32,
          c_uint32, c_uint32)
_set_func('darmu_mapping_lookup_virtual', c_uint32, POINTER(_Darmu), c_uint32)
_set_func('darmu_mapping_lookup_raw', c_uint32, POINTER(_Darmu), c_uint32)
_set_func('darmu_register_get', c_uint32, POINTER(_Darmu), c_uint32)
_set_func('darmu_register_set', None, POINTER(_Darmu), c_uint32, c_uint32)
_set_func('darmu_flags_get', c_uint32, POINTER(_Darmu))
_set_func('darmu_flags_set', None, POINTER(_Darmu), c_uint32)
_set_func('darmu_single_step', c_int32, POINTER(_Darmu))
