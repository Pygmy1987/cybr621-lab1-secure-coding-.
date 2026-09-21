/*
 * safecopy.c - Securely copy one file to another using only POSIX
 * system calls (open, read, write, close).
 *
 * Usage: safecopy <source> <destination>
 *
 * Security properties:
 *   - Refuses to overwrite an existing destination (O_EXCL).
 *   - Destination created with mode 0600 (owner read/write only),
 *     subject to the process umask.
 *   - No libc string/formatting functions are used on untrusted data.
 *   - All reads/writes handle short counts and EINTR.
 *   - All file descriptors are closed before the program exits.
 *
 * Compile: gcc -Wall -Wextra -O2 -o safecopy safecopy.c
 */

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#define IO_BUF_SIZE 65536

/* Write a NUL-terminated string literal to a given fd without using
 * any libc formatting/string functions. Length is computed manually
 * so we don't depend on strlen(). Best-effort; ignores partial-write
 * edge cases on stderr since this is only used for diagnostics. */
static void write_msg(int fd, const char *msg)
{
    size_t len = 0;
    while (msg[len] != '\0')
        len++;

    size_t off = 0;
    while (off < len) {
        ssize_t w = write(fd, msg + off, len - off);
        if (w < 0) {
            if (errno == EINTR)
                continue;
            return;
        }
        off += (size_t)w;
    }
}

/* Read exactly one buffer's worth (or less, at EOF), retrying on
 * EINTR and handling partial reads. Returns number of bytes read
 * (0 at EOF), or -1 on error (errno set). */
static ssize_t read_full(int fd, void *buf, size_t count)
{
    size_t total = 0;
    unsigned char *p = (unsigned char *)buf;

    while (total < count) {
        ssize_t n = read(fd, p + total, count - total);
        if (n < 0) {
            if (errno == EINTR)
                continue;
            return -1;
        }
        if (n == 0)
            break; /* EOF */
        total += (size_t)n;
    }
    return (ssize_t)total;
}

/* Write exactly count bytes, retrying on EINTR and handling partial
 * writes. Returns 0 on success, -1 on error (errno set). */
static int write_full(int fd, const void *buf, size_t count)
{
    size_t total = 0;
    const unsigned char *p = (const unsigned char *)buf;

    while (total < count) {
        ssize_t n = write(fd, p + total, count - total);
        if (n < 0) {
            if (errno == EINTR)
                continue;
            return -1;
        }
        total += (size_t)n;
    }
    return 0;
}

/* close() retrying on EINTR. On Linux, a failed close() (other than
 * EINTR) must not be retried, since the fd is unconditionally
 * released by the kernel; we just report the error. */
static int close_retry(int fd)
{
    int rc;
    do {
        rc = close(fd);
    } while (rc < 0 && errno == EINTR);
    return rc;
}

int main(int argc, char *argv[])
{
    if (argc != 3) {
        write_msg(STDERR_FILENO, "Usage: safecopy <source> <destination>\n");
        return 1;
    }

    const char *src_path = argv[1];
    const char *dst_path = argv[2];

    int src_fd = -1;
    int dst_fd = -1;
    int exit_code = 0;

    /* Open source read-only. */
    do {
        src_fd = open(src_path, O_RDONLY);
    } while (src_fd < 0 && errno == EINTR);

    if (src_fd < 0) {
        write_msg(STDERR_FILENO, "safecopy: failed to open source file\n");
        return 1;
    }

    /* Refuse to follow a symlink at the destination path and refuse
     * to overwrite an existing destination: O_CREAT|O_EXCL fails
     * atomically if the path already exists (including as a
     * symlink). Mode 0600 restricts access to the owner; the
     * effective mode is further limited by the process umask. */
    do {
        dst_fd = open(dst_path, O_WRONLY | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
    } while (dst_fd < 0 && errno == EINTR);

    if (dst_fd < 0) {
        if (errno == EEXIST)
            write_msg(STDERR_FILENO, "safecopy: destination file already exists\n");
        else
            write_msg(STDERR_FILENO, "safecopy: failed to create destination file\n");
        close_retry(src_fd);
        return 1;
    }

    unsigned char buf[IO_BUF_SIZE];

    for (;;) {
        ssize_t nread = read_full(src_fd, buf, sizeof(buf));

        if (nread < 0) {
            write_msg(STDERR_FILENO, "safecopy: error reading source file\n");
            exit_code = 1;
            break;
        }

        if (nread == 0)
            break; /* EOF, copy complete */

        if (write_full(dst_fd, buf, (size_t)nread) < 0) {
            write_msg(STDERR_FILENO, "safecopy: error writing destination file\n");
            exit_code = 1;
            break;
        }

        if ((size_t)nread < sizeof(buf))
            break; /* short read means EOF was reached */
    }

    /* Always attempt to close both descriptors before exiting,
     * regardless of what happened above. A failed close() on the
     * destination (e.g. delayed write-back error) is treated as a
     * copy failure. */
    if (close_retry(src_fd) < 0) {
        write_msg(STDERR_FILENO, "safecopy: error closing source file\n");
        exit_code = 1;
    }

    if (close_retry(dst_fd) < 0) {
        write_msg(STDERR_FILENO, "safecopy: error closing destination file\n");
        exit_code = 1;
    }

    return exit_code;
}