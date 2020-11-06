#include "qtrencode.h"

/**
 * @brief QtRencode::dumps
 * @param data
 * @param bits
 * @return QByteArray
 * 对原始json数据编码
 */
QByteArray QtRencode::dumps(const QByteArray &data, int bits) {
  FLOAT_BITS = bits;
  QJsonParseError error;
  QJsonDocument json = QJsonDocument::fromJson(data, &error);
  if (error.error != QJsonParseError::NoError) {
    qCritical() << error.errorString();
    return QByteArray();
  }
  return dumps(json, bits);
}

/**
 * @brief QtRencode::dumps
 * @param data
 * @param bits
 * @return QByteArray
 * 对josn数据编码
 */
QByteArray QtRencode::dumps(const QJsonDocument &data, int bits) {
  FLOAT_BITS = bits;
  return dumps(data.toVariant());
}

/**
 * @brief QtRencode::dumps
 * @param data
 * @param bits
 * @return QByteArray
 * 对QVariant数据编码
 */
QByteArray QtRencode::dumps(const QVariant &data, int bits) {
  FLOAT_BITS = bits;
  char *buf = NULL;
  unsigned int pos = 0;
  encode(&buf, &pos, data);
  QByteArray result = QByteArray(buf, pos);
  free(buf);
  return result;
}

/**
 * @brief QtRencode::loads
 * @param data
 * @return QVariant
 * 编码后的数据解码为json
 */
QVariant QtRencode::loads(const QByteArray &data, bool json) {
  USE_JSON = json;
  quint32 pos = 0;
  return decode(data, &pos);
}

void QtRencode::swap_byte_order_ushort(unsigned short *s) {
  s[0] = (s[0] >> 8) | (s[0] << 8);
}

short QtRencode::swap_byte_order_short(char *c) {
  short s;
  char *p = (char *)(&s);
  p[0] = c[1];
  p[1] = c[0];
  return s;
}

void QtRencode::swap_byte_order_uint(int *i) {
  i[0] = (i[0] >> 24) | ((i[0] << 8) & 0x00FF0000) |
         ((i[0] >> 8) & 0x0000FF00) | (i[0] << 24);
}

int QtRencode::swap_byte_order_int(char *c) {
  int i;
  char *p = (char *)(&i);
  p[0] = c[3];
  p[1] = c[2];
  p[2] = c[1];
  p[3] = c[0];
  return i;
}

void QtRencode::swap_byte_order_ulong_long(long long *l) {
  l[0] =
      (l[0] >> 56) | ((l[0] << 40) & 0x00FF000000000000) |
      ((l[0] << 24) & 0x0000FF0000000000) | ((l[0] << 8) & 0x000000FF00000000) |
      ((l[0] >> 8) & 0x00000000FF000000) | ((l[0] >> 24) & 0x0000000000FF0000) |
      ((l[0] >> 40) & 0x000000000000FF00) | (l[0] << 56);
}

long long QtRencode::swap_byte_order_long_long(char *c) {
  long long l;
  char *p = (char *)(&l);
  p[0] = c[7];
  p[1] = c[6];
  p[2] = c[5];
  p[3] = c[4];
  p[4] = c[3];
  p[5] = c[2];
  p[6] = c[1];
  p[7] = c[0];
  return l;
}

float QtRencode::swap_byte_order_float(char *c) {
  float f;
  char *p = (char *)(&f);
  p[0] = c[3];
  p[1] = c[2];
  p[2] = c[1];
  p[3] = c[0];
  return f;
}

double QtRencode::swap_byte_order_double(char *c) {
  double d;
  char *p = (char *)(&d);
  p[0] = c[7];
  p[1] = c[6];
  p[2] = c[5];
  p[3] = c[4];
  p[4] = c[3];
  p[5] = c[2];
  p[6] = c[1];
  p[7] = c[0];
  return d;
}

void QtRencode::write_buffer_char(char **buf, unsigned int *pos, char c) {
  buf[0] = (char *)(realloc(buf[0], pos[0] + 1));
  Q_ASSERT_X(buf[0] != NULL, "write_buffer_char",
             "Error in realloc, 1 byte needed");
  memcpy(&buf[0][pos[0]], &c, 1);
  pos[0] += 1;
}

