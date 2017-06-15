#include <QCoreApplication>
#include <QTime>
#include <QThread>
#include <rxqt.hpp>
#include <rx-drop_map.hpp>
#include <rxcpp/rx.hpp>
#include "sampleprocessor.h"

QSharedPointer<std::random_device> rd;

namespace rxo = rxcpp::operators;

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    rd.reset(new std::random_device);
    auto thread = rxcpp::observe_on_event_loop();

    std::vector<QSharedPointer<SampleProcessor>> list;
    for (int i = 1; i <= 10; ++i)
        list.push_back( QSharedPointer<SampleProcessor>(new SampleProcessor(QString("proc-").append(QString::number(i)), rd)) );

    auto process = [](auto text, QSharedPointer<SampleProcessor> processor) {
        qDebug().noquote() << QThread::currentThreadId() << ":" << processor->name() << "- making observable for:" << text;
        QTime time; time.start();
        return rxcpp::observable<>::create<QString>([text, processor, time](const rxcpp::subscriber<QString>& s){
            auto r = processor->doWork(text);
            s.on_next( r + " - execution time: " + QString::number(time.elapsed()) + " msec" );
            s.on_completed();
        });
    };

    auto input = rxcpp::observable<>::interval(std::chrono::steady_clock::now(), std::chrono::milliseconds(50)).
                 take(20).
                 subscribe_on(thread);

    auto process_all = [list, process, thread](const QString& text) {
        auto all = rxcpp::sources::iterate(list).
                   map([=](auto processor) {
                       return process(text, processor).
                       subscribe_on(thread);
                   });
        return all.as_dynamic() | rxo::merge();
    };

    auto x = input
     | rxo::drop_map([=](auto x) {
           QTime time; time.start();
           return process_all(QString::number(x)).tap([](auto) {}, [x, time]() {
               qDebug() << QThread::currentThreadId() << ": frame" << x << "processed in" << time.elapsed() << "msec";
           });
       });

    QTime started; started.start();
    x.observe_on(rxcpp::observe_on_qt_event_loop()).
      subscribe([](QString x) {
        qDebug() << QThread::currentThreadId() << ":" << x;
    }, [&]() {
        qDebug() << QThread::currentThreadId() << ": completed in" << started.elapsed() << "msec";
        app.quit();
    });

    qDebug() << QThread::currentThreadId() << "started";
    return app.exec();
}
