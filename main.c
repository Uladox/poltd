#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>
#include <sys/stat.h>
#include <stdio.h>
#include <pthread.h>
#include <stdint.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <linux/limits.h>
#include <string.h>

#define NIT_SHORT_NAMES
#include <nit/socket.h>

int
main(int argc, char *argv[])
{
	pid_t pid, sid;
	Nit_joiner *jnr;
	Nit_joint  *jnt;

        char polt_name[] = "polt/";
	char io_end[]    = ".io";
	int name_parts = sizeof(polt_name) + sizeof(io_end) - 2;
	char jnr_name[NAME_MAX];

	if (argc < 3) {
		fprintf(stderr, "Too few arguments\n");
		exit(EXIT_FAILURE);
	}

	if (strlen(argv[2]) >= NAME_MAX - name_parts) {
		fprintf(stderr, "Filename \"%s\" too long\n", argv[2]);
		exit(EXIT_FAILURE);
	}

	strcpy(jnr_name, polt_name);
	strcpy(jnr_name, argv[2]);
	strcat(jnr_name, io_end);

	if (chdir(argv[1]) < 0) {
		fprintf(stderr, "Could not change working directory to %s\n",
			argv[1]);
		exit(EXIT_FAILURE);
	}

	if (!(jnr = joiner_new(jnr_name))) {
		fprintf(stderr, "Failed to start io sockets\n");
		exit(EXIT_FAILURE);
	}

	pid = fork();

	if (pid < 0)
		exit(EXIT_FAILURE);
	else if (pid > 0)
		exit(EXIT_SUCCESS);

	umask(0);

	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);

	openlog(argv[0], LOG_NOWAIT | LOG_PID, LOG_USER);
	syslog(LOG_NOTICE, "successfully started daemon\n");

	if ((sid = setsid()) < 0) {
		syslog(LOG_ERR, "could not create process group\n");
		exit(EXIT_FAILURE);
	}

	sleep(10);

	joiner_free(jnr);

	syslog(LOG_NOTICE, "successfully exited daemon\n");
	exit(EXIT_SUCCESS);
}