void QtRencode::write_buffer(char **buf, unsigned int *pos, void *data,
                             int size) {
  buf[0] = (char *)(realloc(buf[0], pos[0] + size));
  Q_ASSERT_X(
      buf[0] != NULL, "write_buffer",
      QString("Error in realloc, %1 bytes needed").arg(size).toUtf8().data());
  memcpy(&buf[0][pos[0]], data, size);
  pos[0] += size;
}

bool QtRencode::check_pos(const QByteArray &data, unsigned int pos) {
  if (pos >= (unsigned int)data.size()) {
    qCritical() << "Tried to access data[" << pos
                << "] but data len is: " << data.size();
    return false;
  }
  return true;
}

void QtRencode::encode_char(char **buf, unsigned int *pos, signed char x) {
  qDebug() << "---encode_char---" << &buf << pos[0] << x;
  if (0 <= x && x < INT_POS_FIXED_COUNT)
    write_buffer_char(buf, pos, INT_POS_FIXED_START + x);
  else if (-INT_NEG_FIXED_COUNT <= x && x < 0)
    write_buffer_char(buf, pos, INT_NEG_FIXED_START - 1 - x);
  else if (-128 <= x && x <= 127) {
    write_buffer_char(buf, pos, CHR_INT1);
    write_buffer_char(buf, pos, x);
  }
}

void QtRencode::encode_short(char **buf, unsigned int *pos, short x) {
  qDebug() << "---encode_short---" << &buf << pos[0] << x;
  write_buffer_char(buf, pos, CHR_INT2);
  if (!BIG_ENDIAN) {
    if (x > 0)
      swap_byte_order_ushort((unsigned short *)(&x));
    else
      x = swap_byte_order_short((char *)(&x));
  }
  write_buffer(buf, pos, &x, sizeof(x));
}

void QtRencode::encode_int(char **buf, unsigned int *pos, int x) {
  qDebug() << "---encode_int---" << &buf << pos[0] << x;
  write_buffer_char(buf, pos, CHR_INT4);
  if (!BIG_ENDIAN) {
    if (x > 0)
      swap_byte_order_uint(&x);
    else
      x = swap_byte_order_int((char *)(&x));
  }
  write_buffer(buf, pos, &x, sizeof(x));
}

void QtRencode::encode_long_long(char **buf, unsigned int *pos, long long x) {
  qDebug() << "---encode_long_long---" << &buf << pos[0] << x;
  write_buffer_char(buf, pos, CHR_INT8);
  if (!BIG_ENDIAN) {
    if (x > 0)
      swap_byte_order_ulong_long(&x);
    else
      x = swap_byte_order_long_long((char *)(&x));
  }
  write_buffer(buf, pos, &x, sizeof(x));
}

void QtRencode::encode_big_number(char **buf, unsigned int *pos,
                                  QByteArray &x) {
  qDebug() << "---encode_big_number---" << &buf << pos[0] << x;
  write_buffer_char(buf, pos, CHR_INT);
  char *d = x.data();
  write_buffer(buf, pos, d, x.size());
  write_buffer_char(buf, pos, CHR_TERM);
}

void QtRencode::encode_float32(char **buf, unsigned int *pos, float x) {
  qDebug() << "---encode_float32---" << &buf << pos[0] << x;
  write_buffer_char(buf, pos, CHR_FLOAT32);
  if (!BIG_ENDIAN) x = swap_byte_order_float((char *)(&x));
  write_buffer(buf, pos, &x, sizeof(x));
}

void QtRencode::encode_float64(char **buf, unsigned int *pos, double x) {
  qDebug() << "---encode_float64---" << &buf << pos[0] << x;
  write_buffer_char(buf, pos, CHR_FLOAT64);
  if (!BIG_ENDIAN) x = swap_byte_order_double((char *)(&x));
  write_buffer(buf, pos, &x, sizeof(x));
}

