/* This file is part of the KDE project

   Copyright (C) 2008 Dario Freddi <drf@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "polkitkde1helper.h"

#include "helperadaptor.h"

#include <QtCore/QTimer>
#include <QtCore/QSettings>
#include <QtXml/QDomDocument>

#include <QtDBus/QDBusConnection>

#include <PolkitQt1/Authority>
#include <QDir>
#include <KLocalizedString>

bool orderByPriorityLessThan(const PKLAEntry &e1, const PKLAEntry &e2)
{
    return e1.fileOrder < e2.fileOrder;
}

PolkitKde1Helper::PolkitKde1Helper(QObject* parent)
    : QObject(parent)
{
    qDBusRegisterMetaType<PKLAEntry>();
    qDBusRegisterMetaType<QList<PKLAEntry> >();
    (void) new HelperAdaptor(this);
    // Register the DBus service
    if (!QDBusConnection::systemBus().registerService("org.kde.polkitkde1.helper")) {
        qDebug() << QDBusConnection::systemBus().lastError().message();;
        QTimer::singleShot(0, QCoreApplication::instance(), SLOT(quit()));
        return;
    }

    if (!QDBusConnection::systemBus().registerObject("/Helper", this)) {
        qDebug() << "unable to register service interface to dbus";
        QTimer::singleShot(0, QCoreApplication::instance(), SLOT(quit()));
        return;
    }
}

PolkitKde1Helper::~PolkitKde1Helper()
{

}

void PolkitKde1Helper::saveGlobalConfiguration(const QString& adminIdentities, int systemPriority, int policiesPriority)
{
    qDebug() << "Request to save the global configuration by " << message().service();
    PolkitQt1::Authority::Result result;
    PolkitQt1::SystemBusNameSubject subject(message().service());

    result = PolkitQt1::Authority::instance()->checkAuthorizationSync("org.kde.polkitkde1.changesystemconfiguration",
                                                                      subject, PolkitQt1::Authority::AllowUserInteraction);
    switch (result)
    {
        case PolkitQt1::Authority::Yes:
            qDebug() << "Authorized successfully";
            break;
        case PolkitQt1::Authority::No:
            sendErrorReply(QDBusError::AccessDenied, i18n("Changing global configurations is unauthorized"));
            return;
        default:
            sendErrorReply(QDBusError::AccessDenied, i18n("Unknown reply from QPolkit-1\nError: %1", PolkitQt1::Authority::instance()->errorDetails()));
            return;
    }

    // Ok, let's see what we have to save here.
    QSettings kdesettings("/etc/polkit-1/polkit-kde-1.conf", QSettings::IniFormat);
    kdesettings.beginGroup("General");

    kdesettings.setValue("ConfigPriority", systemPriority);
    kdesettings.setValue("PoliciesPriority", policiesPriority);

    QString contents = QString("[Configuration]\nAdminIdentities=%1\n").arg(adminIdentities);

    qDebug() << contents << "will be wrote to the local authority file";
    QFile wfile(QString("/etc/polkit-1/localauthority.conf.d/%1-polkitkde.conf").arg(systemPriority));
    if (!wfile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        sendErrorReply(QDBusError::Failed, i18n("Error opening the global configuration file: %1\nError code: %2.", wfile.fileName(), wfile.error()));
        return;
    }
    if (wfile.write(contents.toUtf8()) < 0) {
        sendErrorReply(QDBusError::Failed, i18n("Error occurred while writing settings to file: %1.", wfile.fileName()));
        return;
    }
    if (!wfile.flush()) {
        sendErrorReply(QDBusError::Failed, i18n("Error occurred while flushing settings to file: %1.", wfile.fileName()));
        return;
    }
    wfile.close();

    // TODO: Move files around if the priority was changed
}

QVariantList PolkitKde1Helper::retrievePolicies()
{
    qDebug() << "Request to retrieve the action authorizations by " << message().service();
    PolkitQt1::Authority::Result result;
    PolkitQt1::SystemBusNameSubject subject(message().service());

    result = PolkitQt1::Authority::instance()->checkAuthorizationSync("org.kde.polkitkde1.readauthorizations",
                                                                      subject, PolkitQt1::Authority::AllowUserInteraction);
    switch (result)
    {
        case PolkitQt1::Authority::Yes:
            qDebug() << "Authorized successfully";
            break;
        case PolkitQt1::Authority::No:
            sendErrorReply(QDBusError::AccessDenied, i18n("Reading policy settings is unauthorized"));
            return QVariantList();
        default:
            sendErrorReply(QDBusError::AccessDenied, i18n("Unknown reply from QPolkit-1\nError: %1",PolkitQt1::Authority::instance()->errorDetails()));
            return QVariantList();
    }

    return reloadFileList();
}

QVariantList PolkitKde1Helper::entriesFromFile(int filePriority, const QString& filePath)
{
    QList< QVariant > retlist;
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Failed to open " << filePath;
        sendErrorReply(QDBusError::Failed, i18n("Error opening the file: %1\nError code: %2.", file.fileName(), file.error()));
        return QVariantList();
    }

    int priority = 0;

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        if (line.startsWith('[')) {
            // Ok, that's a key entry
            PKLAEntry entry;
            entry.filePath = filePath;
            entry.title = line.split('[').last().split(']').first();
            entry.filePriority = filePriority;
            entry.fileOrder = priority;
            // Now parse over all the rest
            while (!in.atEnd()) {
                QString line = in.readLine();
                if (line.startsWith(QLatin1String("Identity="))) {
                    entry.identity = line.split("Identity=").last();
                } else if (line.startsWith(QLatin1String("Action="))) {
                    entry.action = line.split("Action=").last();
                } else if (line.startsWith(QLatin1String("ResultAny="))) {
                    entry.resultAny = line.split("ResultAny=").last();
                } else if (line.startsWith(QLatin1String("ResultInactive="))) {
                    entry.resultInactive = line.split("ResultInactive=").last();
                } else if (line.startsWith(QLatin1String("ResultActive="))) {
                    entry.resultActive = line.split("ResultActive=").last();
                } else if (line.startsWith('[')) {
                    // Ouch!!
                    qWarning() << "The file appears malformed!! " << filePath;
                    return QVariantList();
                }

                // Did we parse it all?
                if (!entry.identity.isEmpty() && !entry.action.isEmpty() && !entry.resultActive.isEmpty() &&
                    !entry.resultAny.isEmpty() && !entry.resultInactive.isEmpty() && !entry.filePath.isEmpty()) {
                    // Awesome, add and wave goodbye. And also increase the priority
                    ++priority;
                    qDebug() << "PKLA Parsed:" << entry.title << entry.action << entry.identity << entry.resultAny
                             << entry.resultInactive << entry.resultActive << entry.fileOrder;
                    retlist.append(QVariant::fromValue(entry));
                    break;
                }
            }
        }
    }

    return retlist;
}

void PolkitKde1Helper::writeImplicitPolicy(const QList<PKLAEntry>& policy)
{
    QList<PKLAEntry> entries = policy;

    if (entries.empty()) {
        return;
    }

    PolkitQt1::Authority::Result result;
    PolkitQt1::SystemBusNameSubject subject(message().service());

    result = PolkitQt1::Authority::instance()->checkAuthorizationSync("org.kde.polkitkde1.changeimplicitauthorizations",
                                                                      subject, PolkitQt1::Authority::AllowUserInteraction);

    switch (result)
    {
        case PolkitQt1::Authority::Yes:
            qDebug() << "Authorized successfully";
            break;
        case PolkitQt1::Authority::No:
            sendErrorReply(QDBusError::AccessDenied, i18n("Saving implicit policy settings is unauthorized"));
            return;
        default:
            sendErrorReply(QDBusError::AccessDenied, i18n("Unknown reply from QPolkit-1\nError: %1", PolkitQt1::Authority::instance()->errorDetails()));
            return;
    }

    foreach(const PKLAEntry &entry, entries) {
        QDomDocument doc = QDomDocument("policy");
        QStringList actionNameSplitted = entry.action.split('.');
        QString newName;
        QFile *pfile = new QFile("/usr/share/polkit-1/actions/org.freedesktop.kit.policy");
        // Search for a valid file
        foreach(const QString &nameSplitted , actionNameSplitted) {
            newName.append(nameSplitted);
            pfile = new QFile("/usr/share/polkit-1/actions/" + newName + ".policy");
            if (!pfile->open(QIODevice::ReadOnly)) {
                newName.append(".");
                delete pfile;
                pfile = NULL;
                continue;
            }
            if (!pfile->exists()) {
                newName.append(".");
                pfile->close();
                delete pfile;
                pfile = NULL;
                continue;
            }

            // We should now have found a valid file.
            break;
        }

        // Check if the file is valid
        if (pfile == NULL) {
            sendErrorReply(QDBusError::Failed, i18n("Did not find a valid policy file for the implicit action: %1.", entry.action));
            return;
        }

        // Create XML doc.
        QString domDocumentError;
        if (!doc.setContent(pfile, &domDocumentError)) {
            pfile->close();
            sendErrorReply(QDBusError::Failed, i18n("Error occurred while parsing file: %1\nError message: %2", pfile->fileName(), domDocumentError));
            return;
        }
        pfile->close();

        QDomElement el = doc.firstChildElement("policyconfig").firstChildElement("action");
        while (!el.isNull() && el.attribute("id", QString()) != entry.action) {
            el = el.nextSiblingElement("action");
        }
        el = el.firstChildElement("defaults");

        // Delete all its childrens, :P
        QDomNodeList nodelist = el.childNodes();
        while (nodelist.length() > 0) {
            el.removeChild(nodelist.item(0));
        }

        // Add new elements
        QDomElement inactiveElem = el.appendChild(doc.createElement("allow_inactive")).toElement();
        QDomElement activeElem = el.appendChild(doc.createElement("allow_active")).toElement();
        QDomElement anyElem = el.appendChild(doc.createElement("allow_any")).toElement();
        inactiveElem.appendChild(doc.createTextNode(entry.resultInactive));
        activeElem.appendChild(doc.createTextNode(entry.resultActive));
        anyElem.appendChild(doc.createTextNode(entry.resultAny));

        // Write the settings to the XML
        if (!pfile->open(QIODevice::WriteOnly)) {
            // Failed to write to the file?
            qDebug() << "Failed writing to file: " << pfile->fileName();
            sendErrorReply(QDBusError::Failed, i18n("Error opening the file: %1\nError code: %2.", pfile->fileName(), pfile->error()));
            return;
        }
        QTextStream stream(pfile);
        doc.save(stream, 2);
        pfile->close();
        delete pfile;
    }
}

void PolkitKde1Helper::writePolicy(const QList<PKLAEntry>& policy)
{
    QList<PKLAEntry> entries = policy;

    PolkitQt1::Authority::Result result;
    PolkitQt1::SystemBusNameSubject subject(message().service());

    result = PolkitQt1::Authority::instance()->checkAuthorizationSync("org.kde.polkitkde1.changeexplicitauthorizations",
                                                                      subject, PolkitQt1::Authority::AllowUserInteraction);
    switch (result)
    {
        case PolkitQt1::Authority::Yes:
            qDebug() << "Authorized successfully";
            break;
        case PolkitQt1::Authority::No:
            sendErrorReply(QDBusError::AccessDenied, "Saving explicit policy settings is unauthorized");
            return;
        default:
            sendErrorReply(QDBusError::AccessDenied, i18n("Unknown reply from QPolkit-1\nError: %1", PolkitQt1::Authority::instance()->errorDetails()));
            return;
    }

    // First delete all the old files, we do not need them anymore
    foreach(const QFileInfo &nestedInfo, oldNestedList) {
        QFile::remove(nestedInfo.absoluteFilePath());
    }

    qSort(entries.begin(), entries.end(), orderByPriorityLessThan);

    QSettings kdesettings("/etc/polkit-1/polkit-kde-1.conf", QSettings::IniFormat);
    kdesettings.beginGroup("General");

    QString pathName = QString("/var/lib/polkit-1/localauthority/%1-polkitkde.d/")
                            .arg(kdesettings.value("PoliciesPriority",75).toInt());

    foreach(const PKLAEntry &entry, entries) {
        QString fullPath;

        // First check if it already has a filePath,
        // else generate fullPath from the policy action name
        if (!entry.filePath.isEmpty()) {
            fullPath = entry.filePath;
        } else {
            QStringList dotSplittedFileName = entry.action.split('.');

            // Skip the action name
            dotSplittedFileName.removeLast();
            dotSplittedFileName << "pkla";

            // Check if the polkitkde.d dir exists. If not, create it.
            QDir path(pathName);

            if (!path.exists()) {
                if (!path.mkpath(path.absolutePath())) {
                    sendErrorReply(QDBusError::Failed, i18n("Error creating the directory: %1.", path.absolutePath()));
                    return;
                }
            }

            fullPath = pathName + dotSplittedFileName.join(".");
        }

        QString contents;
        contents.append(formatPKLAEntry(entry));
        contents.append('\n');

        QFile wfile(fullPath);
        if (!wfile.open(QIODevice::Append | QIODevice::Truncate | QIODevice::Text)) {
            sendErrorReply(QDBusError::Failed, i18n("Error opening the explicit setting file: %1\nError code: %2.", wfile.fileName(), wfile.error()));
            return;
        }
        if (wfile.write(contents.toUtf8()) < 0) {
            sendErrorReply(QDBusError::Failed, i18n("Error occurred while writing explicit settings to file: %1.", wfile.fileName()));
            return;
        }
        if (!wfile.flush()) {
            sendErrorReply(QDBusError::Failed, i18n("Error occurred while flushing explicit settings to file: %1.", wfile.fileName()));
            return;
        }
    }

    // Reload fileList;
    reloadFileList();
}

QString PolkitKde1Helper::formatPKLAEntry(const PKLAEntry& entry)
{
    QString retstring;
    retstring.append(QString("[%1]\n").arg(entry.title));
    retstring.append(QString("Identity=%1\n").arg(entry.identity));
    retstring.append(QString("Action=%1\n").arg(entry.action));
    retstring.append(QString("ResultAny=%1\n").arg(entry.resultAny));
    retstring.append(QString("ResultInactive=%1\n").arg(entry.resultInactive));
    retstring.append(QString("ResultActive=%1\n").arg(entry.resultActive));
    return retstring;
}

QVariantList PolkitKde1Helper::reloadFileList()
{
    QVariantList retlist;
    oldNestedList.clear();

    // Iterate over the directory and find out everything
    QDir baseDir("/var/lib/polkit-1/localauthority/");
    QFileInfoList baseList = baseDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);

    foreach (const QFileInfo &info, baseList) {
        int filePriority = info.baseName().split('-').first().toInt();
        qDebug() << "Iterating over the directory " << info.absoluteFilePath() << ", which has a priority of " << filePriority;

        QDir nestedDir(info.absoluteFilePath());
        QFileInfoList nestedList = nestedDir.entryInfoList(QDir::Files);

        foreach (const QFileInfo &nestedInfo, nestedList) {
            qDebug() << "Parsing file " << nestedInfo.absoluteFilePath();
            retlist.append(entriesFromFile(filePriority, nestedInfo.absoluteFilePath()));
        }

        // Save all the fileinfo's, for a later delete if we choose to save
        oldNestedList << nestedList;
    }

    return retlist;
}
