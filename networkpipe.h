#ifndef NETWORKPIPE_H
#define NETWORKPIPE_H

#include <QObject>

class NetworkPipePrivate;


///
/// \brief The NetworkPipe class provides a service that collects UDP packets
///        from a specific port and forwards them to all connected TCP clients.
///
class NetworkPipe : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(NetworkPipe)
    Q_DISABLE_COPY(NetworkPipe)

public:
    explicit NetworkPipe(QObject* parent = nullptr);

    void initialize(quint16 sourcePort,
                    quint16 destinationPort,
                    int* error = nullptr);

private:
    NetworkPipePrivate* d_ptr;
};

#endif // NETWORKPIPE_H