void QtRencode::encode_str(char **buf, unsigned int *pos, QByteArray x) {
  qDebug() << "---encode_str---" << &buf << pos[0] << x;
  int lx = x.size();
  char *d = x.data();
  if (lx < STR_FIXED_COUNT) {
    write_buffer_char(buf, pos, STR_FIXED_START + lx);
    write_buffer(buf, pos, d, lx);
  } else {
    QString s = QString::number(lx) + ":";
    QByteArray tmp = s.toLatin1();
    char *p = tmp.data();
    write_buffer(buf, pos, p, tmp.size());
    write_buffer(buf, pos, d, lx);
  }
}

void QtRencode::encode_none(char **buf, unsigned int *pos) {
  qDebug() << "---encode_none---" << &buf << pos[0] << "none";
  write_buffer_char(buf, pos, CHR_NONE);
}

void QtRencode::encode_bool(char **buf, unsigned int *pos, bool x) {
  qDebug() << "---encode_bool---" << &buf << pos[0] << x;
  if (x)
    write_buffer_char(buf, pos, CHR_TRUE);
  else
    write_buffer_char(buf, pos, CHR_FALSE);
}

void QtRencode::encode_list(char **buf, unsigned int *pos,
                            const QVariantList &x) {
  qDebug() << "---encode_list---" << &buf << pos[0] << x;
  if (x.size() < LIST_FIXED_COUNT) {
    write_buffer_char(buf, pos, LIST_FIXED_START + x.size());
    for (QVariant i : x) encode(buf, pos, i);
  } else {
    write_buffer_char(buf, pos, CHR_LIST);
    for (QVariant i : x) encode(buf, pos, i);
    write_buffer_char(buf, pos, CHR_TERM);
  }
}

void QtRencode::encode_dict(char **buf, unsigned int *pos,
                            const QMap<QVariant, QVariant> &x) {
  qDebug() << "---encode_dict---" << &buf << pos[0] << x;
  if (x.size() < DICT_FIXED_COUNT) {
    write_buffer_char(buf, pos, DICT_FIXED_START + x.size());
    for (auto it = x.begin(); it != x.end(); it++) {
      encode(buf, pos, it.key());
      encode(buf, pos, it.value());
    }
  } else {
    write_buffer_char(buf, pos, CHR_DICT);
    for (auto it = x.begin(); it != x.end(); it++) {
      encode(buf, pos, it.key());
      encode(buf, pos, it.value());
    }
    write_buffer_char(buf, pos, CHR_TERM);
  }
}

void QtRencode::encode(char **buf, unsigned int *pos, const QVariant &data) {
  if (data.type() == QVariant::List)
    encode_list(buf, pos, data.toList());
  else if (data.type() == QVariant::Map ||
           data.canConvert<QMap<QVariant, QVariant>>())
    encode_dict(buf, pos, data.value<QMap<QVariant, QVariant>>());
  else if (data.type() == QVariant::Bool)
    encode_bool(buf, pos, data.toBool());
  else if (data.type() == QVariant::String ||
           data.type() == QVariant::ByteArray) {
    encode_str(buf, pos, data.toByteArray());
  } else if (data.isNull())
    encode_none(buf, pos);
  else if (data.type() == QVariant::Double)
    if (FLOAT_BITS == 32)
      encode_float32(buf, pos, data.toFloat());
    else if (FLOAT_BITS == 64)
      encode_float64(buf, pos, data.toDouble());
    else {
      qCritical() << "Float bits (" << FLOAT_BITS << ") is not 32 or 64";
      return;
    }
  else if (data.canConvert(QVariant::LongLong)) {
    // char short int float double long longlong
    qlonglong v = data.toLongLong();
    if (-128 <= v && v < 128)
      encode_char(buf, pos, data.value<qint8>());
    else if (-32768 <= v && v < 32768)
      encode_short(buf, pos, data.value<qint16>());
    else if (MIN_SIGNED_INT <= v && v < MAX_SIGNED_INT)
      encode_int(buf, pos, data.toInt());
    else if (MIN_SIGNED_LONGLONG <= v && v < MAX_SIGNED_LONGLONG)
      encode_long_long(buf, pos, v);
    else {
      QByteArray tmp = data.toByteArray();
      if (tmp.size() >= MAX_INT_LENGTH) {
        qCritical() << "Number is longer than " << MAX_INT_LENGTH
                    << " characters";
        return;
      }
      encode_big_number(buf, pos, tmp);
    }
  } else
    Q_ASSERT_X(
        false, "encode",
        QString("type %1 not handled").arg(data.typeName()).toUtf8().data());
}

