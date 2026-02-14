/*
 * Qt-based file operations backend implementation
 * src/backends/qt/qt_fileops.cpp
 */

#include "qt_fileops.h"

#include "../../core/fs_ops.h"

#include <QCoreApplication>
#include <QFile>
#include <QFileInfo>
#include <QObject>
#include <QThread>

#include <algorithm>
#include <atomic>
#include <dirent.h>
#include <functional>
#include <limits>
#include <vector>
#include <cerrno>
#include <cstring>
#include <sys/stat.h>

namespace PCManFM {

namespace {

std::string toNativePath(const QString& path) {
    const QByteArray bytes = QFile::encodeName(path);
    return std::string(bytes.constData(), static_cast<std::size_t>(bytes.size()));
}

QString fromNativePath(const std::string& path) {
    return QString::fromLocal8Bit(path.c_str());
}

FileOpProgress toQtProgress(const FsOps::ProgressInfo& core) {
    FileOpProgress qt{};
    qt.bytesDone = core.bytesDone;
    qt.bytesTotal = core.bytesTotal;
    qt.filesDone = core.filesDone;
    qt.filesTotal = core.filesTotal;
    qt.currentPath = fromNativePath(core.currentPath);
    return qt;
}

void setErrnoError(FsOps::Error& err, const char* context) {
    err.code = errno;
    err.message = std::string(context) + ": " + std::string(std::strerror(errno));
}

void addU64Saturated(std::uint64_t& dst, std::uint64_t value) {
    const auto max = std::numeric_limits<std::uint64_t>::max();
    dst = (value > max - dst) ? max : (dst + value);
}

void addIntSaturated(int& dst, int value) {
    const int max = std::numeric_limits<int>::max();
    if (value <= 0 || dst >= max) {
        return;
    }
    dst = (value > max - dst) ? max : (dst + value);
}

struct SourceStats {
    std::uint64_t bytesTotal = 0;
    int entryCount = 0;
};

bool scanPathStats(const std::string& path, SourceStats& stats, FsOps::Error& err, int depth = 0) {
    if (depth > FsOps::kMaxRecursionDepth) {
        err.code = ELOOP;
        err.message = "Maximum recursion depth exceeded";
        return false;
    }

    struct stat st{};
    if (::lstat(path.c_str(), &st) < 0) {
        setErrnoError(err, "lstat");
        return false;
    }

    addIntSaturated(stats.entryCount, 1);
    if (S_ISREG(st.st_mode)) {
        addU64Saturated(stats.bytesTotal, static_cast<std::uint64_t>(st.st_size));
    }

    if (!S_ISDIR(st.st_mode)) {
        return true;
    }

    DIR* dir = ::opendir(path.c_str());
    if (!dir) {
        setErrnoError(err, "opendir");
        return false;
    }

    for (;;) {
        errno = 0;
        dirent* ent = ::readdir(dir);
        if (!ent) {
            if (errno != 0) {
                setErrnoError(err, "readdir");
                ::closedir(dir);
                return false;
            }
            break;
        }

        const char* child = ent->d_name;
        if (!child || child[0] == '\0' || std::strcmp(child, ".") == 0 || std::strcmp(child, "..") == 0) {
            continue;
        }

        std::string childPath = path;
        if (!childPath.empty() && childPath.back() != '/') {
            childPath.push_back('/');
        }
        childPath += child;

        if (!scanPathStats(childPath, stats, err, depth + 1)) {
            ::closedir(dir);
            return false;
        }
    }

    ::closedir(dir);
    return true;
}

}  // namespace

class QtFileOps::Worker : public QObject {
    Q_OBJECT

   public:
    explicit Worker(QObject* parent = nullptr) : QObject(parent), cancelled_(false) {}

   public Q_SLOTS:
    void processRequest(const FileOpRequest& req) {
        cancelled_.store(false);

        switch (req.type) {
            case FileOpType::Copy:
                performCopy(req);
                break;
            case FileOpType::Move:
                performMove(req);
                break;
            case FileOpType::Delete:
                performDelete(req);
                break;
        }
    }

    void cancel() { cancelled_.store(true); }

   Q_SIGNALS:
    void progress(const FileOpProgress& info);
    void finished(bool success, const QString& errorMessage);

