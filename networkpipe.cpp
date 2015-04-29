#include "networkpipe.h"

#include <iostream>

#include <QCoreApplication>
#include <QUdpSocket>
#include <QTcpSocket>
#include <QTcpServer>
#include <QList>


class NetworkPipePrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(NetworkPipe)
    Q_DISABLE_COPY(NetworkPipePrivate)

public:
    QTcpServer* outputServer;
    QList<QTcpSocket*> clientConnections;

    QUdpSocket* inputSocket;

    explicit NetworkPipePrivate(NetworkPipe* parent);

public slots:
    void handleIncomingConnection();
    void handleClientError(QAbstractSocket::SocketError error);

    void forwardData();

private:
    NetworkPipe* q_ptr;
};

#include "networkpipe.moc"


NetworkPipePrivate::NetworkPipePrivate(NetworkPipe* parent) :
    QObject(parent),
    q_ptr(parent)
{
    bool isConnected = false;
    Q_UNUSED(isConnected);


    // Initialize the TCP output server.
    outputServer = new(std::nothrow) QTcpServer(this);
    Q_CHECK_PTR(outputServer);

    // Handle incoming client connections.
    isConnected = connect(outputServer,
                          SIGNAL(newConnection()),
                          this,
                          SLOT(handleIncomingConnection()));
    Q_ASSERT(isConnected == true);


    // Initialize the UDP input server.
    inputSocket = new(std::nothrow) QUdpSocket(this);
    Q_CHECK_PTR(inputSocket);

    isConnected = connect(inputSocket,
                          SIGNAL(readyRead()),
                          this,
                          SLOT(forwardData()));
    Q_ASSERT(isConnected == true);
}

///
/// \brief This slot gets called when a client wants to connect to the TCP
///        server. It setups the error handling and adds it to the connection
///        list.
///
void NetworkPipePrivate::handleIncomingConnection()
{
    bool isConnected = false;
    Q_UNUSED(isConnected);


    while (outputServer->hasPendingConnections() == true) {
        QTcpSocket* clientConnection = outputServer->nextPendingConnection();
        Q_CHECK_PTR(clientConnection);

        // Handle client connection errors.
        isConnected = connect(
                    clientConnection,
                    SIGNAL(error(QAbstractSocket::SocketError)),
                    this,
                    SLOT(handleClientError(QAbstractSocket::SocketError)));
        Q_ASSERT(isConnected == true);

        // Add the connection to the client list.
        clientConnections.append(clientConnection);
    }
}

///
/// \brief This slot gets called by a QTcpSocket that encountered a network
///        error and removes it from the list of connected clients.
///
/// \param error The exact reason of the network error. It is currently
///              not used.
///
void NetworkPipePrivate::handleClientError(QAbstractSocket::SocketError error)
{
    Q_UNUSED(error); //< The exact reason doesn't matter.

    // Get the client connection that sent the error signal. Assume that only
    // QTcpSocket objects are calling this slot.
    QTcpSocket* clientConnection = qobject_cast<QTcpSocket*>(sender());
    Q_CHECK_PTR(clientConnection);

    // Remove the failed client socket from the connection list.
    clientConnections.removeAll(clientConnection);
}

///
/// \brief This slot typically gets called by the listening UDP socket and
///        passes all incoming data unfiltered to the connected TCP client
///        sockets.
///
void NetworkPipePrivate::forwardData()
{
    // Get the receiving UDP socket. Assume that only QUdpSocket objects are
    // calling this slot.
    QUdpSocket* server = qobject_cast<QUdpSocket*>(sender());

    // Receive all data.
    QByteArray data;
    data.resize(server->pendingDatagramSize());
    server->readDatagram(data.data(), data.size());

    // Forward the data to all connected clients.
    foreach (QTcpSocket* client, clientConnections)
        client->write(data);
}


NetworkPipe::NetworkPipe(QObject *parent) :
    QObject(parent),
    d_ptr(nullptr)
{
    d_ptr = new(std::nothrow) NetworkPipePrivate(this);
    Q_CHECK_PTR(d_ptr);
}

///
/// \brief Initializes the NetworkPipe service by binding the TCP and UDP
///        sockets to the given parameters.
///
/// \param sourcePort The port the UDP socket shall listen to.
/// \param sourceAddress The address the UDP socket shall be bound to.
/// \param destinationPort The port the TCP server shall listen to.
/// \param destinationAddress The address the TCP server shall listen on.
///
/// \param error If set to a valid intptr error will contain 0 on success,
///              -1 if the TCP server failed to listen and -2 if the UDP
///              socket failed to bind.
///
void NetworkPipe::initialize(quint16 sourcePort,
                             const QString& sourceAddress,
                             quint16 destinationPort,
                             const QString& destinationAddress,
                             int* error)
{
    Q_D(NetworkPipe);

    // Enable the TCP server.
    bool isServerListening = d->outputServer->listen(
                QHostAddress(destinationAddress),
                destinationPort);
    if (isServerListening == false) {
        std::cout << "TCP Server failed to listen." << std::endl;
        if (error != nullptr)
            *error = -1;

        return;
    }


    // Enable the UDP receiver. The bind shall be exclusive to get an error
    // if there is already a service bound to the given address/port.
    bool isInputListening = d->inputSocket->bind(
                QHostAddress(sourceAddress),
                sourcePort,
                QAbstractSocket::DontShareAddress);
    if (isInputListening == false) {
        std::cout << "UDP Socket failed to bind." << std::endl;
        if (error != nullptr)
            *error = -2;

        return;
    }


    if (error != nullptr)
        *error = 0;
}
