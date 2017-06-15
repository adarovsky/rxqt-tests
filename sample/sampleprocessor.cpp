#include <QThread>
#include "sampleprocessor.h"

SampleProcessor::SampleProcessor(const QString & name, QSharedPointer<std::random_device> random)
    : m_counter( 0 ), m_name(name), m_random(random), m_dist(0, 20)
{

}

QString SampleProcessor::doWork(const QString& input)
{
    m_counter++;
    auto p = 100 + m_dist(*m_random);
    QThread::currentThread()->msleep(p);
    return QString("0x%4: %1: %2-%3").arg(m_name, input,
                                                    QString::number(m_counter),
                                                    QString::number((uint64_t)QThread::currentThreadId(), 16));
}
