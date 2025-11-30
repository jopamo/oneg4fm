/*
 * Qt-based file operations backend
 * src/backends/qt/qt_fileops.h
 */

#ifndef QT_FILEOPS_H
#define QT_FILEOPS_H

#include <QAtomicInteger>
#include <QDir>
#include <QFile>
#include <QMutex>
#include <QThread>

#include "../../core/ifileops.h"

namespace PCManFM {

class QtFileOps : public IFileOps {
    Q_OBJECT

   public:
    explicit QtFileOps(QObject* parent = nullptr);
    ~QtFileOps() override;

    void start(const FileOpRequest& req) override;
    void cancel() override;

   private Q_SLOTS:
    void onWorkerFinished(bool success, const QString& errorMessage);

   Q_SIGNALS:
    void startRequest(const FileOpRequest& req);
    void cancelRequest();

   private:
    class Worker;
    Worker* worker_;
    QThread* workerThread_;
};

}  // namespace PCManFM

#endif  // QT_FILEOPS_H