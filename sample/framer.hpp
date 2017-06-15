#ifndef FRAMER_HPP
#define FRAMER_HPP
#include <QByteArray>
#include <rxcpp/rx.hpp>

class framer {
    QByteArray buffer;
    int packetLength;
    void enqueue(const QByteArray& data);
    QByteArray popFrame();
public:
    framer(): packetLength(-1) {}
    rxcpp::observable<QByteArray> operator() (const QByteArray& buf);
};

#endif // FRAMER_HPP
