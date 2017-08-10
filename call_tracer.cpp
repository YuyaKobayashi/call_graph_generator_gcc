#include <dlfcn.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>

#include <string>
#include <iostream>
#include <fstream>
#include <unordered_map>


using namespace std;

static pid_t gettid(void)
{
	return syscall(SYS_gettid);
}

static unordered_map<pid_t, ofstream> files;
static ofstream& get_file(pid_t tid)
{
	ofstream &f = files[tid];

	if(! f.is_open()){
		string name;
		pid_t pid = getpid();
		name = string("func_hist.") + to_string(pid) + string("_") + to_string(tid);
		f.open(name.c_str());
	}

	return f;
}

typedef unsigned t_fdepth;
static unordered_map<pid_t, t_fdepth> func_depthes;

extern "C"
void __cyg_profile_func_enter(void* func_addr, void* call_site)
{
	pid_t tid = gettid();
	t_fdepth &d = func_depthes[tid];
	ofstream &f = get_file(tid);
	Dl_info dli;
	if( 0 != dladdr(func_addr, &dli)){
		//construct the function name
		string funcname;
		if( dli.dli_sname == 0 ) funcname = "NULL";
		else funcname = dli.dli_sname;

		for( t_fdepth i=0 ; i<d; i++)
			f << string("\t");
	   	f << "enter:" + funcname + "@" + dli.dli_fname;
	}

	f << endl;
	d++;
}

extern "C"
void __cyg_profile_func_exit(void* func_addr, void* call_site)
{
	pid_t tid = gettid();
	t_fdepth &d = func_depthes[tid];
	ofstream &f = get_file(tid);
	Dl_info dli;
	d--;
	if( 0 != dladdr(func_addr, &dli)){
		//construct the function name
		string funcname;
		if( dli.dli_sname == 0 ) funcname = "NULL";
		else funcname = dli.dli_sname;
		

		for( t_fdepth i=0 ; i<d; i++)
			f << string("\t");
	   	f << "exit:" + funcname + "@" + dli.dli_fname;

	}

	f << endl;
}
