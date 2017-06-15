#include <QString>
#include <QSharedPointer>
#include <QtTest>
#include <QtDebug>

#include <rxcpp/rx.hpp>
#include "benchmark.h"

namespace rxo = rxcpp::operators;

const int kNumbSamples = 1000;
class SampleProcessor
{
    int m_counter;
public:
    SampleProcessor() : m_counter(0) {}
    QString doWork(const QString& input) {
        QString r = input;
        for (int i = 0; i < 1000; ++i)
            r += QString::number(i);
        return r;
    }
};

void TestBenchmark::testSingleThreadedFrameProcessing()
{
    std::vector<QSharedPointer<SampleProcessor>> list;
    for (int i = 1; i <= 10; ++i)
        list.push_back( QSharedPointer<SampleProcessor>(new SampleProcessor ) );

    auto process = [](QString text, QSharedPointer<SampleProcessor> processor) {
        return rxcpp::observable<>::create<QString>([text, processor](const rxcpp::subscriber<QString>& s){
            auto r = processor->doWork(text);
            s.on_next( r );
            s.on_completed();
        });
    };

    auto input = rxcpp::observable<>::range(1, kNumbSamples);

    auto process_all = [=](const QString& text) {
        auto all = rxcpp::sources::iterate(list).
                   map([=](QSharedPointer<SampleProcessor> processor) {
                       return process(text, processor);
                   });
        return all.as_dynamic() | rxo::merge();
    };

    auto x = input
     | rxo::concat_map([=](int x) {
           return process_all(QString::number(x));
       });

    QBENCHMARK_ONCE {
        x.
          subscribe([](QString) {
        }, [&]() {
        });
    }
}

void TestBenchmark::testMultiThreadedFrameProcessing()
{
    std::vector<QSharedPointer<SampleProcessor>> list;
    for (int i = 1; i <= 10; ++i)
        list.push_back( QSharedPointer<SampleProcessor>(new SampleProcessor ) );

    auto thread = rxcpp::observe_on_new_thread();

    auto process = [](QString text, QSharedPointer<SampleProcessor> processor) {
        return rxcpp::observable<>::create<QString>([text, processor](const rxcpp::subscriber<QString>& s){
            auto r = processor->doWork(text);
            s.on_next( r );
            s.on_completed();
        });
    };

    auto input = rxcpp::observable<>::range(1, kNumbSamples).
                 subscribe_on(rxcpp::observe_on_new_thread());

    auto process_all = [=](const QString& text) {
        auto all = rxcpp::sources::iterate(list).
                   map([=](QSharedPointer<SampleProcessor> processor) {
                       return process(text, processor).
                              subscribe_on(thread).
                              as_dynamic();
                   });
        return all | rxo::merge(thread);
    };

    auto x = input
     | rxo::concat_map([=](int x) {
           return process_all(QString::number(x));
       });

    QBENCHMARK_ONCE {
        x.observe_on(rxcpp::observe_on_new_thread()).
          as_blocking().
          subscribe([](QString) {
          }, [&]() {
          });
    }
}

void TestBenchmark::testEventLoopFrameProcessing()
{
    std::vector<QSharedPointer<SampleProcessor>> list;
    for (int i = 1; i <= 10; ++i)
        list.push_back( QSharedPointer<SampleProcessor>(new SampleProcessor ) );

    auto thread = rxcpp::observe_on_event_loop();

    auto process = [](QString text, QSharedPointer<SampleProcessor> processor) {
        return rxcpp::observable<>::create<QString>([text, processor](const rxcpp::subscriber<QString>& s){
            auto r = processor->doWork(text);
            s.on_next( r );
            s.on_completed();
        });
    };

    auto input = rxcpp::observable<>::range(1, kNumbSamples).
                 subscribe_on(rxcpp::observe_on_new_thread());

    auto process_all = [=](const QString& text) {
        auto all = rxcpp::sources::iterate(list).
                   map([=](QSharedPointer<SampleProcessor> processor) {
                       return process(text, processor).
                       subscribe_on(thread).
                       as_dynamic();
                   });
        return all | rxo::merge(thread);
    };

    auto x = input
     | rxo::concat_map([=](int x) {
           return process_all(QString::number(x));
       });

    QBENCHMARK_ONCE {
        x.observe_on(rxcpp::observe_on_new_thread()).
          as_blocking().
          subscribe([](QString) {
          }, [&]() {
          });
    }
}

QTEST_GUILESS_MAIN(TestBenchmark)
