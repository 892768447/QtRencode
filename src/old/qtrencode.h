#ifndef QTRENCODE_H
#define QTRENCODE_H

#pragma once

#include <QByteArray>
#include <QDataStream>
#include <QDebug>
#include <QJsonDocument>
#include <QVariant>
#include <QtEndian>

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

 public:
  static QByteArray dumps(const QJsonDocument &data);
  static QByteArray dumps(const QByteArray &data);
  static QVariant loads(const QByteArray &data);

 private:
  static void encode(QByteArray *out, const QVariant &value);
  static void encodeInt(QByteArray *out, const QVariant &value);
  static void encodeDouble(QByteArray *out, const QVariant &value);
  static void encodeByteArray(QByteArray *out, const QVariant &value);
  static void encodeBool(QByteArray *out, const QVariant &value);
  static void encodeNone(QByteArray *out);
  static void encodeList(QByteArray *out, const QVariant &value);
  static void encodeMap(QByteArray *out, const QVariant &value);

  static QVariant decode(const QByteArray &in, quint32 *pos);
  static QVariant decodeChar(const QByteArray &in, quint32 *pos);
  static QVariant decodeShort(const QByteArray &in, quint32 *pos);
  static QVariant decodeInt(const QByteArray &in, quint32 *pos);
  static QVariant decodeLongLong(const QByteArray &in, quint32 *pos);
  static QVariant decodeBigNumber(const QByteArray &in, quint32 *pos);
  static QVariant decodeFloat(const QByteArray &in, quint32 *pos);
  static QVariant decodeDouble(const QByteArray &in, quint32 *pos);
  static QVariant decodeList(const QByteArray &in, quint32 *pos);
  static QVariant decodeDict(const QByteArray &in, quint32 *pos);
  static QVariant decodeFixedPosInt(const QByteArray &in, quint32 *pos);
  static QVariant decodeFixedNegInt(const QByteArray &in, quint32 *pos);
  static QVariant decodeFixeByteArray(const QByteArray &in, quint32 *pos);
  static QVariant decodeByteArray(const QByteArray &in, quint32 *pos);
  static QVariant decodeFixedList(const QByteArray &in, quint32 *pos);
  static QVariant decodeFixedDict(const QByteArray &in, quint32 *pos);
};

#endif  // QTRENCODE_H