QVariant QtRencode::decode_char(const QByteArray &data, unsigned int *pos) {
  signed char c;
  if (!check_pos(data, pos[0] + 1)) return NULL;
  const char *tmp = data.constData();
  memcpy(&c, &tmp[pos[0] + 1], 1);
  pos[0] += 2;
  qDebug() << "---decode_char---" << pos[0] << c;
  return QVariant(c);
}

QVariant QtRencode::decode_short(const QByteArray &data, unsigned int *pos) {
  short s;
  if (!check_pos(data, pos[0] + 2)) return NULL;
  const char *tmp = data.constData();
  memcpy(&s, &tmp[pos[0] + 1], 2);
  pos[0] += 3;
  if (!BIG_ENDIAN) s = swap_byte_order_short((char *)(&s));
  qDebug() << "---decode_short---" << pos[0] << s;
  return QVariant(s);
}

QVariant QtRencode::decode_int(const QByteArray &data, unsigned int *pos) {
  int i;
  if (!check_pos(data, pos[0] + 4)) return NULL;
  const char *tmp = data.constData();
  memcpy(&i, &tmp[pos[0] + 1], 4);
  pos[0] += 5;
  if (!BIG_ENDIAN) i = swap_byte_order_int((char *)(&i));
  qDebug() << "---decode_int---" << i << pos;
  return QVariant(i);
}

QVariant QtRencode::decode_long_long(const QByteArray &data,
                                     unsigned int *pos) {
  long long l;
  if (!check_pos(data, pos[0] + 8)) return NULL;
  const char *tmp = data.constData();
  memcpy(&l, &tmp[pos[0] + 1], 8);
  pos[0] += 9;
  if (!BIG_ENDIAN) l = swap_byte_order_long_long((char *)(&l));
  qDebug() << "---decode_long_long---" << pos[0] << l;
  return QVariant(l);
}

QVariant QtRencode::decode_fixed_pos_int(const QByteArray &data,
                                         unsigned int *pos) {
  pos[0] += 1;
  int v = data.at(pos[0] - 1) - INT_POS_FIXED_START;
  qDebug() << "---decode_fixed_pos_int---" << pos[0] << v;
  return QVariant(v);
}

QVariant QtRencode::decode_fixed_neg_int(const QByteArray &data,
                                         unsigned int *pos) {
  pos[0] += 1;
  int v = (data.at(pos[0] - 1) - INT_NEG_FIXED_START + 1) * -1;
  qDebug() << "---decode_fixed_pos_int---" << pos[0] << v;
  return (v);
}

QVariant QtRencode::decode_big_number(const QByteArray &data,
                                      unsigned int *pos) {
  pos[0] += 1;
  int x = 18;
  if (!check_pos(data, pos[0] + x)) return NULL;
  while (data.at(pos[0] + x) != CHR_TERM) {
    x += 1;
    if (x >= MAX_INT_LENGTH) {
      qCritical() << "Number is longer than " << MAX_INT_LENGTH
                  << " characters";
      return NULL;
    }
    if (!check_pos(data, pos[0] + x)) return NULL;
  }
  //  big_number = int(data[pos[0]:pos[0]+x]);
  long long big_number = data.mid(pos[0], x).toLongLong();
  pos[0] += x + 1;
  qDebug() << "---decode_big_number---" << pos[0] << big_number;
  return QVariant(big_number);
}

QVariant QtRencode::decode_float32(const QByteArray &data, unsigned int *pos) {
  float f;
  if (!check_pos(data, pos[0] + 4)) return NULL;
  const char *tmp = data.constData();
  memcpy(&f, &tmp[pos[0] + 1], 4);
  pos[0] += 5;
  if (!BIG_ENDIAN) f = swap_byte_order_float((char *)(&f));
  qDebug() << "---decode_float32---" << pos[0] << f;
  return QVariant(f);
}

