#include <unistd.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
#include "Commands.h"
#include <fcntl.h>

using namespace std;

#if 0
#define FUNC_ENTRY()  \
  cout << __PRETTY_FUNCTION__ << " --> " << endl;

#define FUNC_EXIT()  \
  cout << __PRETTY_FUNCTION__ << " <-- " << endl;
#else
#define FUNC_ENTRY()
#define FUNC_EXIT()
#endif

int _parseCommandLine(const char* cmd_line, char** args) {
  FUNC_ENTRY()
  stringstream check1(cmd_line);
  string intermediate;
  int i = 0;
  while(getline(check1, intermediate, ' '))
  {
    args[i] = (char*)malloc(intermediate.length()+1);
    memset(args[i], 0, intermediate.length()+1);
    strcpy(args[i], intermediate.c_str());
    args[++i] = NULL;
  }
  return i;

  FUNC_EXIT()
}

bool _isBackgroundComamnd(const char* cmd_line) {
  const string whitespace = " \t\n";
  const string str(cmd_line);
  return str[str.find_last_not_of(whitespace)] == '&';
}

void _removeBackgroundSign(char* cmd_line) {
  const string whitespace = " \t\n";
  const string str(cmd_line);
  // find last character other than spaces
  size_t idx = str.find_last_not_of(whitespace);
  // if all characters are spaces then return
  if (idx == string::npos) {
    return;
  }
  // if the command line does not end with & then return
  if (cmd_line[idx] != '&') {
    return;
  }
  // replace the & (background sign) with space and then remove all tailing spaces.
  cmd_line[idx] = ' ';
  // truncate the command line string up to the last non-space character
  cmd_line[str.find_last_not_of(whitespace, idx-1) + 1] = 0;
}

// TODO: Add your implementation for classes in Commands.h
Command::Command(const char* cmd_line) {

}
Command::~Command() {

}

BuiltInCommand::BuiltInCommand(const char* cmd_line) : Command::Command(cmd_line) {

}

ExternalCommand::ExternalCommand(const char* cmd_line) : Command::Command(cmd_line) {

}

GetCurrDirCommand::GetCurrDirCommand(const char* cmd_line) : BuiltInCommand::BuiltInCommand(cmd_line) {

}

CopyCommand::CopyCommand(const char* cmd_line) : BuiltInCommand::BuiltInCommand(cmd_line) {

}

JobsCommand::JobsCommand(const char* cmd_line, JobsList* jobs) : BuiltInCommand::BuiltInCommand(cmd_line) {

}

BackgroundCommand::BackgroundCommand(const char* cmd_line, JobsList* jobs) : BuiltInCommand::BuiltInCommand(cmd_line) {

}

ForegroundCommand::ForegroundCommand(const char* cmd_line, JobsList* jobs) : BuiltInCommand::BuiltInCommand(cmd_line) {

}

QuitCommand::QuitCommand(const char* cmd_line, JobsList* jobs) : BuiltInCommand::BuiltInCommand(cmd_line) {

}

ChangeDirCommand::ChangeDirCommand(const char* cmd_line, char** plastPwd) : BuiltInCommand::BuiltInCommand(cmd_line) {

}

HistoryCommand::HistoryCommand(const char* cmd_line, CommandsHistory* history) : BuiltInCommand::BuiltInCommand(cmd_line) {

}

KillCommand::KillCommand(const char* cmd_line, JobsList* jobs) : BuiltInCommand::BuiltInCommand(cmd_line) {
	jobs_list = jobs;
}

ShowPidCommand::ShowPidCommand(const char* cmd_line) : BuiltInCommand::BuiltInCommand(cmd_line) {

}

JobsList::JobsList() {

}

void JobsList::addJob(pid_t pid, bool isStopped) {
	  list<JobEntry>::iterator it;
	  for (it = jobs.begin(); it != jobs.end(); it++) {
		  if (it->curr_pid == pid) {
			  	  it->finished = 0; //hon
				  it->time_begin = time(NULL);
				  //int status; //hon
				  if (isStopped)
					  it->stopped = 1; //hon
				  else
					  it->stopped = 0;
				  return;
		  }

	  }
	  list<JobEntry>::iterator tt = jobs.end();
	  tt--;
	  int id = tt->job_id + 1;
	  string str = SmallShell::getInstance().curr_line;
	  if (isStopped) {
		  JobEntry job(id, str, pid, time(NULL), 1, 0);
		  jobs.push_back(job);
		  //SmallShell::getInstance().jobs_list->jobs.push_back(job);
	  } else {
		  JobEntry job(id, str, pid, time(NULL), 0, 0);
		  jobs.push_back(job);
		  //SmallShell::getInstance().jobs_list->jobs.push_back(job);
	  }
  }

