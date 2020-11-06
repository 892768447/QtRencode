#include "qtrencode.h"

/**
 * @brief QtRencode::dumps
 * @param data
 * @return QByteArray
 * 对原始json数据编码
 */
QByteArray QtRencode::dumps(const QByteArray &data) {
  QJsonParseError error;
  QJsonDocument json = QJsonDocument::fromJson(data, &error);
  if (error.error != QJsonParseError::NoError) {
    qWarning() << error.errorString();
    return QByteArray();
  }
  return dumps(json);
}

/**
 * @brief QtRencode::dumps
 * @param data
 * @return QByteArray
 * 对josn数据编码
 */
QByteArray QtRencode::dumps(const QJsonDocument &data) {
  QByteArray out;
  encode(&out, data.toVariant());
  return out;
}

/**
 * @brief QtRencode::loads
 * @param data
 * @return QVariant
 * 编码后的数据解码为json
 */
QVariant QtRencode::loads(const QByteArray &data) {
  quint32 pos = 0;
  return decode(data, &pos);
}

void QtRencode::encode(QByteArray *out, const QVariant &value) {
  if (value.type() == QVariant::List)
    encodeList(out, value);
  else if (value.type() == QVariant::Map)
    encodeMap(out, value);
  else if (value.type() == QVariant::Bool)
    encodeBool(out, value);
  else if (value.type() == QVariant::String ||
           value.type() == QVariant::ByteArray)
    encodeByteArray(out, value);
  else if (value.isNull())
    encodeNone(out);
  else if (value.type() == QVariant::Double)
    encodeDouble(out, value);
  else if (value.canConvert(QVariant::LongLong))
    // char short int float double long longlong
    encodeInt(out, value);
  else
    qWarning() << "Unknow type:" << value.typeName();
}

void QtRencode::encodeInt(QByteArray *out, const QVariant &value) {
  //  qDebug() << "===encodeInt===" << value;
  qint64 data = value.toLongLong();
  if (0 <= data && data < INT_POS_FIXED_COUNT)
    out->append(INT_POS_FIXED_START + value.value<qint8>());
  else if (-INT_NEG_FIXED_COUNT <= data && data < 0)
    out->append(INT_NEG_FIXED_START - 1 - value.value<qint8>());
  else if (-128 <= data && data < 128) {
    out->append(CHR_INT1);
    QByteArray b_tmp;
    QDataStream d_tmp(&b_tmp, QIODevice::WriteOnly);
    d_tmp << value.value<qint8>();
    out->append(b_tmp);
  } else if (-32768 <= data && data < 32768) {
    //  qDebug() << "---encodeShort---";
    out->append(CHR_INT2);
    QByteArray b_tmp;
    QDataStream d_tmp(&b_tmp, QIODevice::WriteOnly);
    d_tmp << value.value<qint16>();
    out->append(b_tmp);
  } else if (MIN_SIGNED_INT <= data && data < MAX_SIGNED_INT) {
    //  qDebug() << "---encodeInt---";
    out->append(CHR_INT4);
    QByteArray b_tmp;
    QDataStream d_tmp(&b_tmp, QIODevice::WriteOnly);
    d_tmp << value.value<qint32>();
    out->append(b_tmp);
  } else if (MIN_SIGNED_LONGLONG <= data && data < MAX_SIGNED_LONGLONG) {
    //  qDebug() << "---encodeLongLong---";
    out->append(CHR_INT8);
    QByteArray b_tmp;
    QDataStream d_tmp(&b_tmp, QIODevice::WriteOnly);
    d_tmp << value.value<qint64>();
    out->append(b_tmp);
  } else {
    QByteArray s = value.toByteArray();
    Q_ASSERT(s.size() < MAX_INT_LENGTH);
    out->append(CHR_INT);
    out->append(s);
    out->append(CHR_TERM);
  }
}

