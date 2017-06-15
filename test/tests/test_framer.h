#ifndef BENCHMARK_H
#define BENCHMARK_H
#include <QObject>

class TestFramer: public QObject
{
    Q_OBJECT
private slots:
    void testSinglePacket();
    void test2Packets();
    void test2SplitPackets();
};

#endif // BENCHMARK_H