void JobsList::removeFinishedJobs() {
	  list<JobEntry>::iterator it;
	  int status;
	  	  for (it = jobs.begin(); it != jobs.end(); it++) {
	  		if ( waitpid(it->curr_pid, &status, WNOHANG|WUNTRACED) > 0) {
	  			// if (/*(WIFEXITED(status)) || *//*(it->finished == 0)*/) { //hon SO NOT SURE
	  			if (!WIFSTOPPED(status))
	  				it = jobs.erase(it);
	  			// }
	  		}
	  	  }
}

void ExternalCommand::execute() {
	string cmd_str = "";
	int bg_flag = 0;
	if (_isBackgroundComamnd(SmallShell::getInstance().curr_line.c_str()))
		bg_flag = 1;
	for (unsigned int i = 0; i < SmallShell::getInstance().getCmdVec().size(); i++) {
		if (i == SmallShell::getInstance().getCmdVec().size() - 1) {
			if (SmallShell::getInstance().getCmdVec()[i] == "&") {
				break;
			} else {
				string str = SmallShell::getInstance().getCmdVec()[i];
				if (str[str.size() - 1] == '&') {
					string op = "";
					for (unsigned int j = 0; j < str.size() - 1; j++)
						op += str[j];
					cmd_str += op;
					cmd_str += " ";
					break;
				}
			}
		}
		cmd_str += SmallShell::getInstance().getCmdVec()[i];
		cmd_str += " ";
	}
	string b("bash");
	string c("-c");
	char* ext[4] = {(char*)b.c_str(), (char*)c.c_str(), (char*)cmd_str.c_str(), NULL};
	pid_t son_pid = fork();
	SmallShell::getInstance().fg_pid = son_pid;
	if (son_pid == 0) {
		setpgrp();
		execv("/bin/bash", ext);
	} else {
		int status;
		if (!bg_flag)
			waitpid(SmallShell::getInstance().fg_pid, &status, WUNTRACED);
		else {
			SmallShell::getInstance().jobs_list->addJob(SmallShell::getInstance().fg_pid, false);
			waitpid(SmallShell::getInstance().fg_pid, &status,WNOHANG|WUNTRACED);

		}
	}
	return;
}

void GetCurrDirCommand::execute() {
	char cwd[PATH_MAX];
	if (getcwd(cwd, sizeof(cwd)) != NULL) {
		//SmallShell::getInstance().plastPwd = cwd;
		cout << cwd << endl;
	} else {
		perror("smash error: getcwd failed");
		return;
	}
	return;
}

void ChangeDirCommand::execute() {
	const char* newPwd = SmallShell::getInstance().getCmdVec()[1].c_str();
	const char* line = "-";
	char cwd[PATH_MAX];
	if (SmallShell::getInstance().getCmdVec().size() > 2) {
		cout << "smash error: cd: too many arguments" << endl;
		return;
	}
	if (strcmp(newPwd, line) == 0) {
		if (strcmp(SmallShell::getInstance().prevPwd, "") == 0) {
			cout << "smash error: cd: OLDPWD not set" << endl;
		} else {
			if (chdir(SmallShell::getInstance().prevPwd) == -1) {
				perror("smash error: chdir failed");
				return;
			} else {
				char temp[PATH_MAX];
				strcpy(temp, SmallShell::getInstance().currPwd);
				strcpy(SmallShell::getInstance().currPwd, SmallShell::getInstance().prevPwd);
				strcpy(SmallShell::getInstance().prevPwd, temp);
			}
		}
	} else {
		strcpy(SmallShell::getInstance().prevPwd, SmallShell::getInstance().currPwd);
		if (getcwd(cwd, sizeof(cwd)) != NULL)
			strcpy(SmallShell::getInstance().prevPwd, cwd);
		if (chdir(newPwd) == -1) {
			perror("smash error: chdir failed");
			return;
		} else {
			if (strcmp(SmallShell::getInstance().prevPwd, "") == 0) {
				if (getcwd(cwd, sizeof(cwd)) != NULL) {
					//strcpy(SmallShell::getInstance().prevPwd, cwd);
					strcpy(SmallShell::getInstance().currPwd, cwd);
				}
			} else {
				if (getcwd(cwd, sizeof(cwd)) != NULL) {
					strcpy(SmallShell::getInstance().currPwd, cwd);
				}
			}
		}
	}
	return;
}

