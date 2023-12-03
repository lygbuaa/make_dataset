/* SignalBase.h */
#ifndef __SIGNAL_BASE_H__
#define __SIGNAL_BASE_H__

#include <memory>
#include <unistd.h>
#include <signal.h>
#include "LogFatalException.h"

/*T is the derived-class name*/
class SignalBase
{
public:
    SignalBase(){};
    ~SignalBase(){};
    static void QuitHandler(int sig)
    {
        LOGPF("@q@ pid %d catch quit signal %d", getpid(), sig);
        LOGPF("@q@ call-stack dumped: %s", LogFatalException::BackTrace(16, 2).c_str());
        //use _exit(), exit() may cause re-enter problem
        _exit(sig);
    }
    static void SigSegvHandler(int sig, siginfo_t *si, void *arg)
    {
        LOGPF("@q@ pid %d catch segment fault @%p", getpid(), si->si_addr);
        LOGPF("@q@ call-stack dumped: %s", LogFatalException::BackTrace(16, 2).c_str());
        //use _exit(), exit() may cause re-enter problem
        _exit(sig);
    }
    static void CatchSignal()
    {
        signal(SIGINT, QuitHandler); //2
        signal(SIGABRT, QuitHandler);// 6
        // signal(SIGSEGV, QuitHandler);// 15
        signal(SIGTERM, QuitHandler);// 15

        /* catch segment fault */
        struct sigaction sa;
        sigemptyset(&sa.sa_mask);
        sa.sa_sigaction = SigSegvHandler;
        sa.sa_flags   = SA_SIGINFO;
        sigaction(SIGSEGV, &sa, NULL);
    }
};

#endif //__SIGNAL_BASE_H__