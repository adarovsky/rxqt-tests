#ifndef BENCHMARK_H
#define BENCHMARK_H
#include <QObject>

class TestBenchmark: public QObject
{
    Q_OBJECT
private slots:
    void testSingleThreadedFrameProcessing();
    void testMultiThreadedFrameProcessing();
    void testEventLoopFrameProcessing();
};

#endif // BENCHMARK_H
