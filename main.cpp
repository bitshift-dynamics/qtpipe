///
/// EXIT CODES
///     0 - Normal exit.
///    -1 - Argument error.
///    -2 - TCP server failed to bind to destination address and port.
///    -3 - UDP socket failed to bind to source address and port.
///
#include <iostream>

#include <QCoreApplication>
#include <QStringList>
#include <QMap>

#include "networkpipe.h"


QMap<QString, QString> readArguments(const QStringList& arguments,
                                     bool* error = nullptr);

/*#include <QJsonArray>
#include <QJsonDocument>
#include <QVariant>
#include <QJsonObject>
#include <QDebug>*/

///
/// \brief Checks the application's arguments and Initializes the pipe
///        component.
///
int main(int argc, char** argv)
{
    QCoreApplication a(argc, argv);


    // JSON Demo
    /*QVariantMap m;
    m["version"] = 3;

    QVariantMap t;
    t["a"] = 10;

    QVariantMap b;
    b["b"] = 4.5f;

    QJsonArray o;
    o.append(QJsonValue::fromVariant(t));
    o.append(QJsonValue::fromVariant(b));

    m["o"] = o.toVariantList();

    QJsonDocument doc(QJsonValue::fromVariant(m).toObject());
    qDebug() << doc.toJson(QJsonDocument::Compact);*/


    // Extract and check arguments.
    bool hasArgumentError = false;
    QMap<QString, QString> arguments = readArguments(
                QCoreApplication::arguments(),
                &hasArgumentError);
    if (hasArgumentError == true) {
        std::cout << "Usage: qtpipe --source-port=[PORT] "
                     "--source-address=[ADDRESS] --destination-port=[PORT] "
                     "--destination-address=[ADDRESS]"
                  << std::endl;
        return -1;
    }


    // Initialize the NetworkPipe.
    NetworkPipe* pipe = new(std::nothrow) NetworkPipe;
    Q_CHECK_PTR(pipe);

    // Pass the arguments to the pipe and check for errors.
    int initializationError = 0;
    pipe->initialize(arguments["sourcePort"].toUShort(),
            arguments["sourceAddress"],
            arguments["destinationPort"].toUShort(),
            arguments["destinationAddress"],
            &initializationError);
    if (initializationError == -1)
        return -2;
    else if (initializationError == -2)
        return -3;


    return a.exec();
}

///
/// \brief readArguments
///
/// \param arguments
/// \param error
///
/// \return
///
QMap<QString, QString> readArguments(const QStringList& arguments,
                                     bool* error)
{
    int sourcePortIndex = arguments.indexOf("--source-port");
    int sourceAddressIndex = arguments.indexOf("--source-address");
    int destinationPortIndex = arguments.indexOf("--destination-port");
    int destinationAddressIndex = arguments.indexOf("--destination-address");

    // Check if all needed arguments are available.
    if (sourcePortIndex == -1 || sourceAddressIndex == -1 ||
            destinationPortIndex == -1 || destinationAddressIndex == -1) {
        if (error != nullptr)
            *error = true;

        return QMap<QString, QString>();
    }


    // Extract the relevant arguments.
    QMap<QString, QString> parameters;
    parameters["sourcePort"] = arguments.at(
                sourcePortIndex + 1);
    parameters["sourceAddress"] = arguments.at(
                sourceAddressIndex + 1);
    parameters["destinationPort"] = arguments.at(
                destinationPortIndex + 1);
    parameters["destinationAddress"] = arguments.at(
                destinationAddressIndex + 1);


    // Check if the given ports are in range.
    bool conversionOk = false;
    int sourcePort = parameters["sourcePort"].toInt(&conversionOk);
    if (conversionOk == false) {
        if (error != nullptr)
            *error = true;

        return QMap<QString, QString>();
    }

    int destinationPort = parameters["destinationPort"].toInt(&conversionOk);
    if (conversionOk == false) {
        if (error != nullptr)
            *error = true;

        return QMap<QString, QString>();
    }

    if (sourcePort < 0 || sourcePort > 65335 ||
            destinationPort < 0 || destinationPort > 65535) {
        if (error != nullptr)
            *error = true;

        return QMap<QString, QString>();
    }


    //! \todo Check if the given addresses are valid.


    if (error != nullptr)
        *error = false;

    return parameters;
}
