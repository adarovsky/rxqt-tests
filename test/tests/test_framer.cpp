#include <QString>
#include <QSharedPointer>
#include <QDataStream>
#include <QtTest>
#include <QtDebug>

#include <rxcpp/rx.hpp>
#include <rxcpp/rx-test.hpp>
#include "framer.hpp"
#include "test_framer.h"

namespace rx=rxcpp;
namespace rxu=rxcpp::util;
namespace rxs=rxcpp::sources;
namespace rxo=rxcpp::operators;
namespace rxsub=rxcpp::subjects;
namespace rxsc=rxcpp::schedulers;
namespace rxn=rx::notifications;
namespace rxt = rxcpp::test;

void TestFramer::testSinglePacket()
{
    auto sc = rxsc::make_test();
    auto w = sc.create_worker();
    const rxsc::test::messages<QByteArray> on;
    const rxsc::test::messages<QByteArray> on_out;

    QByteArray payload;
    QDataStream ds(&payload, QIODevice::WriteOnly);
    QByteArray message("asd");
    ds << message;

    auto xs = sc.make_hot_observable({
        on.next(210, payload),
        on.completed(300)
    });

    framer f;

    auto res = w.start(
        [xs, &f]() {
            return xs
                | rxo::concat_map(f)
                | rxo::as_dynamic(); // forget type to workaround lambda deduction bug on msvc 2013
        }
    );

    auto required = rxu::to_vector({
        on_out.next(210, message),
        on_out.completed(300)
    });
    auto actual = res.get_observer().messages();
    QTEST_ASSERT(required == actual);
}

void TestFramer::test2Packets()
{
    auto sc = rxsc::make_test();
    auto w = sc.create_worker();
    const rxsc::test::messages<QByteArray> on;
    const rxsc::test::messages<QByteArray> on_out;

    QByteArray payload;
    QDataStream ds(&payload, QIODevice::WriteOnly);
    QByteArray message1("asd");
    ds << message1;
    QByteArray message2("123");
    ds << message2;

    auto xs = sc.make_hot_observable({
        on.next(210, payload),
        on.completed(300)
    });

    framer f;

    auto res = w.start(
        [xs, &f]() {
            return xs
                | rxo::concat_map(f)
                | rxo::as_dynamic(); // forget type to workaround lambda deduction bug on msvc 2013
        }
    );

    auto required = rxu::to_vector({
        on_out.next(210, message1),
        on_out.next(210, message2),
        on_out.completed(300)
    });
    auto actual = res.get_observer().messages();
    QTEST_ASSERT(required == actual);
}

void TestFramer::test2SplitPackets()
{
    auto sc = rxsc::make_test();
    auto w = sc.create_worker();
    const rxsc::test::messages<QByteArray> on;
    const rxsc::test::messages<QByteArray> on_out;

    QByteArray payload;
    QDataStream ds(&payload, QIODevice::WriteOnly);
    QByteArray message1("asd");
    ds << message1;
    QByteArray message2("123");
    ds << message2;

    auto xs = sc.make_hot_observable({
        on.next(210, payload.left(2)),
        on.next(220, payload.mid(2)),
        on.completed(300)
    });

    framer f;

    auto res = w.start(
        [xs, &f]() {
            return xs
                | rxo::concat_map(f)
                | rxo::as_dynamic(); // forget type to workaround lambda deduction bug on msvc 2013
        }
    );

    auto required = rxu::to_vector({
        on_out.next(220, message1),
        on_out.next(220, message2),
        on_out.completed(300)
    });
    auto actual = res.get_observer().messages();
    QTEST_ASSERT(required == actual);
}

QTEST_GUILESS_MAIN(TestFramer)