   private:
    bool performOperationList(const FileOpRequest& req,
                              const std::function<bool(const std::string&,
                                                       const std::string&,
                                                       FsOps::ProgressInfo&,
                                                       const FsOps::ProgressCallback&,
                                                       FsOps::Error&)>& op,
                              bool needsDestination) {
        struct SourcePlan {
            std::string sourcePath;
            std::string destinationPath;
            SourceStats stats;
            int workUnits = 1;
        };

        std::vector<SourcePlan> plans;
        plans.reserve(static_cast<std::size_t>(req.sources.size()));

        int totalWorkUnits = 0;
        std::uint64_t totalBytes = 0;

        for (const QString& sourcePath : req.sources) {
            if (cancelled_.load()) {
                Q_EMIT finished(false, QStringLiteral("Operation cancelled"));
                return false;
            }

            const QString fileName = QFileInfo(sourcePath).fileName();
            const QString destinationPath =
                needsDestination ? (req.destination + QLatin1Char('/') + fileName) : QString();

            const std::string srcNative = toNativePath(sourcePath);
            const std::string dstNative = needsDestination ? toNativePath(destinationPath) : std::string();

            FsOps::Error err;
            SourceStats stats;
            if (!scanPathStats(srcNative, stats, err)) {
                Q_EMIT finished(false, QString::fromLocal8Bit(err.message.c_str()));
                return false;
            }

            SourcePlan plan;
            plan.sourcePath = srcNative;
            plan.destinationPath = dstNative;
            plan.stats = stats;
            plan.workUnits = (req.type == FileOpType::Delete) ? std::max(1, stats.entryCount) : 1;

            addIntSaturated(totalWorkUnits, plan.workUnits);
            addU64Saturated(totalBytes, stats.bytesTotal);
            plans.push_back(std::move(plan));
        }

        int completedWorkUnits = 0;
        std::uint64_t completedBytes = 0;

        for (const SourcePlan& plan : plans) {
            if (cancelled_.load()) {
                Q_EMIT finished(false, QStringLiteral("Operation cancelled"));
                return false;
            }

            FsOps::ProgressInfo sourceProgress{};
            sourceProgress.filesTotal = plan.workUnits;
            sourceProgress.filesDone = 0;
            sourceProgress.bytesTotal = plan.stats.bytesTotal;
            sourceProgress.bytesDone = 0;
            sourceProgress.currentPath = plan.sourcePath;

            auto opProgress = [this, &plan, &completedWorkUnits, &completedBytes, totalWorkUnits,
                               totalBytes](const FsOps::ProgressInfo& sourceInfo) {
                FsOps::ProgressInfo overall = sourceInfo;
                const int localDone = std::min(std::max(0, sourceInfo.filesDone), plan.workUnits);
                const std::uint64_t localBytesDone = std::min(sourceInfo.bytesDone, plan.stats.bytesTotal);
                overall.filesTotal = totalWorkUnits;
                overall.filesDone =
                    std::min(std::numeric_limits<int>::max() - completedWorkUnits, localDone) + completedWorkUnits;
                overall.bytesTotal = totalBytes;
                overall.bytesDone = std::min<std::uint64_t>(totalBytes, completedBytes + localBytesDone);
                if (overall.currentPath.empty()) {
                    overall.currentPath = plan.sourcePath;
                }
                Q_EMIT progress(toQtProgress(overall));
                return !cancelled_.load();
            };

            FsOps::Error err;
            if (!op(plan.sourcePath, plan.destinationPath, sourceProgress, opProgress, err)) {
                if (cancelled_.load() || err.code == ECANCELED) {
                    Q_EMIT finished(false, QStringLiteral("Operation cancelled"));
                }
                else {
                    const QString msg =
                        err.isSet() ? QString::fromLocal8Bit(err.message.c_str()) : QStringLiteral("Operation failed");
                    Q_EMIT finished(false, msg);
                }
                return false;
            }

            addIntSaturated(completedWorkUnits, plan.workUnits);
            addU64Saturated(completedBytes, plan.stats.bytesTotal);

            FsOps::ProgressInfo overallFinal{};
            overallFinal.filesTotal = totalWorkUnits;
            overallFinal.filesDone = completedWorkUnits;
            overallFinal.bytesTotal = totalBytes;
            overallFinal.bytesDone = completedBytes;
            overallFinal.currentPath = plan.sourcePath;
            Q_EMIT progress(toQtProgress(overallFinal));
        }

        Q_EMIT finished(true, QString());
        return true;
    }

    void performCopy(const FileOpRequest& req) {
        performOperationList(
            req,
            [req](const std::string& src, const std::string& dst, FsOps::ProgressInfo& progress,
                  const FsOps::ProgressCallback& cb, FsOps::Error& err) {
                const bool preserve = req.preserveOwnership;
                return FsOps::copy_path(src, dst, progress, cb, err, preserve);
            },
            /*needsDestination=*/true);
    }

    void performMove(const FileOpRequest& req) {
        performOperationList(
            req,
            [req](const std::string& src, const std::string& dst, FsOps::ProgressInfo& progress,
                  const FsOps::ProgressCallback& cb, FsOps::Error& err) {
                return FsOps::move_path(src, dst, progress, cb, err, /*forceCopyFallbackForTests=*/false,
                                        req.preserveOwnership);
            },
            /*needsDestination=*/true);
    }

    void performDelete(const FileOpRequest& req) {
        performOperationList(
            req,
            [](const std::string& src, const std::string& /*unused*/, FsOps::ProgressInfo& progress,
               const FsOps::ProgressCallback& cb,
               FsOps::Error& err) { return FsOps::delete_path(src, progress, cb, err); },
            /*needsDestination=*/false);
    }

    std::atomic<bool> cancelled_;
};

QtFileOps::QtFileOps(QObject* parent) : IFileOps(parent), worker_(new Worker), workerThread_(new QThread) {
    worker_->moveToThread(workerThread_);

    connect(this, &QtFileOps::startRequest, worker_, &Worker::processRequest);
    connect(this, &QtFileOps::cancelRequest, worker_, &Worker::cancel);
    connect(worker_, &Worker::progress, this, &QtFileOps::progress);
    connect(worker_, &Worker::finished, this, &QtFileOps::onWorkerFinished);

    workerThread_->start();
}

void QtFileOps::onWorkerFinished(bool success, const QString& errorMessage) {
    Q_EMIT finished(success, errorMessage);
}

QtFileOps::~QtFileOps() {
    cancel();
    workerThread_->quit();
    workerThread_->wait();
    delete worker_;
    delete workerThread_;
}

void QtFileOps::start(const FileOpRequest& req) {
    Q_EMIT startRequest(req);
}

void QtFileOps::cancel() {
    Q_EMIT cancelRequest();
}

}  // namespace PCManFM

#include "qt_fileops.moc"
