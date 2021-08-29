#include <cstdlib>
#include <cstdio>
#include <cerrno>
#include <iostream>
#include <istream>
#include <ostream>
#include <map>

#include <sys/stat.h>

#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <libutil.h>
#include <dirent.h>

#include <fcgiapp.h>

#include "klaculate.h"
#include "klac.h"
#include "util/InputHelper.h"
#include "util/OutputHelper.h"
#include "util/TokenData.h"

/*
 * Adapted from https://gist.github.com/dermesser/e2f9b66457ae19ebd116
 * Adapted from https://forums.freebsd.org/threads/sample-daemon-in-c.73059/
 */

#define _countof(arg) ( (sizeof arg) / (sizeof arg[0]) )

const char* const sockpath = "/tmp/klaculate.sock";

void* start_fcgi_worker(void* arg);
static void control(int sig);
void runFolder(std::string folder, util::OutputHelper* oh);

bool stopService = false;

const unsigned int n_threads = 4;
pthread_t threads[n_threads];

struct FCGI_Info {
	int fcgi_fd;
};

std::map<std::string, std::vector<std::string>*> *tokenplacer;

int main(int argc, char** argv) {
	std::cout << KLACULATE_NAME << " " << KLACULATE_VERSION << std::endl << std::flush;

	// precache all templates
	/* std::cout << "Precaching templates..." << std::flush;
	util::OutputHelper* oh = new util::OutputHelper();
	runFolder("", oh);
	std::cout << "\tAttempting preload..." << std::flush;
	std::string garbage = oh->getOutput();
	if (garbage.empty()) {
		std::cout << " \033[31mFAILED!\033[39m" << std::flush;
	}
	else {
		std::cout << " \033[32mSUCCESS!\033[39m" << std::flush;
		garbage.clear();
	}
	oh = nullptr;
	std::cout << std::endl << "Precache section finished." << std::endl << std::flush; // */

	struct pidfh *pfh = NULL;
	bool set_daemon = true;
	bool set_pid = true;
	bool set_traps = true;

	if (argc != 0) {
		for (int a = 0; a < argc; a++) {
			if (argv[a][0] == '-') {
				int b = strlen(argv[a]);
				for (int c = 1; c < b; c++) {
					switch (argv[a][c]) {
						case 'd': set_daemon = false;
							break;
						case 'p': set_pid = false;
							break;
						case 't': set_traps = false;
							break;
					}
				}
			}
		}
	}
	pid_t otherpid = 0;

	if (set_traps) {
		signal(SIGABRT, &control);
		signal(SIGALRM, &control);
		//signal(SIGILL, &control);
		signal(SIGFPE, &control);
		signal(SIGHUP, &control);
		signal(SIGINFO, &control);
		signal(SIGINT, &control);
		signal(SIGQUIT, &control);
		signal(SIGTERM, &control);
	}

	if (set_pid) {
		pfh = pidfile_open("/var/run/klac.pid", 0600, &otherpid);
		if (pfh == NULL) {
			if (errno == EEXIST) {
				std::cerr << "Daemon already running, pid: " << std::to_string(otherpid) << std::endl;
				exit(EXIT_FAILURE);
			}
			std::cerr << "Cannot open or create pidfile\n" << std::endl;
		}
	}

	if (set_daemon) {
		if (daemon(0, 0) == -1) {
			std::cerr << "daemon() fails." << std::endl;
			pidfile_remove(pfh);
			exit(EXIT_FAILURE);
		}
	}

	if (set_pid) {
		pidfile_write(pfh);
	}

	int fcgifd = FCGX_OpenSocket(sockpath, 128);

	chmod(sockpath, 0777);

	if (0 > fcgifd) {
		printf("Error opening socket\n");
		exit(1);
	}

	/*
	if ( FCGX_IsCGI() ) {
		printf("Please run this process as FastCGI process.\n");
		exit(1);
	}
	 */

	struct FCGI_Info info;
	info.fcgi_fd = fcgifd;

	tdInit();
	tdList();

	for (unsigned int i = 0; i < n_threads; i++) {
		pthread_create(&threads[i], NULL, start_fcgi_worker, (void*) &info);
	}

	// Wait indefinitely
	for (unsigned int i = 0; i < n_threads; i++) {
		pthread_join(threads[i], NULL);
		if (stopService) { break; }
	}

	if (set_pid) {
		pidfile_remove(pfh);
	}
	remove("/tmp/klaculate.sock");

	return 0;
}

void* start_fcgi_worker(void* arg) {
	struct FCGI_Info* info = (struct FCGI_Info*) arg;

	FCGX_Init();

	FCGX_Request request;

	FCGX_InitRequest(&request, info->fcgi_fd, 0);
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
	while (!stopService) {
		pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
		FCGX_Accept_r(&request);
		pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

		runtask(&request);

		FCGX_Finish_r(&request);
	}
	return 0;
}

static void control(int sig) {
	static const char *labels[] = {
		"-", "SIGHUP", "SIGINT", "SIGQUIT", "SIGILL", "SIGTRAP", "SIGABRT", "SIGEMT", "SIGFPE", "SIGKILL", "SIGBUS", "SIGSEGV", "SIGSYS", "SIGPIPE", "SIGALRM", "SIGTERM"
	};
	if (sig > 0 && sig < (int) _countof(labels))
		fprintf(stderr, "Captured %s.\n", labels[sig]);
	else
		fprintf(stderr, "Something bad %d happened.\n", sig);

	if (sig == SIGINT || sig == SIGQUIT || sig == SIGTERM) {
		FCGX_ShutdownPending();
		stopService = true;
		for (unsigned int a = 0; a < n_threads; a++) {
			pthread_cancel(threads[a]);
		}
	}
}

void runFolder(std::string folder, util::OutputHelper * oh) {
	DIR* dir;
	struct dirent* ent;
	std::string fname("/usr/home/klaculate/templates/");
	fname.append(folder);

	if ((dir = opendir(fname.c_str())) != NULL) {
		while ((ent = readdir(dir)) != NULL) {
			if (ent->d_name[0] != '.') {
				std::string partname(folder);
				if (!folder.empty()) {
					partname.append("/");
				}
				partname.append(ent->d_name);

				std::string fullname(fname);
				if (!folder.empty()) {
					fullname.append("/");
				}
				fullname.append(ent->d_name);
				struct stat fs;
				int result = stat(fullname.c_str(), &fs);
				if (result == 0) {
					if (S_ISDIR(fs.st_mode)) {
						runFolder(partname, oh);
					}
					else if (S_ISREG(fs.st_mode)) {
						oh->addTemplate(partname);
					}
				}
			}
		}
		closedir(dir);
	}
	free(dir);
	free(ent);
}
