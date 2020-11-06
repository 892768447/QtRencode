#include <QtTest>
#include "qtrencode.h"

class TestQtRencode : public QObject {
  Q_OBJECT

 public:
  TestQtRencode();
  ~TestQtRencode();

 private slots:
  void test_case1();
};

TestQtRencode::TestQtRencode() {}

TestQtRencode::~TestQtRencode() {}

void TestQtRencode::test_case1() {
  QByteArray data(
      "[\"configure-window\",1,242,265,667.2,471,{\"name\":\"irony\"},0,{\"1\":"
      "2},["
      "false,true],1,[1367,281],[]]");

  qInfo() << QtRencode::dumps(QVariant(2));
  QMap<QVariant, QVariant> intjson;
  intjson.insert(1, 2);
  QByteArray intret = QtRencode::dumps(QVariant::fromValue(intjson));
  qInfo() << intret;
  QVariant inttmp = QtRencode::loads(intret, false);
  qInfo() << inttmp.value<QMap<QVariant, QVariant>>().keys();

  QByteArray result = QtRencode::dumps(data);
  qInfo() << result << result.size();

  QVariant tmp = QtRencode::loads(result);
  qInfo() << tmp;
  QJsonDocument doc(tmp.toJsonArray());
  qInfo() << doc.toJson(QJsonDocument::Compact);

  qInfo() << QtRencode::loads(QByteArray(">"), false);
  qInfo() << QtRencode::loads(QByteArray("E"), false);

  QByteArray str;
  str.fill('f', 255);
  qInfo() << str;
  qInfo() << QtRencode::dumps(QVariant(str));
  qInfo() << QtRencode::loads(
      QByteArray(
          "255:"
          "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff"
          "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff"
          "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff"
          "fffffffffffffffffffffffffffffffffffffffffffffffffff"),
      false);

  qInfo() << QtRencode::dumps(QVariant(QByteArray("\x00")));

  qInfo() << QVariant(QByteArray("aa"));

  qInfo() << QtRencode::loads(QByteArray(";"), false);
}

QTEST_APPLESS_MAIN(TestQtRencode)

#include "tst_testqtrencode.moc"
