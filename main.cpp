///
/// EXIT CODES
///     0 - Normal exit.
///    -1 - Argument error.
///    -2 - TCP server failed to bind to destination address and port.
///    -3 - UDP socket failed to bind to source address and port.
///
/// TESTING THE APPLICATION
///    Use 'nc 127.0.0.1 65001' to open a TCP connection.
///    Use 'nc -u 127.0.0.1 65000' to send UDP packets.
///
#include <QtCore/QCommandLineParser>
#include <QtCore/QCoreApplication>
#include <QtCore/QStringList>
#include <QtCore/QMap>

#include "networkpipe.h"


///
/// \brief Checks the application's arguments and Initializes the pipe
///        component.
///
int main(int argc, char** argv)
{
    QCoreApplication a(argc, argv);
    QCoreApplication::setApplicationName("qtpipe");
    QCoreApplication::setApplicationVersion("1.0");

    QCommandLineParser parser;
    parser.setApplicationDescription("UDP to TCP forwarding tool.");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument(
                "udp-port",
                "The UDP port where to accept packets from.");
    parser.addPositionalArgument(
                "tcp-port",
                "The TCP port where client connections are accepted from.");

    parser.process(a.arguments());

    // Check if all positional arguments are set.
    QStringList arguments = parser.positionalArguments();
    if (arguments.count() != 2)
        parser.showHelp(-1);

    // Parse the positional arguments.
    bool conversionOk = false;

    quint16 sourcePort = arguments.at(0).toUShort(&conversionOk);
    if (conversionOk == false)
        parser.showHelp(-2);

    quint16 destinationPort = arguments.at(1).toUShort(&conversionOk);
    if (conversionOk == false)
        parser.showHelp(-2);


    // Initialize the NetworkPipe.
    NetworkPipe* pipe = new(std::nothrow) NetworkPipe;
    Q_CHECK_PTR(pipe);

    // Pass the arguments to the pipe and check for errors.
    int initializationError = 0;
    pipe->initialize(sourcePort, destinationPort, &initializationError);

    if (initializationError == -1)
        return -2;
    else if (initializationError == -2)
        return -3;


    return a.exec();
}