void QtRencode::encodeDouble(QByteArray *out, const QVariant &value) {
  //  qDebug() << "===encodeDouble===" << value;
  qint64 data = value.toLongLong();
  QByteArray b_tmp;
  QDataStream d_tmp(&b_tmp, QIODevice::WriteOnly);
  d_tmp.setFloatingPointPrecision(QDataStream::SinglePrecision);
  //  d_tmp.setVersion(QDataStream::Qt_4_5);

  if (MIN_SIGNED_INT <= data && data < MAX_SIGNED_INT) {
    out->append(CHR_FLOAT32);
    d_tmp << value.toFloat();
    out->append(b_tmp);
  } else if (MIN_SIGNED_LONGLONG <= data && data < MAX_SIGNED_LONGLONG) {
    out->append(CHR_FLOAT64);
    d_tmp << value.toDouble();
    out->append(b_tmp);
  }
}

void QtRencode::encodeByteArray(QByteArray *out, const QVariant &value) {
  //  qDebug() << "===encodeByteArray===" << value;
  const QByteArray data = value.toByteArray();
  if (data.size() < STR_FIXED_COUNT)
    out->append(STR_FIXED_START + data.size());
  else {
    out->append(QString::number(data.size()));
    out->append(":");
  }
  out->append(data);
}

void QtRencode::encodeBool(QByteArray *out, const QVariant &value) {
  //  qDebug() << "===encodeBool===" << value;
  out->append(value.toBool() ? CHR_TRUE : CHR_FALSE);
}

void QtRencode::encodeNone(QByteArray *out) {
  //  qDebug() << "===encodeNone===";
  out->append(CHR_NONE);
}

void QtRencode::encodeList(QByteArray *out, const QVariant &value) {
  //  qDebug() << "===encodeList===" << value;
  const QList<QVariant> list = value.toList();
  if (list.size() < LIST_FIXED_COUNT) {
    out->append(LIST_FIXED_START + list.size());
    for (auto it = list.begin(); it != list.end(); it++) encode(out, *it);
  } else {
    out->append(CHR_LIST);
    for (auto it = list.begin(); it != list.end(); it++) encode(out, *it);
    out->append(CHR_TERM);
  }
}

void QtRencode::encodeMap(QByteArray *out, const QVariant &value) {
  //  qDebug() << "===encodeMap===" << value;
  const QMap<QString, QVariant> map = value.toMap();
  if (map.size() < DICT_FIXED_COUNT) {
    out->append(DICT_FIXED_START + map.size());
    for (auto it = map.begin(); it != map.end(); it++) {
      encode(out, it.key());
      encode(out, it.value());
    }
  } else {
    out->append(CHR_DICT);
    for (auto it = map.begin(); it != map.end(); it++) {
      encode(out, it.key());
      encode(out, it.value());
    }
    out->append(CHR_TERM);
  }
}

QVariant QtRencode::decode(const QByteArray &in, quint32 *pos) {
  quint8 code = in.at(pos[0]);
  switch (code) {
    case CHR_INT1:
      return decodeChar(in, pos);
    case CHR_INT2:
      return decodeShort(in, pos);
    case CHR_INT4:
      return decodeInt(in, pos);
    case CHR_INT8:
      return decodeLongLong(in, pos);
    case CHR_INT:
      return decodeBigNumber(in, pos);
    case CHR_FLOAT32:
      return decodeFloat(in, pos);
    case CHR_FLOAT64:
      return decodeDouble(in, pos);
    case CHR_NONE:
      pos[0] += 1;
      return NULL;
    case CHR_TRUE:
      pos[0] += 1;
      return true;
    case CHR_FALSE:
      pos[0] += 1;
      return false;
    case CHR_LIST:
      return decodeList(in, pos);
    case CHR_DICT:
      return decodeDict(in, pos);
    default:
      if (INT_POS_FIXED_START <= code &&
          code < INT_POS_FIXED_START + INT_POS_FIXED_COUNT)
        return decodeFixedPosInt(in, pos);
      else if (INT_NEG_FIXED_START <= code &&
               code < INT_NEG_FIXED_START + INT_NEG_FIXED_COUNT)
        return decodeFixedNegInt(in, pos);
      else if (STR_FIXED_START <= code &&
               code < STR_FIXED_START + STR_FIXED_COUNT)
        return decodeFixeByteArray(in, pos);
      else if (49 <= code && code <= 57)
        return decodeByteArray(in, pos);
      else if (LIST_FIXED_START <= code &&
               code <= LIST_FIXED_START + LIST_FIXED_COUNT - 1)
        return decodeFixedList(in, pos);
      else if (DICT_FIXED_START <= code &&
               code < DICT_FIXED_START + DICT_FIXED_COUNT)
        return decodeFixedDict(in, pos);
      break;
  }
  return NULL;
}

