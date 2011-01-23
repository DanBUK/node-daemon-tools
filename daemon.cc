/*
* daemon-tools.node
*** A node.JS addon that allows creating Unix/Linux Daemons in pure Javascript.
*** Copyright 2010 (c) <arthur@norgic.com>
* Under MIT License. See LICENSE file.
* Updated by Daniel Bartlett 2011 <dan@f-box.org>
*/

#include <v8.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ev.h>

#define PID_MAXLEN 10

using namespace v8;

// Go through special routines to become a daemon.
// if successful, returns daemon's PID
Handle<Value> Start(const Arguments& args) {
	pid_t pid, sid;
	int do_cd_root;
	if (args.Length() < 1) {
		return ThrowException(Exception::TypeError(
			String::New("Must have one argument true/false as to if we cd /")));
	}

	do_cd_root = args[0]->Int32Value();

	pid = fork();
	if(pid > 0) exit(0);
	if(pid < 0) exit(1);

	ev_default_fork();
	
	// Can be changed after with process.umaks
	umask(0);
	
	sid = setsid();
	if(sid < 0) exit(1);
	
	// Can be changed with process.chdir
	if (do_cd_root == 1) {
		chdir("/");
	}
	
	return Integer::New(getpid());
}

// Close Standard IN/OUT/ERR Streams
Handle<Value> CloseIO(const Arguments& args) {
	freopen("/dev/null", "r", stdin);
	freopen("/dev/null", "w", stdout);
	freopen("/dev/null", "w", stderr);
}

// File-lock to make sure that only one instance of daemon is running.. also for storing PID
/* lock ( filename )
*** filename: a path to a lock-file.
*** Note: if filename doesn't exist, it will be created when function is called.
*/
Handle<Value> LockD(const Arguments& args) {
	if(!args[0]->IsString())
		return Boolean::New(false);
	
	String::Utf8Value data(args[0]->ToString());
	char pid_str[PID_MAXLEN+1];
	
	int lfp = open(*data, O_RDWR | O_CREAT, 0640);
	if(lfp < 0) exit(1);
	if(lockf(lfp, F_TLOCK, 0) < 0) exit(0);
	
	int len = snprintf(pid_str, PID_MAXLEN, "%d", getpid());
	write(lfp, pid_str, len);
	
	return Boolean::New(true);
}


const char* ToCString(const v8::String::Utf8Value& value) {
	return *value ? *value : "<string conversion failed>";
}

/* chroot ( folder )
 * folder: The new root
*/
Handle<Value> Chroot(const Arguments& args) {
	if (args.Length() < 1) {
		return ThrowException(Exception::TypeError(
			String::New("Must have one argument; a string of the folder to chroot to.")
		));
	}
	uid_t uid;
	int rv;

	uid = getuid();
	if (uid != 0) {
		return ThrowException(Exception::Error(
			String::New("You must be root in order to use chroot.")
		));
	}

	String::Utf8Value folderUtf8(args[0]->ToString());
	const char *folder = ToCString(folderUtf8);
/*
	rv = chdir(folder);
	if (rv != 0) {
		return ThrowException(Exception::Error(
			String::New("Failed to cd to the folder.")
		));
	}
*/
	rv = chroot(folder);
	if (rv != 0) {
		return ThrowException(Exception::Error(
			String::New("Failed do chroot to the folder.")
		));
	}
	chdir("/");

	return Boolean::New(true);
}

extern "C" void init(Handle<Object> target) {
	HandleScope scope;
	
	target->Set(String::New("start"), FunctionTemplate::New(Start)->GetFunction());
	target->Set(String::New("lock"), FunctionTemplate::New(LockD)->GetFunction());
	target->Set(String::New("closeIO"), FunctionTemplate::New(CloseIO)->GetFunction());
	target->Set(String::New("chroot"), FunctionTemplate::New(Chroot)->GetFunction());
}
