#define FUSE_USE_VERSION 31

#include <fuse.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <dirent.h>
#include <libgen.h>

void log_action(const char *status, const char *tag, const char *information) {
    FILE *log_file = fopen("logs-fuse.log", "a");
    if (log_file) {
        time_t now = time(NULL);
        struct tm *t = localtime(&now);
        fprintf(log_file, "[%s]::%02d/%02d/%04d-%02d:%02d:%02d::[%s]::[%s]\n",
                status,
                t->tm_mday, t->tm_mon + 1, t->tm_year + 1900,
                t->tm_hour, t->tm_min, t->tm_sec,
                tag, information);
        fclose(log_file);
    }
}

void base64_decode(const char *encoded, char *decoded) {
    FILE *pipe = popen("echo -n | base64 -d", "r");
    fwrite(encoded, 1, strlen(encoded), pipe);
    pclose(pipe);
}

void rot13_decode(char *text) {
    for (int i = 0; text[i]; i++) {
        if ('a' <= text[i] && text[i] <= 'z') {
            text[i] = 'a' + (text[i] - 'a' + 13) % 26;
        } else if ('A' <= text[i] && text[i] <= 'Z') {
            text[i] = 'A' + (text[i] - 'A' + 13) % 26;
        }
    }
}

int hex_value(char c) {
    if ('0' <= c && c <= '9') return c - '0';
    if ('a' <= c && c <= 'f') return c - 'a' + 10;
    if ('A' <= c && c <= 'F') return c - 'A' + 10;
    return -1;
}

void hex_decode(const char *encoded, char *decoded) {
    while (*encoded && encoded[1]) {
        *(decoded++) = (char)((hex_value(*encoded) << 4) | hex_value(encoded[1]));
        encoded += 2;
    }
    *decoded = '\0';
}

void reverse_text(char *text) {
    int len = strlen(text);
    for (int i = 0; i < len / 2; i++) {
        char temp = text[i];
        text[i] = text[len - i - 1];
        text[len - i - 1] = temp;
    }
}

static char *construct_path(const char *path) {
    static char full_path[512];
    snprintf(full_path, sizeof(full_path), "%s%s", "/home/fadhils/inikaryakita/pesan", path);
    return full_path;
}

static const char *correct_password = "Sempol";
static int authenticated = 0;

static int fs_getattr(const char *path, struct stat *stbuf, struct fuse_file_info *fi) {
    (void) fi;
    int res;
    char *full_path = construct_path(path);

    res = lstat(full_path, stbuf);
    if (res == -1)
        return -errno;

    return 0;
}

static int fs_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi, enum fuse_readdir_flags flags) {
    (void) offset;
    (void) fi;

    DIR *dp;
    struct dirent *de;
    struct stat st;
    char *full_path = construct_path(path);

    dp = opendir(full_path);
    if (dp == NULL)
        return -errno;

    while ((de = readdir(dp)) != NULL) {
        memset(&st, 0, sizeof(st));
        st.st_ino = de->d_ino;
        st.st_mode = de->d_type << 12;

        if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0) {
            continue;
        }

        if (filler(buf, de->d_name, &st, 0, 0)) {
            break;
        }
    }

    closedir(dp);
    return 0;
}

static int fs_open(const char *path, struct fuse_file_info *fi) {
    if (strncmp(path, "/rahasia-berkas", 15) == 0 && !authenticated) {
        log_action("FAILED", "open", path);
        return -EACCES;
    }

    int res;
    char *full_path = construct_path(path);

    res = open(full_path, fi->flags);
    if (res == -1)
        return -errno;

    close(res);
    return 0;
}

static int fs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
    if (strncmp(path, "/rahasia-berkas", 15) == 0 && !authenticated) {
        log_action("FAILED", "read", path);
        return -EACCES;
    }

    int fd;
    int res;
    char *full_path = construct_path(path);

    fd = open(full_path, O_RDONLY);
    if (fd == -1)
        return -errno;

    res = pread(fd, buf, size, offset);
    if (res == -1)
        res = -errno;

    close(fd);

    char *filename = basename(full_path);
    if (strncmp(filename, "base64_", 7) == 0) {
        char decoded[1024];
        base64_decode(buf, decoded);
        strcpy(buf, decoded);
    } else if (strncmp(filename, "rot13_", 6) == 0) {
        rot13_decode(buf);
    } else if (strncmp(filename, "hex_", 4) == 0) {
        char decoded[1024];
        hex_decode(buf, decoded);
        strcpy(buf, decoded);
    } else if (strncmp(filename, "rev_", 4) == 0) {
        reverse_text(buf);
    }

    log_action("SUCCESS", "read", path);
    return res;
}

static struct fuse_operations fs_oper = {
    .getattr = fs_getattr,
    .readdir = fs_readdir,
    .open = fs_open,
    .read = fs_read,
};

int main(int argc, char *argv[]) {
    umask(0);

    char password[256];
    printf("Enter password to access 'rahasia-berkas': ");
    scanf("%255s", password);
    if (strcmp(password, correct_password) == 0) {
        authenticated = 1;
        log_action("SUCCESS", "authenticate", "User authenticated successfully");
    } else {
        log_action("FAILED", "authenticate", "User authentication failed");
    }

    return fuse_main(argc, argv, &fs_oper, NULL);
}
