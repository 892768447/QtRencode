#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
Created on 2020年11月5日
@author: Irony
@site: https://pyqt.site https://github.com/PyQt5
@email: 892768447@qq.com
@file:
@description:
"""

import sys
import unittest
from ctypes import CDLL

from rencode import _rencode as rencode
from rencode import rencode_orig

# For PyQt5
try:
    from PyQt5 import sip
except:
    import sip
from PyQt5.QtCore import QVariant, QByteArray

# For PySide2
# import shiboken2
# from PySide2.QtCore import Qt, QByteArray, QVariant, QCoreApplication

# Hack to deal with python 2 and 3 differences with unicode literals.
if sys.version < '3':
    import codecs


    def u(x):
        return codecs.unicode_escape_decode(x)[0]
else:
    unicode = str


    def u(x):
        return x

dll = CDLL('../Release/src/release/QtRencode.dll')


def q_dumps(value, bits=32):
    def en_byte(v):
        if isinstance(v, str):
            return QByteArray(v.encode())
        elif isinstance(v, (bytes, bytearray)):
            return QByteArray(v)
        return v

    try:
        if isinstance(value, str):
            src = QVariant(QByteArray(value.encode()))
        elif isinstance(value, (bytes, bytearray)):
            src = QVariant(QByteArray(value))
        elif isinstance(value, (tuple, list)):
            src = QVariant([QVariant(en_byte(v)) for v in value])
        else:
            src = QVariant(value)
        dst = QByteArray()
        dll.dumps(sip.unwrapinstance(dst), sip.unwrapinstance(src), bits)
        # dll.dumps(shiboken2.getCppPointer(dst)[0], shiboken2.getCppPointer(src)[0])
        return dst.data()
    except Exception as e:
        print('q_dumps error:', e)
        raise ValueError


def q_loads(src):
    def try_getqbytearray(v):
        if isinstance(v, QByteArray):
            return v.data()
        return v

    try:
        if isinstance(src, str):
            src = QByteArray(src.encode())
        elif isinstance(src, (list, tuple)):
            src = QByteArray(bytes(src))
        else:
            src = QByteArray(src)
        dst = QVariant()
        dll.loads(sip.unwrapinstance(dst), sip.unwrapinstance(src), False)
        # dll.dumps(shiboken2.getCppPointer(dst)[0], shiboken2.getCppPointer(src)[0])
        value = dst.value()
        if isinstance(value, str):
            value = value.encode()
        if isinstance(value, list):
            value = [try_getqbytearray(v) for v in value]
        return value
    except Exception as e:
        print('q_loads error:', e)
        raise ValueError


class TestRencode(unittest.TestCase):
    def test_encode_fixed_pos_int(self):
        self.assertTrue(rencode.dumps(1) == rencode_orig.dumps(1) == q_dumps(1))
        self.assertTrue(rencode.dumps(40) == rencode_orig.dumps(40) == q_dumps(40))

    def test_encode_fixed_neg_int(self):
        self.assertTrue(rencode.dumps(-10) == rencode_orig.dumps(-10) == q_dumps(-10))
        self.assertTrue(rencode.dumps(-29) == rencode_orig.dumps(-29) == q_dumps(-29))

    def test_encode_int_char_size(self):
        self.assertTrue(rencode.dumps(100) == rencode_orig.dumps(100) == q_dumps(100))
        self.assertTrue(rencode.dumps(-100) == rencode_orig.dumps(-100) == q_dumps(-100))

    def test_encode_int_short_size(self):
        self.assertTrue(rencode.dumps(27123) == rencode_orig.dumps(27123) == q_dumps(27123))
        self.assertTrue(rencode.dumps(-27123) == rencode_orig.dumps(-27123) == q_dumps(-27123))

    def test_encode_int_int_size(self):
        self.assertTrue(rencode.dumps(7483648) == rencode_orig.dumps(7483648) == q_dumps(7483648))
        self.assertTrue(rencode.dumps(-7483648) == rencode_orig.dumps(-7483648) == q_dumps(-7483648))

    def test_encode_int_long_long_size(self):
        self.assertTrue(rencode.dumps(8223372036854775808) == rencode_orig.dumps(8223372036854775808) == q_dumps(
            8223372036854775808))
        self.assertTrue(rencode.dumps(-8223372036854775808) == rencode_orig.dumps(-8223372036854775808) == q_dumps(
            -8223372036854775808))

    def test_encode_int_big_number(self):
        n = int("9" * 62)
        self.assertTrue(rencode.dumps(n) == rencode_orig.dumps(n))
        self.assertRaises(ValueError, rencode.dumps, int("9" * 65))

    def test_encode_float_32bit(self):
        self.assertTrue(rencode.dumps(1234.56) == rencode_orig.dumps(1234.56) == q_dumps(1234.56))

    def test_encode_float_64bit(self):
        self.assertTrue(rencode.dumps(1234.56, 64) == rencode_orig.dumps(1234.56, 64) == q_dumps(1234.56, 64))

    def test_encode_float_invalid_size(self):
        self.assertRaises(ValueError, rencode.dumps, 1234.56, 36)
        self.assertTrue(b'' == q_dumps(1234.56, 36))

    def test_encode_fixed_str(self):
        self.assertTrue(rencode.dumps(b"foobarbaz") == rencode_orig.dumps(b"foobarbaz") == q_dumps(b"foobarbaz"))

    def test_encode_str(self):
        self.assertTrue(rencode.dumps(b"f" * 255) == rencode_orig.dumps(b"f" * 255) == q_dumps(b"f" * 255))
        self.assertTrue(rencode.dumps(b"\0") == rencode_orig.dumps(b"\0") == q_dumps(b"\0"))

    def test_encode_unicode(self):
        self.assertTrue(rencode.dumps(u("fööbar")) == rencode_orig.dumps(u("fööbar")) == q_dumps(u("fööbar")))

    def test_encode_none(self):
        self.assertTrue(rencode.dumps(None) == rencode_orig.dumps(None) == q_dumps(None))

    def test_encode_bool(self):
        self.assertTrue(rencode.dumps(True) == rencode_orig.dumps(True) == q_dumps(True))
        self.assertTrue(rencode.dumps(False) == rencode_orig.dumps(False) == q_dumps(False))

    def test_encode_fixed_list(self):
        l = [100, -234.01, b"foobar", u("bäz")] * 4
        self.assertTrue(rencode.dumps(l) == rencode_orig.dumps(l) == q_dumps(l))

    def test_encode_list(self):
        l = [100, -234.01, b"foobar", u("bäz")] * 80
        self.assertTrue(rencode.dumps(l) == rencode_orig.dumps(l) == q_dumps(l))

    def test_encode_fixed_dict(self):
        s = b"abcdefghijk"
        d = dict(zip(s, [1234] * len(s)))
        self.assertTrue(rencode.dumps(d) == rencode_orig.dumps(d))

    def test_encode_dict(self):
        s = b"abcdefghijklmnopqrstuvwxyz1234567890"
        d = dict(zip(s, [1234] * len(s)))
        self.assertTrue(rencode.dumps(d) == rencode_orig.dumps(d))

    def test_decode_fixed_pos_int(self):
        self.assertTrue(rencode.loads(rencode.dumps(10)) == 10 == q_loads(rencode.dumps(10)))

    def test_decode_fixed_neg_int(self):
        self.assertTrue(rencode.loads(rencode.dumps(-10)) == -10 == q_loads(rencode.dumps(-10)))

    def test_decode_char(self):
        self.assertTrue(rencode.loads(rencode.dumps(100)) == 100 == q_loads(rencode.dumps(100)))
        self.assertTrue(rencode.loads(rencode.dumps(-100)) == -100 == q_loads(rencode.dumps(-100)))
        self.assertRaises(IndexError, rencode.loads, bytes(bytearray([62])))
        self.assertTrue(0 == q_loads(bytes(bytearray([62]))))

    def test_decode_short(self):
        self.assertTrue(rencode.loads(rencode.dumps(27123)) == 27123 == q_loads(rencode.dumps(27123)))
        self.assertTrue(rencode.loads(rencode.dumps(-27123)) == -27123 == q_loads(rencode.dumps(-27123)))
        self.assertRaises(IndexError, rencode.loads, bytes(bytearray([63])))
        self.assertTrue(0 == q_loads(bytes(bytearray([63]))))

    def test_decode_int(self):
        self.assertTrue(rencode.loads(rencode.dumps(7483648)) == 7483648 == q_loads(rencode.dumps(7483648)))
        self.assertTrue(rencode.loads(rencode.dumps(-7483648)) == -7483648 == q_loads(rencode.dumps(-7483648)))
        self.assertRaises(IndexError, rencode.loads, bytes(bytearray([64])))
        self.assertTrue(0 == q_loads(bytes(bytearray([64]))))

    def test_decode_long_long(self):
        self.assertTrue(rencode.loads(rencode.dumps(8223372036854775808)) == 8223372036854775808 == q_loads(
            rencode.dumps(8223372036854775808)))
        self.assertTrue(rencode.loads(rencode.dumps(-8223372036854775808)) == -8223372036854775808 == q_loads(
            rencode.dumps(-8223372036854775808)))
        self.assertRaises(IndexError, rencode.loads, bytes(bytearray([65])))
        self.assertTrue(0 == q_loads(bytes(bytearray([65]))))

    def test_decode_int_big_number(self):
        n = int(b"9" * 62)
        toobig = '={x}\x7f'.format(x='9' * 65).encode()
        self.assertTrue(rencode.loads(rencode.dumps(n)) == n)
        self.assertRaises(IndexError, rencode.loads, bytes(bytearray([61])))
        self.assertRaises(ValueError, rencode.loads, toobig)
        self.assertTrue(0 == q_loads(bytes(bytearray([61]))))
        self.assertTrue(0 == q_loads(toobig))

    def test_decode_float_32bit(self):
        f = rencode.dumps(1234.56)
        self.assertTrue(rencode.loads(f) == rencode_orig.loads(f) == q_loads(f))
        self.assertRaises(IndexError, rencode.loads, bytes(bytearray([66])))
        self.assertTrue(0 == q_loads(bytes(bytearray([66]))))

    def test_decode_float_64bit(self):
        f = rencode.dumps(1234.56, 64)
        self.assertTrue(rencode.loads(f) == rencode_orig.loads(f) == q_loads(f))
        self.assertRaises(IndexError, rencode.loads, bytes(bytearray([44])))
        self.assertTrue(0 == q_loads(bytes(bytearray([44]))))

    def test_decode_fixed_str(self):
        self.assertTrue(
            rencode.loads(rencode.dumps(b"foobarbaz")) == b"foobarbaz" == q_loads(rencode.dumps(b"foobarbaz")))
        self.assertRaises(IndexError, rencode.loads, bytes(bytearray([130])))
        self.assertTrue(0 == q_loads(bytes(bytearray([130]))))

    def test_decode_str(self):
        self.assertTrue(rencode.loads(rencode.dumps(b"f" * 255)) == b"f" * 255 == q_loads(rencode.dumps(b"f" * 255)))
        self.assertRaises(IndexError, rencode.loads, b"50")
        self.assertTrue(0 == q_loads(b"50"))

    def test_decode_unicode(self):
        self.assertTrue(rencode.loads(rencode.dumps(u("fööbar"))) == u("fööbar").encode("utf8") == q_loads(
            rencode.dumps(u("fööbar"))))

    def test_decode_none(self):
        self.assertIsNone(rencode.loads(rencode.dumps(None)))
        self.assertIsNone(q_loads(rencode.dumps(None)))

    def test_decode_bool(self):
        self.assertTrue(rencode.loads(rencode.dumps(True)) == True == q_loads(rencode.dumps(True)))
        self.assertTrue(rencode.loads(rencode.dumps(False)) == False == q_loads(rencode.dumps(False)))

    def test_decode_fixed_list(self):
        l = [100, False, b"foobar", u("bäz").encode("utf8")] * 4
        self.assertListEqual(list(rencode.loads(rencode.dumps(l))), l)
        self.assertListEqual(q_loads(rencode.dumps(l)), l)
        self.assertRaises(IndexError, rencode.loads, bytes(bytearray([194])))
        self.assertTrue([] == q_loads(bytes(bytearray([194]))))

    def test_decode_list(self):
        l = [100, False, b"foobar", u("bäz").encode("utf8")] * 80
        self.assertTrue(list(rencode.loads(rencode.dumps(l))) == list(l) == q_loads(rencode.dumps(l)))
        self.assertRaises(IndexError, rencode.loads, bytes(bytearray([59])))
        self.assertTrue(0 == q_loads(bytes(bytearray([59]))))

    def test_decode_fixed_dict(self):
        s = b"abcdefghijk"
        d = dict(zip(s, [1234] * len(s)))
        self.assertTrue(list(rencode.loads(rencode.dumps(d))) == list(d))
        self.assertRaises(IndexError, rencode.loads, bytes(bytearray([104])))

    def test_decode_dict(self):
        s = b"abcdefghijklmnopqrstuvwxyz1234567890"
        d = dict(zip(s, [b"foo" * 120] * len(s)))
        d2 = {b"foo": d, b"bar": d, b"baz": d}
        self.assertTrue(rencode.loads(rencode.dumps(d2)) == d2)
        self.assertRaises(IndexError, rencode.loads, bytes(bytearray([60])))
        self.assertTrue(0 == q_loads(bytes(bytearray([60]))))

    def test_decode_str_bytes(self):
        b = [202, 132, 100, 114, 97, 119, 1, 0, 0, 63, 1, 242, 63]
        d = bytes(bytearray(b))
        self.assertTrue(rencode.loads(rencode.dumps(d)) == d == q_loads(rencode.dumps(d)))

    def test_decode_str_nullbytes(self):
        b = (
            202, 132, 100, 114, 97, 119, 1, 0, 0, 63, 1, 242, 63, 1, 60, 132, 120, 50, 54, 52, 49, 51, 48, 58, 0, 0, 0,
            1,
            65, 154, 35, 215, 48, 204, 4, 35, 242, 3, 122, 218, 67, 192, 127, 40, 241, 127, 2, 86, 240, 63, 135, 177,
            23,
            119, 63, 31, 226, 248, 19, 13, 192, 111, 74, 126, 2, 15, 240, 31, 239, 48, 85, 238, 159, 155, 197, 241, 23,
            119,
            63, 2, 23, 245, 63, 24, 240, 86, 36, 176, 15, 187, 185, 248, 242, 255, 0, 126, 123, 141, 206, 60, 188, 1,
            27,
            254, 141, 169, 132, 93, 220, 252, 121, 184, 8, 31, 224, 63, 244, 226, 75, 224, 119, 135, 229, 248, 3, 243,
            248,
            220, 227, 203, 193, 3, 224, 127, 47, 134, 59, 5, 99, 249, 254, 35, 196, 127, 17, 252, 71, 136, 254, 35, 196,
            112, 4, 177, 3, 63, 5, 220)
        d = bytes(bytearray(b))
        self.assertTrue(rencode.loads(rencode.dumps(d)) == d == q_loads(rencode.dumps(d)))

    def test_decode_utf8(self):
        s = b"foobarbaz"
        # no assertIsInstance with python2.6
        d = rencode.loads(rencode.dumps(s), decode_utf8=True)
        if not isinstance(d, unicode):
            self.fail('%s is not an instance of %r' % (repr(d), unicode))
        s = rencode.dumps(b"\x56\xe4foo\xc3")
        self.assertRaises(UnicodeDecodeError, rencode.loads, s, decode_utf8=True)

    def test_version_exposed(self):
        assert rencode.__version__
        assert rencode_orig.__version__
        self.assertEqual(rencode.__version__[1:], rencode_orig.__version__[1:], "version number does not match")


if __name__ == '__main__':
    unittest.main()
