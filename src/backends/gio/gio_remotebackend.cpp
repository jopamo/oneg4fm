/*
 * GIO remote backend implementation
 * src/backends/gio/gio_remotebackend.cpp
 */

#include "gio_remotebackend.h"

#include <QDebug>

namespace PCManFM {

GioRemoteBackend::GioRemoteBackend() {
    volumeMonitor_ = g_volume_monitor_get();
    if (!volumeMonitor_) {
        qWarning() << "Failed to get GIO volume monitor";
    }
}

GioRemoteBackend::~GioRemoteBackend() {
    if (volumeMonitor_) {
        g_object_unref(volumeMonitor_);
    }
}

bool GioRemoteBackend::isRemote(const QUrl& url) const {
    if (!url.isValid()) {
        return false;
    }

    QString scheme = url.scheme().toLower();

    // List of remote URL schemes supported by GVFS/GIO
    static const QStringList remoteSchemes = {
        QStringLiteral("sftp"),    QStringLiteral("ftp"),    QStringLiteral("ftps"),    QStringLiteral("smb"),
        QStringLiteral("cifs"),    QStringLiteral("webdav"), QStringLiteral("webdavs"), QStringLiteral("dav"),
        QStringLiteral("davs"),    QStringLiteral("http"),   QStringLiteral("https"),   QStringLiteral("mtp"),
        QStringLiteral("gphoto2"), QStringLiteral("afp"),    QStringLiteral("nfs")};

    return remoteSchemes.contains(scheme);
}

QString GioRemoteBackend::mapToMountPoint(const QUrl& url) {
    if (!volumeMonitor_ || !isRemote(url)) {
        return QString();
    }

    // Convert QUrl to GFile
    QString urlString = url.toString();
    GFile* gfile = g_file_new_for_uri(urlString.toUtf8().constData());
    if (!gfile) {
        return QString();
    }

    // Get the mount for this file
    GMount* mount = g_file_find_enclosing_mount(gfile, nullptr, nullptr);
    if (!mount) {
        g_object_unref(gfile);
        return QString();
    }

    // Get the root of the mount
    GFile* root = g_mount_get_root(mount);
    QString mountPoint;

    if (root) {
        char* path = g_file_get_path(root);
        if (path) {
            mountPoint = QString::fromUtf8(path);
            g_free(path);
        }
        g_object_unref(root);
    }

    g_object_unref(mount);
    g_object_unref(gfile);

    return mountPoint;
}

}  // namespace PCManFM