QVariant QtRencode::decode_float64(const QByteArray &data, unsigned int *pos) {
  double d;
  if (!check_pos(data, pos[0] + 8)) return NULL;
  const char *tmp = data.constData();
  memcpy(&d, &tmp[pos[0] + 1], 8);
  pos[0] += 9;
  if (!BIG_ENDIAN) d = swap_byte_order_double((char *)(&d));
  qDebug() << "---decode_float64---" << pos[0] << d;
  return QVariant(d);
}

QVariant QtRencode::decode_fixed_str(const QByteArray &data,
                                     unsigned int *pos) {
  unsigned char size = data.at(pos[0]) - STR_FIXED_START;
  if (!check_pos(data, pos[0] + size)) return NULL;
  //  s = data [pos[0] + 1:pos[0] + size];
  QByteArray s = data.mid(pos[0] + 1, size);
  pos[0] += size + 1;
  qDebug() << "---decode_fixed_str---" << pos[0] << s << USE_JSON;
  if (USE_JSON) return QTextCodec::codecForUtfText(s)->toUnicode(s);
  return QVariant(s);
}

QVariant QtRencode::decode_str(const QByteArray &data, unsigned int *pos) {
  unsigned int x = 1;
  if (!check_pos(data, pos[0] + x)) return NULL;
  while (data.at(pos[0] + x) != 58) {
    x += 1;
    if (!check_pos(data, pos[0] + x)) return NULL;
  }

  //  int size = int(data [pos[0]:pos[0] + x]);
  int size = data.mid(pos[0], x).toInt();
  pos[0] += x + 1;
  if (!check_pos(data, pos[0] + size - 1)) return NULL;
  //  s = data [pos[0]:pos[0] + size];
  QByteArray s = data.mid(pos[0], size);
  pos[0] += size;
  qDebug() << "---decode_str---" << pos[0] << s << USE_JSON;
  if (USE_JSON) return QTextCodec::codecForUtfText(s)->toUnicode(s);
  return QVariant(s);
}

QVariantList QtRencode::decode_fixed_list(const QByteArray &data,
                                          unsigned int *pos) {
  qDebug() << "---decode_fixed_list---" << pos[0];
  QVariantList l;
  unsigned char size = (unsigned char)data.at(pos[0]) - LIST_FIXED_START;
  pos[0] += 1;
  if (data.size() < size) return l;
  for (unsigned char i = 0; i < size; i++) l.append(decode(data, pos));
  qDebug() << "---decode_fixed_list---" << pos[0] << l;
  return l;
}

QVariantList QtRencode::decode_list(const QByteArray &data, unsigned int *pos) {
  qDebug() << "---decode_list---" << pos[0];
  QVariantList l;
  pos[0] += 1;
  while (data.at(pos[0]) != CHR_TERM) l.append(decode(data, pos));
  pos[0] += 1;
  qDebug() << "---decode_list---" << pos[0] << l;
  return l;
}

QVariant QtRencode::decode_fixed_dict(const QByteArray &data,
                                      unsigned int *pos) {
  qDebug() << "---decode_fixed_dict---" << pos[0];
  QVariantMap json_ret;
  QMap<QVariant, QVariant> map_ret;
  unsigned char size = (unsigned char)data.at(pos[0]) - DICT_FIXED_START;
  pos[0] += 1;
  for (unsigned char i = 0; i < size; i++) {
    if (USE_JSON) {
      QByteArray tmp = decode(data, pos).toByteArray();
      QString key = QTextCodec::codecForUtfText(tmp)->toUnicode(tmp);
      QVariant value = decode(data, pos);
      json_ret.insert(key, value);
    } else {
      QVariant key = decode(data, pos);
      QVariant value = decode(data, pos);
      map_ret.insert(key, value);
    }
  }
  if (USE_JSON) {
    qDebug() << "---decode_fixed_dict---" << pos[0] << json_ret;
    return json_ret;
  } else {
    qDebug() << "---decode_fixed_dict---" << pos[0] << map_ret;
    return QVariant::fromValue<QMap<QVariant, QVariant>>(map_ret);
  }
}

