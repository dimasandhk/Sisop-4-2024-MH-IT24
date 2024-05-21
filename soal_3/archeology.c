#define FUSE_USE_VERSION 31
#define D_FILE_OFFSET_BITS 64

#include <fuse3/fuse.h>
#include <stdio.h>

int fuse_getattr(const char *path, struct stat *statbuff,
		struct fuse_file_info *fi) {
	printf("getattr for %s BIG BOOTY BANAYNAY\n", path);
	return 0;
}

static const struct fuse_operations hello_oper = {
	.getattr	= fuse_getattr,
};

int main(int argc, char *argv[])
{
	int ret;
	struct fuse_args args = FUSE_ARGS_INIT(argc, argv);

	/* Parse options */
	if (fuse_opt_parse(&args, NULL, NULL, NULL) == -1)
		return 1;

	ret = fuse_main(args.argc, args.argv, &hello_oper, NULL);
	fuse_opt_free_args(&args);
	return ret;
}