void HistoryCommand::execute() {
	list<int>::iterator it_int;
	list<string>::iterator it_string = SmallShell::getInstance().comms.begin();

	for (it_int = SmallShell::getInstance().seq_num.begin();
			it_int != SmallShell::getInstance().seq_num.end(); it_int++) {
		cout << right << setw(5) << *it_int << "  " << *it_string << endl;
		it_string++;
	}
	return;
}

void ShowPidCommand::execute() {
	cout << "smash pid is " << getpid() << endl;
}

void JobsCommand::execute() {
	SmallShell::getInstance().jobs_list->printJobsList();
}

void BackgroundCommand::execute() {
	if (SmallShell::getInstance().getCmdVec().size() > 2) {
		cout << "smash error: bg: invalid arguments" << endl;
		return;
	}
	if (SmallShell::getInstance().getCmdVec().size() == 1) {
		if (SmallShell::getInstance().jobs_list->jobs.empty()) {
			cout << "smash error: bg: there is no stopped jobs to resume" << endl;
			return;
		} else {
			list<JobsList::JobEntry>::iterator it = SmallShell::getInstance().jobs_list->jobs.end();
			it--;
			for (int i = SmallShell::getInstance().jobs_list->jobs.size() - 1; i >= 0; i--) {
				if (it->stopped == 1) {
					cout << it->job_line << " : " << it->curr_pid << endl;
					if (kill(it->curr_pid, SIGCONT) == 0) {
						//SmallShell::getInstance().jobs_list->jobs.erase(it);
						int status;
						if(waitpid(it->curr_pid, &status, WNOHANG|WUNTRACED) > 0)
							SmallShell::getInstance().jobs_list->jobs.erase(it);
						it->stopped = 0;
						return;
					}
					else {
						perror("smash error: kill failed");
						return;
					}
				}
				it--;
			}
			cout << "smash error: bg: there is no stopped jobs to resume" << endl;
			return;
		}
	} else {
	string arg1 = SmallShell::getInstance().getCmdVec()[1];
	for (unsigned int i = 0; i < arg1.size(); i++) {
		if (!(arg1[i] >= '0' && arg1[i] <= '9')) {
			cout << "smash error: bg: invalid arguments" << endl;
			return;
		}
	}
	int job_id = atoi(arg1.c_str());
	list<JobsList::JobEntry>::iterator it;
	for (it = SmallShell::getInstance().jobs_list->jobs.begin(); it != SmallShell::getInstance().jobs_list->jobs.end(); it++) {
		if (it->job_id == job_id) {
			if (it->stopped == 1) {
			cout << it->job_line << " : " << it->curr_pid << endl;
			if (kill(it->curr_pid, SIGCONT) == 0) {
				//SmallShell::getInstance().jobs_list->jobs.erase(it);
				int status;
				if(waitpid(it->curr_pid, &status, WNOHANG|WUNTRACED) > 0)
					SmallShell::getInstance().jobs_list->jobs.erase(it);
				it->stopped = 0;
				return;
			}
			else {
				perror("smash error: kill failed");
				return;
			}
			} else {
				cout << "smash error: bg: job-id " << job_id << " is already running in the background" << endl;
				return;
			}
		}
	}
	cout << "smash error: bg: job-id " << job_id << " does not exist" << endl;
	return;
	}
}

