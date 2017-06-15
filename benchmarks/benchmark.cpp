#include <QString>
#include <QSharedPointer>
#include <QtTest>
#include <QtDebug>

#include <rxqt.hpp>
#include <rx-drop_map.hpp>
#include <rxcpp/rx.hpp>
#include <range/v3/all.hpp>
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

    auto process = [](auto text, QSharedPointer<SampleProcessor> processor) {
        return rxcpp::observable<>::create<QString>([text, processor](const rxcpp::subscriber<QString>& s){
            auto r = processor->doWork(text);
            s.on_next( r );
            s.on_completed();
        });
    };

    auto input = rxcpp::observable<>::range(1, kNumbSamples);

    auto process_all = [list, process](const QString& text) {
        auto all = list | ranges::view::transform([process, text](QSharedPointer<SampleProcessor> processor) {
            return process(text, processor);
        });
        return rxcpp::sources::iterate(all) | rxo::merge();
    };

    auto x = input
     | rxo::concat_map([=](auto x) {
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

    auto process = [](auto text, QSharedPointer<SampleProcessor> processor) {
        return rxcpp::observable<>::create<QString>([text, processor](const rxcpp::subscriber<QString>& s){
            auto r = processor->doWork(text);
            s.on_next( r );
            s.on_completed();
        });
    };

    auto input = rxcpp::observable<>::range(1, kNumbSamples).
                 subscribe_on(rxcpp::observe_on_new_thread());

    auto process_all = [list, process, thread](const QString& text) {
        auto all = list | ranges::view::transform([process, text, thread](QSharedPointer<SampleProcessor> processor) {
            return process(text, processor).subscribe_on(thread).as_dynamic();
        });
        return rxcpp::sources::iterate(all).merge(thread);
    };

    auto x = input
     | rxo::concat_map([=](auto x) {
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

    auto process = [](auto text, QSharedPointer<SampleProcessor> processor) {
        return rxcpp::observable<>::create<QString>([text, processor](const rxcpp::subscriber<QString>& s){
            auto r = processor->doWork(text);
            s.on_next( r );
            s.on_completed();
        });
    };

    auto input = rxcpp::observable<>::range(1, kNumbSamples).
                 subscribe_on(rxcpp::observe_on_new_thread());

    auto process_all = [list, process, thread](const QString& text) {
        auto all = list | ranges::view::transform([process, text, thread](QSharedPointer<SampleProcessor> processor) {
            return process(text, processor).subscribe_on(thread).as_dynamic();
        });
        return rxcpp::sources::iterate(all).merge(thread);
    };

    auto x = input
     | rxo::concat_map([=](auto x) {
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