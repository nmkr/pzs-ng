#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>

#include "config.h"

#include "objects.h"
#include "macros.h"
#include "../conf/zsconfig.h"
#include "../include/zsconfig.defaults.h"

struct stat	entry_stat;
int		zd_length;

#ifndef PATH_MAX
#define _LIMITS_H_
#include <sys/syslimits.h>
#endif

void 
remove_dir_loop(char *path)
{
	struct dirent **list;
	int		n;
	char		target    [PATH_MAX];

	chdir(path);
	n = scandir(path, &list, 0, 0);
	while (n--)
		if (list[n]->d_name[0] != '.') {
			stat(list[n]->d_name, &entry_stat);
			if (S_ISDIR(entry_stat.st_mode)) {
				sprintf(target, "%s/%s", path, list[n]->d_name);
				remove_dir_loop(target);
				rmdir(target);
				chdir(path);
			} else
				unlink(list[n]->d_name);
			free(list[n]);
		}
	free(list);
}

void 
check_dir_loop(char *path)
{
	struct dirent **list;
	int		n;
	char		target    [PATH_MAX];
	DIR            *dirp;

	chdir(path);
	n = scandir(path, &list, 0, 0);
	if (n < 2)
		exit(2);
	while (n--)
		if (list[n]->d_name[0] != '.') {
			stat(list[n]->d_name, &entry_stat);
			if (S_ISDIR(entry_stat.st_mode)) {
				sprintf(target, "%s/%s", path, list[n]->d_name);
				if ((dirp = opendir(target + zd_length))) {
					closedir(dirp);
					check_dir_loop(target);
					chdir(path);
				} else {
					remove_dir_loop(target);
					rmdir(target);
					chdir(path);
				}
			}
			free(list[n]);
		}
	free(list);
}

int 
main(int argc, char **argv)
{
	char		st        [PATH_MAX];

	zd_length = strlen(storage);

	if (argc == 1) {
		check_dir_loop(storage);
	} else {
		if ((zd_length + 1 + strlen(argv[1])) < PATH_MAX) {
			if ( !strncmp(argv[1], "RMD ", 4)) {
				sprintf(st, storage "/%s/%s", sitepath_dir, argv[1] + 4);
			} else {
				sprintf(st, storage "/%s", argv[1]);
			}
		}
		/* check subdirs */
printf("removing dir: %s\n",st);
		check_dir_loop(st);

		/* check current dir */
		if (!opendir(st + zd_length)) {
			remove_dir_loop(st);
			rmdir(st);
		}
	}
	return 0;
}
