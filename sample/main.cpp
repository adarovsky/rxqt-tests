#include <QCoreApplication>
#include <QTime>
#include <QThread>
#include <QtDebug>
#include <rx-drop_map.hpp>
#include <rxqt-eventloop.hpp>
#include <rxcpp/rx.hpp>
#include "sampleprocessor.h"

QSharedPointer<std::random_device> rd;

namespace rxo = rxcpp::operators;
typedef QSharedPointer<SampleProcessor> ProcRef;

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    rd.reset(new std::random_device);
    auto thread = rxcpp::observe_on_event_loop();

    std::vector<ProcRef> list;
    for (int i = 1; i <= 20; ++i)
        list.push_back( ProcRef(new SampleProcessor(QString("proc-").append(QString::number(i)), rd)) );

    auto process = [](QString text, ProcRef processor) {
        qDebug().noquote() << QThread::currentThreadId() << ":" << processor->name() << "- making observable for:" << text;
        QTime time; time.start();
        return rxcpp::observable<>::create<QString>([text, processor, time](const rxcpp::subscriber<QString>& s){
            QTime ctime; ctime.start();
            auto r = processor->doWork(text);
            s.on_next( r + " - execution time (with queue): " + QString::number(time.elapsed()) + " msec, clean: " + QString::number(ctime.elapsed()) + " msec");
            s.on_completed();
        });
    };

    auto input = rxcpp::observable<>::interval(std::chrono::steady_clock::now(), std::chrono::milliseconds(50)).
                 take(20).
                 subscribe_on(thread);

    auto process_all = [list, process, thread](const QString& text) {
        auto all = rxcpp::sources::iterate(list).
                   map([=](ProcRef processor) {
                       return process(text, processor).
                       subscribe_on(thread);
                   });
        return all.as_dynamic() | rxo::merge();
    };

    auto x = input
     | rxo::drop_map([=](int x) {
           QTime time; time.start();
           return process_all(QString::number(x)).tap([](QString) {}, [x, time]() {
               qDebug() << QThread::currentThreadId() << ": frame" << x << "processed in" << time.elapsed() << "msec";
           });
       });

    QTime started; started.start();
    x.observe_on(rxcpp::observe_on_qt_event_loop()).
      subscribe([](QString x) {
        qDebug() << QThread::currentThreadId() << ": - " << x;
    }, [&]() {
        qDebug() << QThread::currentThreadId() << ": completed in" << started.elapsed() << "msec";
        app.quit();
    });

    qDebug() << QThread::currentThreadId() << "started";
    return app.exec();
}