void ForegroundCommand::execute() {
	if (SmallShell::getInstance().getCmdVec().size() > 2) {
		cout << "smash error: fg: invalid arguments" << endl;
		return;
	}
	if (SmallShell::getInstance().getCmdVec().size() == 1) {
		if (SmallShell::getInstance().jobs_list->jobs.empty()) {
			cout << "smash error: fg: jobs list is empty" << endl;
			return;
		} else {
			list<JobsList::JobEntry>::iterator it = SmallShell::getInstance().jobs_list->jobs.end();
			it--;
			if (kill(it->curr_pid, SIGCONT) == 0) {
				int status;
				cout << it->job_line << " : " << it->curr_pid << endl;
				//SmallShell::getInstance().jobs_list->jobs.erase(it);
				SmallShell::getInstance().fg_pid = it->curr_pid;
				if (waitpid(it->curr_pid, &status, WUNTRACED) > 0) {
					if (!WIFSTOPPED(status))
						SmallShell::getInstance().jobs_list->jobs.erase(it);
				}
				it->finished = 1; //hon
				return;
			}
			else {
				perror("smash error: kill failed");
				return;
			}
		}
	} else {
	string arg1 = SmallShell::getInstance().getCmdVec()[1];
	for (unsigned int i = 0; i < arg1.size(); i++) {
		if (!(arg1[i] >= '0' && arg1[i] <= '9')) {
			cout << "smash error: fg: invalid arguments" << endl;
			return;
		}
	}
	int job_id = atoi(arg1.c_str());
	list<JobsList::JobEntry>::iterator it;
	for (it = SmallShell::getInstance().jobs_list->jobs.begin(); it != SmallShell::getInstance().jobs_list->jobs.end(); it++) {
		if (it->job_id == job_id) {
			if (kill(it->curr_pid, SIGCONT) == 0) {
				int status;
				cout << it->job_line << " : " << it->curr_pid << endl;
				//SmallShell::getInstance().jobs_list->jobs.erase(it);
				SmallShell::getInstance().fg_pid = it->curr_pid;
				if (waitpid(it->curr_pid, &status, WUNTRACED) > 0) {
					if (!WIFSTOPPED(status))
						SmallShell::getInstance().jobs_list->jobs.erase(it);
				}
				it->finished = 1; //hon
				return;
			}
			else {
				perror("smash error: kill failed");
				return;
			}
		}
	}
	cout << "smash error: fg: job-id " << job_id << " does not exist" << endl;
	return;
	}
}

void KillCommand::execute() {
	if (SmallShell::getInstance().getCmdVec().size() != 3) {
		cout << "smash error: kill: invalid arguments" << endl;
		return;
	}
	string arg1 = SmallShell::getInstance().getCmdVec()[1];
	string arg2 = SmallShell::getInstance().getCmdVec()[2];
	string sig = "";
	for (unsigned int i = 1; i < arg1.size(); i++)
		sig += arg1[i];

	if (arg1[0] != '-') {
		cout << "smash error: kill: invalid arguments" << endl;
		return;
	}

	for (unsigned int i = 0; i < sig.size(); i++) {
		if (!(sig[i] >= '0' && sig[i] <= '9')) {
			cout << "smash error: kill: invalid arguments" << endl;
			return;
		}
	}
	for (unsigned int i = 0; i < arg2.size(); i++) {
		if (!(arg2[i] >= '0' && arg2[i] <= '9')) {
			cout << "smash error: kill: invalid arguments" << endl;
			return;
		}
	}
	int sig_id = atoi(sig.c_str());
	int job_id = atoi(arg2.c_str());

	list<JobsList::JobEntry>::iterator it;
	for (it = jobs_list->jobs.begin(); it != jobs_list->jobs.end(); it++) {
		if (it->job_id == job_id) {
			if (kill(it->curr_pid, sig_id) == 0) {
				cout << "signal number " << sig_id << " was sent to pid " << it->curr_pid << endl;
				//jobs_list->removeFinishedJobs();
				if (sig_id == SIGSTOP) {
					//waitpid(it->curr_pid, );
					it->stopped = 1;
				}
				else if (sig_id == SIGCONT) {
					it->stopped = 0;
				}
				return;
			}
			else {
				perror("smash error: kill failed");
				return;
			}
		}
	}
	cout << "smash error: kill: job-id " << job_id << " does not exist" << endl;
}

void CopyCommand::execute() {
	int file1_fd = open(SmallShell::getInstance().getCmdVec()[1].c_str(), O_RDONLY, 0666);
	if (file1_fd == -1) {
		perror("smash error: open failed");
		return;
	} else {
		int file2_fd = open(SmallShell::getInstance().getCmdVec()[2].c_str(), O_WRONLY|O_CREAT|O_TRUNC, 0666);
		if (file2_fd == -1) {
			perror("smash error: open failed");
			return;
		} else {
			char buf[2];
			ssize_t count = read(file1_fd, &buf, 1);
			if (count == -1) {
				perror("smash error: read failed");
				return;
			} else {
				if (write(file2_fd, buf, count) == -1) {
					perror("smash error: write failed");
					return;
				}
			}
			while (count > 0) {
				count = read(file1_fd, &buf, 1);
				if (count == -1) {
					perror("smash error: read failed");
					return;
				} else {
					if (write(file2_fd, buf, count) == -1) {
						perror("smash error: write failed");
						return;
					}
				}
			}
		}
		if (close(file2_fd) == -1) {
			perror("smash error: close failed");
			return;
		}
	}
	if (close(file1_fd) == -1) {
		perror("smash error: close failed");
		return;
	}
	cout << SmallShell::getInstance().getCmdVec()[1] << " was copied to " << SmallShell::getInstance().getCmdVec()[2] << endl; 
}

