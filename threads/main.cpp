#include <QCoreApplication>
#include <QTime>
#include <QThread>
#include <QtDebug>
#include <rx-drop_map.hpp>
#include <rxqt.hpp>
#include <rxqt-eventloop.hpp>
#include <rxcpp/rx.hpp>

namespace rxo = rxcpp::operators;

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    auto thread = rxcpp::observe_on_event_loop();
    auto main_thread = rxcpp::observe_on_qt_event_loop();

    auto process = [](QString string) {
        return rxcpp::observable<>::create<QString>([=](const rxcpp::subscriber<QString>& s){
            qDebug() << QThread::currentThreadId() << ": sending next:" << string;
            s.on_next(string);
            s.on_completed();
        });
    };

    auto input = rxcpp::observable<>::range(1, 400);

    auto process_all = [&](int i) {
        qDebug() << QThread::currentThreadId() << ": handling top sequence" << i;
        auto all = rxcpp::sources::range(1, 300).
                   map([=](int j) {
                       return process(QString("%1 - %2").arg(QString::number(i), QString::number(j))).
                               subscribe_on(thread).
                               as_dynamic();
                   });
        return all | rxo::merge(thread);
    };

    (input
     | rxo::concat_map(process_all)
     | rxo::observe_on(main_thread)
     | rxo::as_dynamic())
     .subscribe([](auto x) {
           qDebug() << QThread::currentThreadId() << ": - " << x;
       }, [&]() {
           qDebug() << QThread::currentThreadId() << ": completed";
           QTimer::singleShot(500, &app, SLOT(quit()));
       });

    qDebug() << QThread::currentThreadId() << "started";
    return app.exec();
}
