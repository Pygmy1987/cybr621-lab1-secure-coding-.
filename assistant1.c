#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <sys/types.h>
#include <unistd.h>

#define BUFFER_SIZE 4096

static void write_message(const char *message, size_t length)
{
	size_t offset = 0;

	while (offset < length) {
		ssize_t written = write(STDERR_FILENO, message + offset, length - offset);

		if (written > 0) {
			offset += (size_t)written;
		} else if (written < 0 && errno == EINTR) {
			continue;
		} else {
			break;
		}
	}
}

static int open_for_reading(const char *path)
{
	int descriptor;

	do {
		descriptor = open(path, O_RDONLY);
	} while (descriptor < 0 && errno == EINTR);

	return descriptor;
}

static int open_for_creation(const char *path)
{
	int descriptor;

	do {
		descriptor = open(path, O_WRONLY | O_CREAT | O_EXCL, 0600);
	} while (descriptor < 0 && errno == EINTR);

	return descriptor;
}

static int copy_contents(int source, int destination)
{
	char buffer[BUFFER_SIZE];

	for (;;) {
		ssize_t bytes_read;
		size_t offset;

		do {
			bytes_read = read(source, buffer, sizeof(buffer));
		} while (bytes_read < 0 && errno == EINTR);

		if (bytes_read == 0) {
			return 0;
		}
		if (bytes_read < 0) {
			return -1;
		}

		offset = 0;
		while (offset < (size_t)bytes_read) {
			ssize_t bytes_written;

			do {
				bytes_written = write(destination, buffer + offset,
									   (size_t)bytes_read - offset);
			} while (bytes_written < 0 && errno == EINTR);

			if (bytes_written <= 0) {
				return -1;
			}
			offset += (size_t)bytes_written;
		}
	}
}

int main(int argc, char *argv[])
{
	int source = -1;
	int destination = -1;
	int status = 1;
	static const char usage[] = "Usage: assistant1 SOURCE DESTINATION\n";
	static const char open_source_error[] = "Error: cannot open source file\n";
	static const char open_destination_error[] =
		"Error: destination exists or cannot be created\n";
	static const char copy_error[] = "Error: file copy failed\n";
	static const char close_error[] = "Error: cannot close file\n";

	if (argc != 3) {
		write_message(usage, sizeof(usage) - 1);
		return 1;
	}

	source = open_for_reading(argv[1]);
	if (source < 0) {
		write_message(open_source_error, sizeof(open_source_error) - 1);
		return 1;
	}

	destination = open_for_creation(argv[2]);
	if (destination < 0) {
		write_message(open_destination_error, sizeof(open_destination_error) - 1);
		goto cleanup;
	}

	if (copy_contents(source, destination) < 0) {
		write_message(copy_error, sizeof(copy_error) - 1);
		goto cleanup;
	}

	status = 0;

cleanup:
	if (destination >= 0 && close(destination) < 0) {
		write_message(close_error, sizeof(close_error) - 1);
		status = 1;
	}
	if (source >= 0 && close(source) < 0) {
		write_message(close_error, sizeof(close_error) - 1);
		status = 1;
	}

	return status;
}
