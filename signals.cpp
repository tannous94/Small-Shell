#include <iostream>
#include <signal.h>
#include "signals.h"
#include "Commands.h"
#include <unistd.h>
#include <sys/types.h>

using namespace std;

void ctrlZHandler(int sig_num) {
	// TODO: Add your implementation
	if (SmallShell::getInstance().fg_pid != getpid()) {
		cout << "smash: got ctrl-Z" << endl;
		if (kill(SmallShell::getInstance().fg_pid, SIGSTOP) == 0) {
			SmallShell::getInstance().jobs_list->addJob(SmallShell::getInstance().fg_pid, true);
			cout << "smash: process " << SmallShell::getInstance().fg_pid << " was stopped" << endl;
		} else {
			perror("smash error: kill failed");
		}
	} else {
		cout << "smash: got ctrl-Z" << endl;
	}
}

void ctrlCHandler(int sig_num) {
  // TODO: Add your implementation
	if (SmallShell::getInstance().fg_pid != getpid()) {
		cout << "smash: got ctrl-C" << endl;
		if (kill(SmallShell::getInstance().fg_pid, SIGKILL) == 0) {
			cout << "smash: process " << SmallShell::getInstance().fg_pid << " was killed" << endl;
			//SmallShell::getInstance().fg_pid = getpid();
		} else
			perror("smash error: kill failed");
	} else {
		cout << "smash: got ctrl-C" << endl;
	}
}
