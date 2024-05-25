#define FUSE_USE_VERSION 31
#define D_FILE_OFFSET_BITS 64

#include <fuse3/fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

int picat_getattr(const char *path, struct stat *stbuf, struct fuse_file_info *fi) 
{
	(void) fi;
	int res = 0;
	
	memset(stbuf, 0 ,sizeof(struct stat));
	if (strcmp(path, "/") == 0) {
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
	} else if (strncmp(path+1, "relic_", 6) == 0){
		stbuf->st_mode = S_IFREG | 0640;
		stbuf->st_nlink = 1;
		stbuf->st_size = 1024;
	} else 
		return -ENOENT;

	return res;
}


int picat_readdir(const char *path, void *buf,
		    fuse_fill_dir_t filler, off_t off,
		    struct fuse_file_info *fi, enum fuse_readdir_flags flags) {
	(void) off;
	(void) fi;
	(void) flags;

	if (strcmp(path, "/") != 0)
		return -ENOENT;

	filler(buf,".", NULL, 0, FUSE_FILL_DIR_DEFAULTS);
	filler(buf,"..", NULL, 0, FUSE_FILL_DIR_DEFAULTS);

	for (int i = 1; i <= 10; i++) {
		char fname[15];
		snprintf(fname, 15, "relic_%d.png", i);
		filler(buf, fname, NULL, 0, FUSE_FILL_DIR_DEFAULTS);
	}
	
	return 0;
}


size_t fsize(char *fname) 
{
	FILE *fp = fopen(fname, "r");
	fseek(fp, 0L, SEEK_END);
	size_t size = (size_t) ftell(fp);
	return size;
}


void picat(char const *fname) 
{
	char destpath[50];
	snprintf(destpath, 50, "./test/%s", fname);
	FILE *dest;
	/*
	for(int i = 0; i <= 6; i++) {
		char pcname[18], pcpath[50];
		snprintf(pcname, 18, "%s.00%d", fname, i);
		snprintf(pcpath, 50, "./relics/%s", pcname);
		FILE *piece = fopen(pcpath, "rb");
		fseek(piece, 0, SEEK_SET);
		dest = fopen(destpath,"ab");
		int pclen = fsize(pcpath);
		for(int i = 0; i < pclen; i++)
			fputc(fgetc(piece), dest);
		fclose(piece);
	}
	
	fclose(dest); */
}

int picat_read(const char *path, char *buf, size_t size, off_t offset,
		struct fuse_file_info *fi) 
{
	(void) fi;
	if(strncmp(path+1, "relic_", 6) != 0)
		return -ENOENT;
	
	//funny(path+1);

	return size;
}


static const struct fuse_operations picat_oper = {
	.getattr	= picat_getattr,
	.readdir	= picat_readdir,
	.read		= picat_read,
};

int main(int argc, char *argv[])
{
	int ret;
	struct fuse_args args = FUSE_ARGS_INIT(argc, argv);

	ret = fuse_main(args.argc, args.argv, &picat_oper, NULL);
	fuse_opt_free_args(&args);
	return ret;
}
