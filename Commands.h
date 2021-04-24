#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <vector>
#include <string>
#include <list>
#include <linux/limits.h>

using namespace std;

#define COMMAND_ARGS_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)
#define HISTORY_MAX_RECORDS (50)

class Command {
// TODO: Add your data members
 public:
  Command(const char* cmd_line);
  virtual ~Command();
  virtual void execute() = 0;
  //virtual void prepare();
  //virtual void cleanup();
  // TODO: Add your extra methods if needed
};

class BuiltInCommand : public Command {
 public:
  BuiltInCommand(const char* cmd_line);
  virtual ~BuiltInCommand() {}
};

class ExternalCommand : public Command {
 public:
  ExternalCommand(const char* cmd_line);
  virtual ~ExternalCommand() {}
  void execute() override;
};

class PipeCommand : public Command {
  // TODO: Add your data members
 public:
  PipeCommand(const char* cmd_line);
  virtual ~PipeCommand() {}
  void execute() override;
};

class RedirectionCommand : public Command {
 // TODO: Add your data members
 public:
  explicit RedirectionCommand(const char* cmd_line);
  virtual ~RedirectionCommand() {}
  void execute() override;
  //void prepare() override;
  //void cleanup() override;
};

class ChangeDirCommand : public BuiltInCommand {
// TODO: Add your data members public:
public:
  ChangeDirCommand(const char* cmd_line, char** plastPwd);
  virtual ~ChangeDirCommand() {}
  void execute() override;
};

class GetCurrDirCommand : public BuiltInCommand {
 public:
  GetCurrDirCommand(const char* cmd_line);
  virtual ~GetCurrDirCommand() {}
  void execute() override;
};

class ShowPidCommand : public BuiltInCommand {
 public:
  ShowPidCommand(const char* cmd_line);
  virtual ~ShowPidCommand() {}
  void execute() override;
};

class JobsList;
class QuitCommand : public BuiltInCommand {
// TODO: Add your data members public:
public:
  QuitCommand(const char* cmd_line, JobsList* jobs);
  virtual ~QuitCommand() {}
  void execute() override;
};

class CommandsHistory {
 protected:
  class CommandHistoryEntry {
	  // TODO: Add your data members
  };
 // TODO: Add your data members
 public:
  CommandsHistory() {}
  ~CommandsHistory() {}
  void addRecord(const char* cmd_line);
  void printHistory();
};

class HistoryCommand : public BuiltInCommand {
 // TODO: Add your data members
 public:
  HistoryCommand(const char* cmd_line, CommandsHistory* history);
  virtual ~HistoryCommand() {}
  void execute() override;
};

class JobsList {
 public:
  class JobEntry {
  public:
		int job_id;
		string job_line;
		pid_t curr_pid;
		time_t time_begin;
		int stopped;
		int finished;

		JobEntry(int job_id, string job_line, pid_t curr_pid, time_t time_begin, int stopped, int finished)
		: job_id(job_id), job_line(job_line), curr_pid(curr_pid), time_begin(time_begin), stopped(stopped), finished(finished) {

		}

