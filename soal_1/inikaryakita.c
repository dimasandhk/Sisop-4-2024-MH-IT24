#define FUSE_USE_VERSION 31

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <assert.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/xattr.h>
#include <stdlib.h>

static const char *dirpath = "/home/dim/uni/sisop/Sisop-4-2024-MH-IT24/soal_1/portofolio";

static int hello_getattr(const char *path, struct stat *stbuf, struct fuse_file_info *fi) {
    char fpath[1000];
    sprintf(fpath,"%s%s",dirpath,path);
    int res;

    res = lstat(fpath, stbuf);
    if (res == -1)
        return -errno;

    return 0;
}

static int hello_mkdir(const char *path, mode_t mode) {
    char fpath[1000];
    sprintf(fpath,"%s%s",dirpath,path);
    int res;

    res = mkdir(fpath, mode);
    if(res == -1)
        return -errno;

    return 0;
}

static int hello_rmdir(const char *path) {
    char fpath[1000];
    sprintf(fpath,"%s%s",dirpath,path);
    int res = rmdir(fpath);
    if (res == -1) {
        return -errno;
    }
    return 0;
}

static int hello_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {
    char fpath[1000];
    sprintf(fpath,"%s%s",dirpath,path);

    DIR *dp;
    struct dirent *de;

    dp = opendir(fpath);
    if (dp == NULL)
        return -errno;

    while ((de = readdir(dp)) != NULL) {
        struct stat st;
        memset(&st, 0, sizeof(st));
        st.st_ino = de->d_ino;
        st.st_mode = de->d_type << 12;
        if (filler(buf, de->d_name, &st, 0))
            break;
    }

    closedir(dp);
    return 0;
}

static int hello_rename(const char *from, const char *to, unsigned int flags) {
    char fromPath[1000], toPath[1000];
    sprintf(fromPath,"%s%s",dirpath,from);
    sprintf(toPath,"%s%s",dirpath,to);

    if (strstr(toPath, "/wm") != NULL) {
        int srcfd = open(fromPath, O_RDONLY);
        int destfd = open(toPath, O_WRONLY | O_CREAT | O_TRUNC, 0644);

        char command[1000];
        sprintf(command, "convert -gravity south -font Arial /proc/%d/fd/%d -fill white -pointsize 50 -annotate +0+0 '%s' /proc/%d/fd/%d", getpid(), srcfd, "inikaryakita.id", getpid(), destfd);

        int res = system(command);
        if (res == -1) {
            perror("gabisa jalanin imagemagick");
            close(srcfd);
            close(destfd);
            return -errno;
        }

        close(srcfd);
        close(destfd);

        if (unlink(fromPath) == -1) {
            perror("Error: Failed to remove the source file.");
            return -errno;
        }
    } else {
        if (rename(fromPath, toPath) == -1) {
            perror("gabisa move");
            return -errno;
        }
    } 
    // sprintf(command, "convert %s -font Arial -pointsize 20 -draw \"gravity south fill black text 0,12 'inikaryakita.id' fill white text 1,11 'inikaryakita.id'\" %s", frompath, temp);
    return 0;
}

static int hello_unlink(const char *path) {
    char fpath[1000];
    sprintf(fpath,"%s%s",dirpath,path);
    int res = unlink(fpath);
    if (res == -1) {
        return -errno;
    }
    return 0;
}

static int hello_chmod(const char *path, mode_t mode) {
    char fpath[1000];
    sprintf(fpath,"%s%s",dirpath,path);
    if(chmod(fpath, mode) < 0)
        return -errno;
    return 0;
}

static int hello_read(const char *path, char *buf, size_t size, off_t offset,
                      struct fuse_file_info *fi) {
    char fpath[1000];
    sprintf(fpath,"%s%s",dirpath,path);

    // Check if the file has a 'test' prefix
    const char *filename = strrchr(path, '/');
    if (filename == NULL) {
        filename = path;
    } else {
        filename++;
    }

    if (strncmp(filename, "test", 4) == 0) {
        // If the file has a 'test' prefix, reverse the words in the lines before output
        FILE *file = fopen(fpath, "r");
        if (file == NULL) {
            return -errno;
        }

        char line[1024];
        buf[0] = '\0';  // Initialize the buffer to an empty string
        while (fgets(line, sizeof(line), file) != NULL) {
            size_t len = strlen(line);
            if (line[len - 1] == '\n') {
                line[len - 1] = '\0';
            }

            // Reverse the characters in each word
            char *start = line;
            char *end = start + strlen(line) - 1;
            while (end > start) {
                char temp = *start;
                *start = *end;
                *end = temp;
                start++;
                end--;
            }

            // Check if the buffer is large enough
            if (strlen(buf) + strlen(line) + 1 > size) {
                fclose(file);
                return -ENOMEM;
            }

            strcat(buf, line);
            strcat(buf, "\n");
        }

        fclose(file);
    } else {
        // If the file doesn't have a 'test' prefix, output normally
        int fd = open(fpath, O_RDONLY);
 if (fd == -1) {
            return -errno;
        }

        int res = pread(fd, buf, size, offset);
        if (res == -1) {
            res = -errno;
        }

        close(fd);
        return res;
    }

    return strlen(buf);
}

static struct fuse_operations hello_oper = {
    .getattr    = hello_getattr,
    .mkdir      = hello_mkdir,
    .rmdir      = hello_rmdir,
    .readdir    = hello_readdir,
    .rename     = hello_rename,
    .chmod      = hello_chmod,
    .read       = hello_read,
    .unlink     = hello_unlink,
};

int main(int argc, char *argv[]) {
    umask(0);
    return fuse_main(argc, argv, &hello_oper, NULL);
}