void QuitCommand::execute() {
	//list<JobsList::JobEntry>::iterator it;
	if (SmallShell::getInstance().cmdv.size() == 1)
		exit(0);
	if (SmallShell::getInstance().cmdv[1] != "kill")
		exit(0);
	SmallShell::getInstance().jobs_list->killAllJobs();
	exit(0);
}

SmallShell::SmallShell() : time_stamp(1) {
// TODO: add your implementation
	jobs_list = new JobsList();
	fg_pid = getpid();
}

SmallShell::~SmallShell() {
// TODO: add your implementation

}

vector<string>& SmallShell::getCmdVec() {
	return cmdv;
}

/**
* Creates and returns a pointer to Command class which matches the given command line (cmd_line)
*/
Command * SmallShell::CreateCommand(const char* cmd_line) {
	if (!getCmdVec().empty())
		getCmdVec().clear();
	string com = "";
	int i = 0, flag;
	if (cmd_line[0] == ' ' || cmd_line[0] == '\t' || cmd_line[0] == '\n' || cmd_line[0] == '\r')
		flag = 1;
	else
		flag = 0;
	while (cmd_line[i] != '\0') {
		if (flag == 1) {
			while (cmd_line[i] == ' ' || cmd_line[i] == '\t' || cmd_line[i] == '\n' || cmd_line[i] == '\r')
				i++;
		}
		if (cmd_line[i] == '\0')
			break;
		flag = 0;
		string c(1, cmd_line[i]);
		com += c;
		i++;
		if (cmd_line[i] == ' ' || cmd_line[i] == '\t' || cmd_line[i] == '\n' || cmd_line[i] == '\r' || cmd_line[i] == '\0') {
			flag = 1;
			cmdv.push_back(com);
			com = "";
		}
	}
	string com_line(cmd_line);
	curr_line = com_line;
	if (!comms.empty()) {
		list<int>::iterator seq_it = seq_num.end();
		list<string>::iterator com_it = comms.end();
		seq_it--;
		com_it--;
		if (*com_it == com_line) {
			(*seq_it)++;
			time_stamp++;

		} else {
			if (comms.size() < HISTORY_MAX_RECORDS) {
				comms.push_back(com_line);
				seq_num.push_back(time_stamp++);
			} else {
				comms.pop_front();
				seq_num.pop_front();
				comms.push_back(com_line);
				seq_num.push_back(time_stamp++);
			}
		}
	} else {
		comms.push_back(com_line);
		seq_num.push_back(time_stamp++);
	}
	if (!getCmdVec().empty()) {
	if (getCmdVec()[0] == "pwd") {
		return new GetCurrDirCommand(cmd_line);
	}
	if (getCmdVec()[0] == "cd") {
		char* lastPwd;
		return new ChangeDirCommand(cmd_line, &lastPwd);
	}
	if (getCmdVec()[0] == "history") {
		CommandsHistory hist;
		return new HistoryCommand(cmd_line, &hist);
	}
	if (getCmdVec()[0] == "showpid") {
		return new ShowPidCommand(cmd_line);
	}
	if (getCmdVec()[0] == "kill") {
		return new KillCommand(cmd_line, jobs_list);
	}
	if (getCmdVec()[0] == "jobs") {
		return new JobsCommand(cmd_line, jobs_list);
	}
	if (getCmdVec()[0] == "bg") {
		return new BackgroundCommand(cmd_line, jobs_list);
	}
	if (getCmdVec()[0] == "fg") {
		return new ForegroundCommand(cmd_line, jobs_list);
	}
	if (getCmdVec()[0] == "quit") {
		return new QuitCommand(cmd_line, jobs_list);
	}
	if (getCmdVec()[0] == "cp") {
		return new CopyCommand(cmd_line);
	} else {
		return new ExternalCommand(cmd_line);
	}
	}
	return nullptr;
}

void SmallShell::executeCommand(const char *cmd_line) {

	Command* cmd = CreateCommand(cmd_line);
	jobs_list->removeFinishedJobs();
	if (!cmdv.empty())
		cmd->execute();
	fg_pid = getpid();
	//cout << jobs_list->jobs.size() << endl;
}