QVariant QtRencode::decode_dict(const QByteArray &data, unsigned int *pos) {
  qDebug() << "---decode_dict---" << pos[0];
  QVariantMap json_ret;
  QMap<QVariant, QVariant> map_ret;
  pos[0] += 1;
  if (!check_pos(data, pos[0])) return NULL;
  while (data.at(pos[0]) != CHR_TERM) {
    if (USE_JSON) {
      QByteArray tmp = decode(data, pos).toByteArray();
      QString key = QTextCodec::codecForUtfText(tmp)->toUnicode(tmp);
      QVariant value = decode(data, pos);
      json_ret.insert(key, value);
    } else {
      QVariant key = decode(data, pos);
      QVariant value = decode(data, pos);
      map_ret.insert(key, value);
    }
  }
  pos[0] += 1;
  if (USE_JSON) {
    qDebug() << "---decode_fixed_dict---" << pos[0] << json_ret;
    return json_ret;
  } else {
    qDebug() << "---decode_fixed_dict---" << pos[0] << map_ret;
    return QVariant::fromValue<QMap<QVariant, QVariant>>(map_ret);
  }
}

QVariant QtRencode::decode(const QByteArray &data, unsigned int *pos) {
  if (pos[0] >= (unsigned int)data.size()) {
    qCritical() << "Malformed rencoded string: data_length: " << data.size()
                << " pos: " << pos[0];
    return NULL;
  }
  unsigned char typecode = data[pos[0]];
  if (typecode == CHR_INT1)
    return decode_char(data, pos);
  else if (typecode == CHR_INT2)
    return decode_short(data, pos);
  else if (typecode == CHR_INT4)
    return decode_int(data, pos);
  else if (typecode == CHR_INT8)
    return decode_long_long(data, pos);
  else if (INT_POS_FIXED_START <= typecode &&
           typecode < INT_POS_FIXED_START + INT_POS_FIXED_COUNT)
    return decode_fixed_pos_int(data, pos);
  else if (INT_NEG_FIXED_START <= typecode &&
           typecode < INT_NEG_FIXED_START + INT_NEG_FIXED_COUNT)
    return decode_fixed_neg_int(data, pos);
  else if (typecode == CHR_INT &&
           data.indexOf(CHR_TERM, (int)pos[0]) > (int)pos[0])
    return decode_big_number(data, pos);
  else if (typecode == CHR_FLOAT32)
    return decode_float32(data, pos);
  else if (typecode == CHR_FLOAT64)
    return decode_float64(data, pos);
  else if (STR_FIXED_START <= typecode &&
           typecode < STR_FIXED_START + STR_FIXED_COUNT)
    return decode_fixed_str(data, pos);
  else if (49 <= typecode && typecode <= 57 &&
           data.indexOf(":", (int)pos[0]) > (int)pos[0])
    return decode_str(data, pos);
  else if (typecode == CHR_NONE) {
    pos[0] += 1;
    return QVariant();
  } else if (typecode == CHR_TRUE) {
    pos[0] += 1;
    return true;
  } else if (typecode == CHR_FALSE) {
    pos[0] += 1;
    return false;
  } else if (LIST_FIXED_START <= typecode &&
             typecode <= LIST_FIXED_START + LIST_FIXED_COUNT - 1)
    return decode_fixed_list(data, pos);
  else if (typecode == CHR_LIST &&
           data.indexOf(CHR_TERM, (int)pos[0]) > (int)pos[0])
    return decode_list(data, pos);
  else if (DICT_FIXED_START <= typecode &&
           typecode < DICT_FIXED_START + DICT_FIXED_COUNT)
    return decode_fixed_dict(data, pos);
  else if (typecode == CHR_DICT &&
           data.indexOf(CHR_TERM, (int)pos[0]) > (int)pos[0])
    return decode_dict(data, pos);
  qCritical() << "Unknow typecode: " << typecode;
  return NULL;
}

void dumps(QByteArray &out, const QVariant &data, int bits) {
  out.clear();
  out.append(QtRencode::dumps(data, bits));
}

void loads(QVariant &out, const QByteArray &data, bool json) {
  out.clear();
  out.setValue(QtRencode::loads(data, json));
}
