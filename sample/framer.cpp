#include <QDataStream>
#include <QtEndian>
#include "framer.hpp"

void framer::enqueue(const QByteArray& data)
{
    buffer.append(data);
}

QByteArray framer::popFrame()
{
    if (packetLength < 0 && buffer.length() >= (int)sizeof(int32_t)) {
        packetLength = qFromBigEndian<int32_t>(buffer.data());
    }

    if (packetLength > 0 && buffer.length() >= packetLength + (int)sizeof(int32_t)) {
        QByteArray container = buffer.mid((int)sizeof(int32_t), packetLength);
        buffer.remove(0, packetLength + (int)sizeof(int32_t));
        packetLength = -1;
        return container;
    }

    return QByteArray();
}

rxcpp::observable<QByteArray> framer::operator() (const QByteArray& buf)
{
    enqueue(buf);
    return rxcpp::observable<>::create<QByteArray>([this](rxcpp::subscriber<QByteArray> s) {
        QByteArray result = popFrame();
        while( !result.isEmpty() ) {
            s.on_next( result );
            result = popFrame();
        }
        s.on_completed();
    });
}
