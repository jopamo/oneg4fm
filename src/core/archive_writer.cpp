/*
 * POSIX-based archive writer implementation using libarchive
 * src/core/archive_writer.cpp
 */

#include "archive_writer.h"

#include <archive.h>
#include <archive_entry.h>

#include <array>
#include <cerrno>
#include <cstring>
#include <dirent.h>
#include <fcntl.h>
#include <limits>
#include <string_view>
#include <sys/stat.h>
#include <unistd.h>

namespace PCManFM::ArchiveWriter {

namespace {

using PCManFM::FsOps::Error;
using PCManFM::FsOps::ProgressCallback;
using PCManFM::FsOps::ProgressInfo;

struct Fd {
    int fd;
    explicit Fd(int f = -1) : fd(f) {}
    ~Fd() {
        if (fd >= 0) {
            ::close(fd);
        }
    }
    Fd(const Fd&) = delete;
    Fd& operator=(const Fd&) = delete;
    Fd(Fd&& other) noexcept : fd(other.fd) { other.fd = -1; }
    Fd& operator=(Fd&& other) noexcept {
        if (this != &other) {
            if (fd >= 0) {
                ::close(fd);
            }
            fd = other.fd;
            other.fd = -1;
        }
        return *this;
    }
    bool valid() const { return fd >= 0; }
};

inline void set_error(Error& err, const std::string& context) {
    err.code = errno;
    err.message = context + ": " + std::strerror(errno);
}

inline void set_archive_error(Error& err, struct archive* ar, const char* context) {
    err.code = EIO;
    err.message = std::string(context) + ": " + archive_error_string(ar);
}

bool should_continue(const ProgressCallback& cb, const ProgressInfo& info) {
    if (!cb) {
        return true;
    }
    return cb(info);
}

std::string parent_dir(const std::string& path) {
    const auto pos = path.find_last_of('/');
    if (pos == std::string::npos) {
        return std::string{};
    }
    if (pos == 0) {
        return "/";  // root
    }
    return path.substr(0, pos);
}

std::string relative_path(const std::string& path, const std::string& base) {
    if (base.empty() || base == ".") {
        return path;
    }
    if (path.size() >= base.size() && std::memcmp(path.data(), base.data(), base.size()) == 0) {
        const std::size_t offset =
            (path.size() > base.size() && path[base.size()] == '/') ? base.size() + 1 : base.size();
        if (offset <= path.size()) {
            return path.substr(offset);
        }
    }
    return path;
}

bool ensure_parent_dirs(const std::string& path, Error& err) {
    const auto parent = parent_dir(path);
    if (parent.empty() || parent == ".") {
        return true;
    }
    return FsOps::make_dir_parents(parent, err);
}

bool accumulate_size(const std::string& path, std::uint64_t& totalBytes, int depth, Error& err) {
    if (depth > FsOps::kMaxRecursionDepth) {
        err.code = ELOOP;
        err.message = "Maximum recursion depth exceeded";
        return false;
    }
    struct stat st{};
    if (::lstat(path.c_str(), &st) != 0) {
        set_error(err, "lstat");
        return false;
    }
    if (S_ISREG(st.st_mode)) {
        totalBytes += static_cast<std::uint64_t>(st.st_size);
        return true;
    }
    if (S_ISLNK(st.st_mode)) {
        return true;  // track but no data payload
    }
    if (!S_ISDIR(st.st_mode)) {
        err.code = ENOTSUP;
        err.message = "Unsupported file type";
        return false;
    }

    Fd dir_fd(::open(path.c_str(), O_RDONLY | O_CLOEXEC | O_DIRECTORY));
    if (!dir_fd.valid()) {
        set_error(err, "open");
        return false;
    }
    DIR* dir = ::fdopendir(dir_fd.fd);
    if (!dir) {
        set_error(err, "fdopendir");
        return false;
    }
    // dir now owns fd
    dir_fd.fd = -1;

    for (;;) {
        errno = 0;
        dirent* ent = ::readdir(dir);
        if (!ent) {
            if (errno != 0) {
                set_error(err, "readdir");
                ::closedir(dir);
                return false;
            }
            break;
        }
        const char* name = ent->d_name;
        if (!name || name[0] == '\0' || std::strcmp(name, ".") == 0 || std::strcmp(name, "..") == 0) {
            continue;
        }
        std::string child = path;
        if (child.back() != '/') {
            child.push_back('/');
        }
        child += name;
        if (!accumulate_size(child, totalBytes, depth + 1, err)) {
            ::closedir(dir);
            return false;
        }
    }
    ::closedir(dir);
    return true;
}

bool write_entry(struct archive* ar,
                 const std::string& path,
                 const std::string& base,
                 ProgressInfo& progress,
                 const ProgressCallback& cb,
                 Error& err,
                 int depth) {
    if (depth > FsOps::kMaxRecursionDepth) {
        err.code = ELOOP;
        err.message = "Maximum recursion depth exceeded";
        return false;
    }

    struct stat st{};
    if (::lstat(path.c_str(), &st) != 0) {
        set_error(err, "lstat");
        return false;
    }

    const std::string relPath = relative_path(path, base);
    progress.currentPath = relPath;
    if (!should_continue(cb, progress)) {
        err.code = ECANCELED;
        err.message = "Cancelled";
        return false;
    }

    archive_entry* entry = archive_entry_new();
    if (!entry) {
        err.code = ENOMEM;
        err.message = "Failed to allocate archive entry";
        return false;
    }

    archive_entry_set_pathname(entry, relPath.c_str());
    archive_entry_set_perm(entry, st.st_mode & 07777);
    archive_entry_set_uid(entry, st.st_uid);
    archive_entry_set_gid(entry, st.st_gid);
    archive_entry_set_mtime(entry, st.st_mtime, 0);
    archive_entry_set_atime(entry, st.st_atime, 0);

    if (S_ISDIR(st.st_mode)) {
        archive_entry_set_filetype(entry, AE_IFDIR);
        archive_entry_set_size(entry, 0);
        if (archive_write_header(ar, entry) != ARCHIVE_OK) {
            set_archive_error(err, ar, "archive_write_header");
            archive_entry_free(entry);
            return false;
        }
        archive_entry_free(entry);
        progress.filesDone += 1;

        Fd dir_fd(::open(path.c_str(), O_RDONLY | O_CLOEXEC | O_DIRECTORY));
        if (!dir_fd.valid()) {
            set_error(err, "open");
            return false;
        }
        DIR* dir = ::fdopendir(dir_fd.fd);
        if (!dir) {
            set_error(err, "fdopendir");
            return false;
        }
        dir_fd.fd = -1;

        for (;;) {
            errno = 0;
            dirent* ent = ::readdir(dir);
            if (!ent) {
                if (errno != 0) {
                    set_error(err, "readdir");
                    ::closedir(dir);
                    return false;
                }
                break;
            }
            const char* name = ent->d_name;
            if (!name || name[0] == '\0' || std::strcmp(name, ".") == 0 || std::strcmp(name, "..") == 0) {
                continue;
            }
            std::string child = path;
            if (child.back() != '/') {
                child.push_back('/');
            }
            child += name;
            if (!write_entry(ar, child, base, progress, cb, err, depth + 1)) {
                ::closedir(dir);
                return false;
            }
        }
        ::closedir(dir);
        return true;
    }

    if (S_ISLNK(st.st_mode)) {
        std::string target(static_cast<std::size_t>(st.st_size) + 1, '\0');
        const ssize_t len = ::readlink(path.c_str(), target.data(), target.size());
        if (len < 0) {
            set_error(err, "readlink");
            archive_entry_free(entry);
            return false;
        }
        target.resize(static_cast<std::size_t>(len));
        archive_entry_set_filetype(entry, AE_IFLNK);
        archive_entry_set_size(entry, 0);
        archive_entry_set_symlink(entry, target.c_str());

        if (archive_write_header(ar, entry) != ARCHIVE_OK) {
            set_archive_error(err, ar, "archive_write_header");
            archive_entry_free(entry);
            return false;
        }
        archive_entry_free(entry);
        progress.filesDone += 1;
        return true;
    }

    if (!S_ISREG(st.st_mode)) {
        archive_entry_free(entry);
        err.code = ENOTSUP;
        err.message = "Unsupported file type";
        return false;
    }

    archive_entry_set_filetype(entry, AE_IFREG);
    archive_entry_set_size(entry, st.st_size);

    if (archive_write_header(ar, entry) != ARCHIVE_OK) {
        set_archive_error(err, ar, "archive_write_header");
        archive_entry_free(entry);
        return false;
    }

    Fd fd(::open(path.c_str(), O_RDONLY | O_CLOEXEC | O_NOFOLLOW));
    if (!fd.valid()) {
        set_error(err, "open");
        archive_entry_free(entry);
        return false;
    }

    std::array<char, 128 * 1024> buffer{};
    for (;;) {
        const ssize_t n = ::read(fd.fd, buffer.data(), buffer.size());
        if (n < 0) {
            if (errno == EINTR) {
                continue;
            }
            set_error(err, "read");
            archive_entry_free(entry);
            return false;
        }
        if (n == 0) {
            break;
        }

        if (archive_write_data(ar, buffer.data(), static_cast<std::size_t>(n)) < 0) {
            set_archive_error(err, ar, "archive_write_data");
            archive_entry_free(entry);
            return false;
        }

        progress.bytesDone += static_cast<std::uint64_t>(n);
        if (!should_continue(cb, progress)) {
            err.code = ECANCELED;
            err.message = "Cancelled";
            archive_entry_free(entry);
            return false;
        }
    }

    archive_entry_free(entry);
    progress.filesDone += 1;
    return true;
}

}  // namespace

bool create_tar_zst(const std::vector<std::string>& sources,
                    const std::string& destination,
                    ProgressInfo& progress,
                    const ProgressCallback& callback,
                    Error& err) {
    progress = {};
    err = {};

    if (sources.empty()) {
        err.code = EINVAL;
        err.message = "No sources provided for compression";
        return false;
    }

    if (!ensure_parent_dirs(destination, err)) {
        return false;
    }

    // Pre-scan for total bytes to feed progress.
    std::uint64_t totalBytes = 0;
    for (const auto& src : sources) {
        if (!accumulate_size(src, totalBytes, 0, err)) {
            return false;
        }
    }
    progress.bytesTotal = totalBytes;

    Fd out_fd(::open(destination.c_str(), O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0644));
    if (!out_fd.valid()) {
        set_error(err, "open");
        return false;
    }

    struct archive* ar = archive_write_new();
    if (!ar) {
        err.code = ENOMEM;
        err.message = "Failed to allocate archive writer";
        return false;
    }

    archive_write_set_format_pax_restricted(ar);
    if (archive_write_add_filter_by_name(ar, "zstd") != ARCHIVE_OK) {
        archive_write_add_filter_none(ar);  // best-effort fallback
    }

    if (archive_write_open_fd(ar, out_fd.fd) != ARCHIVE_OK) {
        set_archive_error(err, ar, "archive_write_open_fd");
        archive_write_free(ar);
        return false;
    }

    bool ok = true;
    for (const auto& src : sources) {
        const std::string base = parent_dir(src);
        if (!write_entry(ar, src, base, progress, callback, err, 0)) {
            ok = false;
            break;
        }
    }

    if (archive_write_close(ar) != ARCHIVE_OK && ok) {
        set_archive_error(err, ar, "archive_write_close");
        ok = false;
    }
    archive_write_free(ar);

    if (!ok) {
        ::unlink(destination.c_str());
    }
    return ok;
}

namespace {

bool sanitize_relative_path(const char* raw, std::string& out) {
    if (!raw || raw[0] == '\0') {
        return false;
    }
    std::string_view view(raw);
    if (!view.empty() && view.front() == '/') {
        return false;  // no absolute paths
    }

    out.clear();
    std::vector<std::string> components;
    std::size_t start = 0;
    while (start < view.size()) {
        const auto slash = view.find('/', start);
        const std::size_t end = (slash == std::string_view::npos) ? view.size() : slash;
        std::string part(view.substr(start, end - start));
        start = (slash == std::string_view::npos) ? view.size() : slash + 1;
        if (part.empty() || part == ".") {
            continue;
        }
        if (part == "..") {
            return false;  // directory traversal not allowed
        }
        components.emplace_back(std::move(part));
    }

    for (std::size_t i = 0; i < components.size(); ++i) {
        out += components[i];
        if (i + 1 < components.size()) {
            out.push_back('/');
        }
    }
    return !out.empty();
}

bool mkdir_all_under(const std::string& root, const std::string& relPath, Error& err) {
    if (relPath.empty()) {
        return true;
    }
    std::string full = root;
    full.push_back('/');
    full += relPath;
    return FsOps::make_dir_parents(full, err);
}

bool write_all_fd_local(int fd, const std::uint8_t* data, std::size_t size, Error& err) {
    std::size_t written = 0;
    while (written < size) {
        const ssize_t n = ::write(fd, data + written, size - written);
        if (n < 0) {
            if (errno == EINTR) {
                continue;
            }
            set_error(err, "write");
            return false;
        }
        written += static_cast<std::size_t>(n);
    }
    return true;
}

bool write_file_from_archive(struct archive* ar,
                             const std::string& destPath,
                             mode_t mode,
                             ProgressInfo& progress,
                             const ProgressCallback& cb,
                             Error& err) {
    Fd fd(::open(destPath.c_str(), O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, mode & 0777));
    if (!fd.valid()) {
        set_error(err, "open");
        return false;
    }

    std::array<char, 128 * 1024> buffer{};
    for (;;) {
        const ssize_t n = archive_read_data(ar, buffer.data(), buffer.size());
        if (n > 0) {
            if (!write_all_fd_local(fd.fd, reinterpret_cast<const std::uint8_t*>(buffer.data()),
                                    static_cast<std::size_t>(n), err)) {
                return false;
            }
            progress.bytesDone += static_cast<std::uint64_t>(n);
            if (!should_continue(cb, progress)) {
                err.code = ECANCELED;
                err.message = "Cancelled";
                return false;
            }
        }
        else if (n == 0) {
            break;
        }
        else {
            set_archive_error(err, ar, "archive_read_data");
            return false;
        }
    }
    return true;
}

}  // namespace

bool extract_tar_zst(const std::string& archivePath,
                     const std::string& destinationDir,
                     ProgressInfo& progress,
                     const ProgressCallback& callback,
                     Error& err) {
    progress = {};
    err = {};

    if (archivePath.empty() || destinationDir.empty()) {
        err.code = EINVAL;
        err.message = "Invalid archive or destination path";
        return false;
    }

    // Destination directory must not already exist.
    struct stat st{};
    if (::lstat(destinationDir.c_str(), &st) == 0) {
        err.code = EEXIST;
        err.message = "Destination already exists";
        return false;
    }

    // Ensure parent exists then create destination directory.
    if (!ensure_parent_dirs(destinationDir, err)) {
        return false;
    }
    if (::mkdir(destinationDir.c_str(), 0777) < 0) {
        set_error(err, "mkdir");
        return false;
    }

    Fd rootFd(::open(destinationDir.c_str(), O_RDONLY | O_CLOEXEC | O_DIRECTORY));
    if (!rootFd.valid()) {
        set_error(err, "open");
        ::rmdir(destinationDir.c_str());
        return false;
    }

    Fd archiveFd(::open(archivePath.c_str(), O_RDONLY | O_CLOEXEC));
    if (!archiveFd.valid()) {
        set_error(err, "open");
        return false;
    }

    archive* ar = archive_read_new();
    if (!ar) {
        err.code = ENOMEM;
        err.message = "Failed to allocate archive reader";
        return false;
    }
    archive_read_support_format_tar(ar);
    archive_read_support_filter_all(ar);

    if (archive_read_open_fd(ar, archiveFd.fd, 128 * 1024) != ARCHIVE_OK) {
        set_archive_error(err, ar, "archive_read_open_fd");
        archive_read_free(ar);
        return false;
    }

    bool ok = true;
    archive_entry* entry = nullptr;
    while (archive_read_next_header(ar, &entry) == ARCHIVE_OK) {
        const char* rawPath = archive_entry_pathname(entry);
        std::string rel;
        if (!sanitize_relative_path(rawPath, rel)) {
            err.code = EINVAL;
            err.message = "Unsafe path in archive entry";
            ok = false;
            break;
        }

        const mode_t mode = archive_entry_mode(entry);
        const auto type = archive_entry_filetype(entry);
        const la_int64_t size = archive_entry_size(entry);
        if (size > 0) {
            const std::uint64_t s = static_cast<std::uint64_t>(size);
            if (s <= std::numeric_limits<std::uint64_t>::max() - progress.bytesTotal) {
                progress.bytesTotal += s;
            }
        }
        progress.currentPath = rel;
        if (!should_continue(callback, progress)) {
            err.code = ECANCELED;
            err.message = "Cancelled";
            ok = false;
            break;
        }

        std::string fullPath = destinationDir;
        fullPath.push_back('/');
        fullPath += rel;

        if (type == AE_IFDIR) {
            if (!mkdir_all_under(destinationDir, rel, err)) {
                ok = false;
                break;
            }
            ::chmod(fullPath.c_str(), mode & 0777);  // best effort
        }
        else if (type == AE_IFLNK) {
            if (!mkdir_all_under(destinationDir, parent_dir(rel), err)) {
                ok = false;
                break;
            }
            const char* link = archive_entry_symlink(entry);
            if (!link) {
                err.code = EINVAL;
                err.message = "Missing symlink target";
                ok = false;
                break;
            }
            if (::symlinkat(link, rootFd.fd, rel.c_str()) < 0) {
                set_error(err, "symlinkat");
                ok = false;
                break;
            }
        }
        else if (type == AE_IFREG) {
            if (!mkdir_all_under(destinationDir, parent_dir(rel), err)) {
                ok = false;
                break;
            }
            if (!write_file_from_archive(ar, fullPath, mode, progress, callback, err)) {
                ok = false;
                break;
            }
            ::chmod(fullPath.c_str(), mode & 0777);
        }
        else {
            err.code = ENOTSUP;
            err.message = "Unsupported entry type";
            ok = false;
            break;
        }

        // Restore times best effort
        struct timespec times[2];
        times[0].tv_sec = archive_entry_atime(entry);
        times[0].tv_nsec = archive_entry_atime_nsec(entry);
        times[1].tv_sec = archive_entry_mtime(entry);
        times[1].tv_nsec = archive_entry_mtime_nsec(entry);
        ::utimensat(AT_FDCWD, fullPath.c_str(), times, AT_SYMLINK_NOFOLLOW);

        progress.filesDone += 1;
    }

    archive_read_close(ar);
    archive_read_free(ar);

    if (!ok) {
        // best-effort cleanup of partially created destination
        FsOps::Error cleanupErr;
        ProgressInfo cleanupProg;
        FsOps::delete_path(destinationDir, cleanupProg, ProgressCallback(), cleanupErr);
    }
    return ok;
}

}  // namespace PCManFM::ArchiveWriter
