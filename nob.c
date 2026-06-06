#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#include "nob.h"

int
main(int argc, char **argv)
{
	NOB_GO_REBUILD_URSELF(argc, argv);
	bool sanitize;
	const char *cc;
	Cmd cmd;

	sanitize = false;
	if(sanitize)
		cc = "clang";
	else
		cc = "cc";

	memset(&cmd, 0, sizeof cmd);
	if(argc > 1){
		if(!strncmp(argv[1], "clean", 5)){
			cmd_append(&cmd, "rm", "-v", "sw.o", "sw");
			if (!cmd_run_sync_and_reset(&cmd))
				return -1;
			else
				return 0;
		}else{
			nob_log(NOB_ERROR, "./nob [clean]\n");
			return -1;
		}
	}

	cmd_append(&cmd, cc, "-std=c99", "-pedantic", "-Wall", "-Wextra");
#ifdef __linux__
	cmd_append(&cmd, "-I/usr/include/dbus-1.0", "-I/usr/lib/dbus-1.0/include", "-I/usr/lib64/dbus-1.0/include");
#elif __NetBSD__
	cmd_append(&cmd, "-I/usr/pkg/include/dbus-1.0", "-I/usr/pkg/lib/dbus-1.0/include");
#endif
	cmd_append(&cmd, "-c", "sw.c");
	cmd_append(&cmd, "-o", "sw.o");
	if(sanitize)
		cmd_append(&cmd, "-fsanitize=address,undefined");
	if (!cmd_run_sync_and_reset(&cmd))
		return -1;

	cmd_append(&cmd, cc, "-o", "sw", "sw.o");
	cmd_append(&cmd, "-s", "-Os");
	if(sanitize)
		cmd_append(&cmd, "-fsanitize=address,undefined");
#ifdef __NetBSD__
	cmd_append(&cmd, "-L/usr/pkg/lib");
	cmd_append(&cmd, "-Wl,-rpath=/usr/pkg/lib");
#endif
	cmd_append(&cmd, "-ldbus-1");
	if (!cmd_run_sync_and_reset(&cmd))
		return -1;

	return 0;
}
