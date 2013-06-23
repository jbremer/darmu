from ctypes import cdll, POINTER, Structure, create_string_buffer, memmove
from ctypes import c_char, c_int32, c_uint32, c_char_p, c_void_p


class _Darmu(Structure):
    _fields_ = [
        ('padding', c_char * 0x400)
    ]


class Darmu(object):
    def __init__(self, image, stack_size):
        self.image = image
        self.stack = create_string_buffer(stack_size)

        self.darmu = _Darmu()
        _lib.darmu_init(self.darmu, self.stack, stack_size)

        self.mappings = []

        self._gc = [self.image, self.stack]

    def set_argv_env(self, argv, env):
        # TODO initialize the contents of the stack
        pass

    def entry_point(self, addr):
        self.r15 = addr

    def mapping(self, raw_offset, size, addr):
        mem = self.image[raw_offset:raw_offset+size]
        self._gc.append(mem)

        self.mappings.append((raw_offset, size, addr))

        _lib.darmu_mapping_add(self.darmu, mem, size, addr)

    def single_step(self):
        ret = _lib.darmu_single_step(self.darmu)
        return ret == 0

    def read(self, addr, size):
        buf = create_string_buffer(size)
        ptr = _lib.darmu_mapping_lookup_raw(self.darmu, addr)
        memmove(buf, ptr, size)
        return buf.raw

    def write(self, addr, buf):
        ptr = _lib.darmu_mapping_lookup_raw(self.darmu, addr)
        memmove(ptr, buf, len(buf))

    def _reg(idx):
        def _read(self):
            return _lib.darmu_register_get(self.darmu, idx)

        def _write(self, value):
            _lib.darmu_register_set(self.darmu, idx, value)

        return property(_read, _write)

    for _ in xrange(16):
        locals()['r%d' % _] = _reg(_)


def _set_func(name, restype, *argtypes):
    getattr(_lib, name).restype = restype
    getattr(_lib, name).argtypes = argtypes

_lib = cdll.LoadLibrary('./libdarmu.so')
_set_func('darmu_init', c_int32, POINTER(_Darmu), c_char_p)
_set_func('darmu_mapping_add', c_int32, POINTER(_Darmu), c_char_p, c_uint32,
          c_uint32)
_set_func('darmu_mapping_lookup_virtual', c_uint32, POINTER(_Darmu), c_uint32)
_set_func('darmu_mapping_lookup_raw', c_void_p, POINTER(_Darmu), c_uint32)
_set_func('darmu_register_get', c_uint32, POINTER(_Darmu), c_uint32)
_set_func('darmu_register_set', None, POINTER(_Darmu), c_uint32, c_uint32)
_set_func('darmu_flags_get', c_uint32, POINTER(_Darmu))
_set_func('darmu_flags_set', None, POINTER(_Darmu), c_uint32)
_set_func('darmu_single_step', c_int32, POINTER(_Darmu))
