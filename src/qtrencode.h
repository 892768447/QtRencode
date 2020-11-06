#ifndef QTRENCODE_H
#define QTRENCODE_H

#pragma once

#include <QByteArray>
#include <QDataStream>
#include <QDebug>
#include <QJsonDocument>
#include <QSysInfo>
#include <QTextCodec>
#include <QVariant>
#include <QtEndian>

static int FLOAT_BITS = 32;  // 32 or 64
static bool USE_JSON = true;

class QtRencode : public QObject {
  Q_OBJECT

  // Default number of bits for serialized floats, either 32 or 64 (also a
  // parameter for dumps()).
  static const quint8 DEFAULT_FLOAT_BITS = 32;
  // Maximum length of integer when written as base 10 string.
  static const quint8 MAX_INT_LENGTH = 64;
  // The bencode 'typecodes' such as i, d, etc have been extended and relocated
  // on the base-256 character set.
  static const quint8 CHR_LIST = 59;
  static const quint8 CHR_DICT = 60;
  static const quint8 CHR_INT = 61;
  static const quint8 CHR_INT1 = 62;
  static const quint8 CHR_INT2 = 63;
  static const quint8 CHR_INT4 = 64;
  static const quint8 CHR_INT8 = 65;
  static const quint8 CHR_FLOAT32 = 66;
  static const quint8 CHR_FLOAT64 = 44;
  static const quint8 CHR_TRUE = 67;
  static const quint8 CHR_FALSE = 68;
  static const quint8 CHR_NONE = 69;
  static const quint8 CHR_TERM = 127;
  // Positive integers with value embedded in typecode.
  static const quint8 INT_POS_FIXED_START = 0;
  static const quint8 INT_POS_FIXED_COUNT = 44;
  // Dictionaries with length embedded in typecode.
  static const quint8 DICT_FIXED_START = 102;
  static const quint8 DICT_FIXED_COUNT = 25;
  // Negative integers with value embedded in typecode.
  static const quint8 INT_NEG_FIXED_START = 70;
  static const quint8 INT_NEG_FIXED_COUNT = 32;
  // Strings with length embedded in typecode.
  static const quint8 STR_FIXED_START = 128;
  static const quint8 STR_FIXED_COUNT = 64;
  // Lists with length embedded in typecode.
  static const quint8 LIST_FIXED_START = STR_FIXED_START + STR_FIXED_COUNT;
  static const quint8 LIST_FIXED_COUNT = 64;

  static const qint32 MAX_SIGNED_INT = INT_MAX;
  static const qint32 MIN_SIGNED_INT = INT_MIN;
  static const qlonglong MAX_SIGNED_LONGLONG = LLONG_MAX;
  static const qlonglong MIN_SIGNED_LONGLONG = LLONG_MIN;

  static const bool BIG_ENDIAN = QSysInfo::ByteOrder == QSysInfo::BigEndian;

 public:
  static QByteArray dumps(const QByteArray &data, int bits = 32);
  static QByteArray dumps(const QJsonDocument &data, int bits = 32);
  static QByteArray dumps(const QVariant &data, int bits = 32);
  static QVariant loads(const QByteArray &data, bool json = true);

 private:
  static void swap_byte_order_ushort(unsigned short *s);
  static short swap_byte_order_short(char *c);
  static void swap_byte_order_uint(int *i);
  static int swap_byte_order_int(char *c);
  static void swap_byte_order_ulong_long(long long *l);
  static long long swap_byte_order_long_long(char *c);
  static float swap_byte_order_float(char *c);
  static double swap_byte_order_double(char *c);

  static void write_buffer_char(char **buf, unsigned int *pos, char c);
  static void write_buffer(char **buf, unsigned int *pos, void *data, int size);
  static bool check_pos(const QByteArray &data, unsigned int pos);

  static void encode_char(char **buf, unsigned int *pos, signed char x);
  static void encode_short(char **buf, unsigned int *pos, short x);
  static void encode_int(char **buf, unsigned int *pos, int x);
  static void encode_long_long(char **buf, unsigned int *pos, long long x);
  static void encode_big_number(char **buf, unsigned int *pos, QByteArray &x);
  static void encode_float32(char **buf, unsigned int *pos, float x);
  static void encode_float64(char **buf, unsigned int *pos, double x);
  static void encode_str(char **buf, unsigned int *pos, QByteArray x);
  static void encode_none(char **buf, unsigned int *pos);
  static void encode_bool(char **buf, unsigned int *pos, bool x);
  static void encode_list(char **buf, unsigned int *pos, const QVariantList &x);
  static void encode_dict(char **buf, unsigned int *pos,
                          const QMap<QVariant, QVariant> &x);
  static void encode(char **buf, unsigned int *pos, const QVariant &data);

  static QVariant decode_char(const QByteArray &data, unsigned int *pos);
  static QVariant decode_short(const QByteArray &data, unsigned int *pos);
  static QVariant decode_int(const QByteArray &data, unsigned int *pos);
  static QVariant decode_long_long(const QByteArray &data, unsigned int *pos);
  static QVariant decode_fixed_pos_int(const QByteArray &data,
                                       unsigned int *pos);
  static QVariant decode_fixed_neg_int(const QByteArray &data,
                                       unsigned int *pos);
  static QVariant decode_big_number(const QByteArray &data, unsigned int *pos);
  static QVariant decode_float32(const QByteArray &data, unsigned int *pos);
  static QVariant decode_float64(const QByteArray &data, unsigned int *pos);
  static QVariant decode_fixed_str(const QByteArray &data, unsigned int *pos);
  static QVariant decode_str(const QByteArray &data, unsigned int *pos);
  static QVariantList decode_fixed_list(const QByteArray &data,
                                        unsigned int *pos);
  static QVariantList decode_list(const QByteArray &data, unsigned int *pos);
  static QVariant decode_fixed_dict(const QByteArray &data, unsigned int *pos);
  static QVariant decode_dict(const QByteArray &data, unsigned int *pos);
  static QVariant decode(const QByteArray &data, unsigned int *pos);
};

extern "C" {
#ifdef _WIN32
__declspec(dllexport) void dumps(QByteArray &out, const QVariant &data,
                                 int bits = 32);
__declspec(dllexport) void loads(QVariant &out, const QByteArray &data,
                                 bool json = true);
#else
void dumps(QByteArray &out, const QVariant &data, int bits = 32);
void loads(QVariant &out, const QByteArray &data, bool json = true);
#endif
};

#endif  // QTRENCODE_H
