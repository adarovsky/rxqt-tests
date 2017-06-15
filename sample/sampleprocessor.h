#ifndef SAMPLEPROCESSOR_H
#define SAMPLEPROCESSOR_H
#include <QString>
#include <QSharedPointer>
#include <random>

class SampleProcessor
{
    int m_counter;
    QString m_name;
    QSharedPointer<std::random_device> m_random;
    std::uniform_int_distribution<int> m_dist;
public:
    SampleProcessor(const QString& name, QSharedPointer<std::random_device> random);
    QString name() const { return m_name; }
    QString doWork(const QString& input);
};

#endif // SAMPLEPROCESSOR_H