QVariant QtRencode::decodeChar(const QByteArray &in, quint32 *pos) {
  QVariant value(qFromBigEndian<qint8>(in.mid(pos[0] + 1, 1)));
  pos[0] += 2;
  return value;
}

QVariant QtRencode::decodeShort(const QByteArray &in, quint32 *pos) {
  QVariant value(qFromBigEndian<qint16>(in.mid(pos[0] + 1, 2)));
  pos[0] += 3;
  return value;
}

QVariant QtRencode::decodeInt(const QByteArray &in, quint32 *pos) {
  QVariant value(qFromBigEndian<qint32>(in.mid(pos[0] + 1, 4)));
  pos[0] += 5;
  return value;
}

QVariant QtRencode::decodeLongLong(const QByteArray &in, quint32 *pos) {
  QVariant value(qFromBigEndian<qint64>(in.mid(pos[0] + 1, 8)));
  pos[0] += 9;
  return value;
}

QVariant QtRencode::decodeBigNumber(const QByteArray &in, quint32 *pos) {
  const int index = in.indexOf(CHR_TERM, pos[0] + 1);
  const QByteArray byte = in.mid(pos[0] + 1, index - pos[0]);
  QVariant value(byte);
  pos[0] += 1 + byte.size();
  return value;
}

QVariant QtRencode::decodeFloat(const QByteArray &in, quint32 *pos) {
  QVariant value(qFromBigEndian<float>(in.mid(pos[0] + 1, 4)));
  pos[0] += 5;
  return value;
}

QVariant QtRencode::decodeDouble(const QByteArray &in, quint32 *pos) {
  QVariant value(qFromBigEndian<double>(in.mid(pos[0] + 1, 8)));
  pos[0] += 9;
  return value;
}

QVariant QtRencode::decodeList(const QByteArray &in, quint32 *pos) {
  QVariantList value;
  pos[0] += 1;
  while (in.at(pos[0]) != CHR_TERM) value.append(decode(in, pos));
  pos[0] += 1;
  return value;
}

QVariant QtRencode::decodeDict(const QByteArray &in, quint32 *pos) {
  QVariantMap value;
  pos[0] += 1;
  while (in.at(pos[0]) != CHR_TERM) {
    value.insert(decode(in, pos).toString(), decode(in, pos));
  }
  pos[0] += 1;
  return value;
}

QVariant QtRencode::decodeFixedPosInt(const QByteArray &in, quint32 *pos) {
  pos[0] += 1;
  return QVariant(qFromBigEndian<qint8>(in.at(pos[0] - 1)) -
                  INT_POS_FIXED_START);
}

QVariant QtRencode::decodeFixedNegInt(const QByteArray &in, quint32 *pos) {
  pos[0] += 1;
  return QVariant(
      (qFromBigEndian<qint8>(in.at(pos[0] - 1)) - INT_POS_FIXED_START + 1) *
      -1);
}

QVariant QtRencode::decodeFixeByteArray(const QByteArray &in, quint32 *pos) {
  quint8 size = in.at(pos[0]) - STR_FIXED_START;
  QVariant value(in.mid(pos[0] + 1, size));
  pos[0] += size + 1;
  return value;
}

QVariant QtRencode::decodeByteArray(const QByteArray &in, quint32 *pos) {
  const int index = in.indexOf(':', pos[0]);
  const QByteArray byte = in.mid(pos[0] + 1, index - pos[0]);
  QVariant value(byte);
  pos[0] += 1 + byte.size();
  return value;
}

QVariant QtRencode::decodeFixedList(const QByteArray &in, quint32 *pos) {
  QVariantList value;
  quint8 size = in.at(pos[0]) - LIST_FIXED_START;
  pos[0] += 1;
  for (quint8 i = 0; i < size; i++) value.append(decode(in, pos));
  return value;
}

QVariant QtRencode::decodeFixedDict(const QByteArray &in, quint32 *pos) {
  QVariantMap value;
  quint8 size = in.at(pos[0]) - DICT_FIXED_START;
  pos[0] += 1;
  for (quint8 i = 0; i < size; i++) {
    value.insert(decode(in, pos).toString(), decode(in, pos));
  }
  return value;
}