		~JobEntry() { }
  };
 // TODO: Add your data members
  list<JobEntry> jobs;
 public:
  JobsList();
  ~JobsList();
  void addJob(pid_t pid, bool isStopped = false);
  list<JobEntry>& getJobs() {
	  return jobs;
  }
  void printJobsList() {
	  removeFinishedJobs();
	  list<JobEntry>::iterator it;
	  for (it = jobs.begin(); it != jobs.end(); it++) {
		  time_t now = time(NULL);
		 // if (it->finished == 0){ //hon
		  if (it->stopped == 1)
			  cout << "[" << it->job_id << "] " << it->job_line << " : " << it->curr_pid << " " << (now - it->time_begin) << " secs (stopped)" << endl;
		  else
			  cout << "[" << it->job_id << "] " << it->job_line << " : " << it->curr_pid << " " << (now - it->time_begin) << " secs" << endl;
		//  }
	  }
  }
  void killAllJobs() {
	  removeFinishedJobs();
	  list<JobEntry>::iterator it;
	  cout << "smash: sending SIGKILL signal to " << jobs.size() << " jobs:" << endl;
	  for (it = jobs.begin(); it != jobs.end(); it++) {
		  pid_t p = it->curr_pid;
		  string s = it->job_line;
		  if (kill(it->curr_pid, SIGKILL) == 0) {
			  cout << p << ": " << s << endl;
		  } else {
			  perror("smash error: kill failed");
		  }
	  }
  }
  void removeFinishedJobs();
  JobEntry * getJobById(int jobId) {
	  list<JobEntry>::iterator it;
	 	  	  for (it = jobs.begin(); it != jobs.end(); it++) {
	 	  		  if (it->job_id == jobId) {
	 	  			 return &(*it);
	 	  		  }
	 	  	  }
	 	  	  return NULL;
  }
  void removeJobById(int jobId) {
	 // removeFinishedJobs();
	  list<JobEntry>::iterator it;
	  	  for (it = jobs.begin(); it != jobs.end(); it++) {
	  		  if (it->job_id == jobId) {
	  			 it = jobs.erase(it);
	  			 break;
	  		  }
	  	  }
  }
  JobEntry * getLastJob(int* lastJobId) {
	  if (!jobs.empty()) {
		  list<JobEntry>::iterator it = jobs.end();
		  it--;
		  *lastJobId = it->job_id;
		  return &(*it);
	  }
	  return NULL;
  }
  JobEntry *getLastStoppedJob(int *jobId) {
	  list<JobEntry>::iterator it = jobs.end();
	  it--;
	  	  for (int i = jobs.size(); i > 0; i--) {
	  		  if (it->stopped == 1) {
	  			  *jobId = it->job_id;
	  			  return &(*it);
	  		  }
	  		  it--;
	  	  }
	  	  return NULL;
  }
  // TODO: Add extra methods or modify exisitng ones as needed
};

class JobsCommand : public BuiltInCommand {
 // TODO: Add your data members
 public:
  JobsCommand(const char* cmd_line, JobsList* jobs);
  virtual ~JobsCommand() {}
  void execute() override;
};

class KillCommand : public BuiltInCommand {
 // TODO: Add your data members
 public:
	JobsList* jobs_list;
  KillCommand(const char* cmd_line, JobsList* jobs);
  virtual ~KillCommand() {}
  void execute() override;
};

class ForegroundCommand : public BuiltInCommand {
 // TODO: Add your data members
 public:
  ForegroundCommand(const char* cmd_line, JobsList* jobs);
  virtual ~ForegroundCommand() {}
  void execute() override;
};

class BackgroundCommand : public BuiltInCommand {
 // TODO: Add your data members
 public:
  BackgroundCommand(const char* cmd_line, JobsList* jobs);
  virtual ~BackgroundCommand() {}
  void execute() override;
};

class CopyCommand : public BuiltInCommand {
 public:
  CopyCommand(const char* cmd_line);
  virtual ~CopyCommand() {}
  void execute() override;
};


class SmallShell {
 private:
  // TODO: Add your data members
	// Command* cmd;
  SmallShell();
 public:
  JobsList* jobs_list;
  vector<string> cmdv;
  list<int> seq_num;
  list<string> comms;
  int time_stamp;
  char currPwd[PATH_MAX];
  char prevPwd[PATH_MAX];
  string curr_line;
  pid_t fg_pid;
  Command *CreateCommand(const char* cmd_line);
  SmallShell(SmallShell const&)      = delete; // disable copy ctor
  void operator=(SmallShell const&)  = delete; // disable = operator
  static SmallShell& getInstance() // make SmallShell singleton
  {
    static SmallShell instance; // Guaranteed to be destroyed.
    // Instantiated on first use.
    return instance;
  }
  ~SmallShell();
  void executeCommand(const char* cmd_line);
  vector<string>& getCmdVec();
 // void setPwd(string pwd);
  // TODO: add extra methods as needed
};

#endif //SMASH_COMMAND_H_
