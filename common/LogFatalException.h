/* LogFatalException.h */
#ifndef __LOG_FATAL_EXCEPTION_H__
#define __LOG_FATAL_EXCEPTION_H__

#include <unistd.h>
#include <signal.h>
#include <exception>
#include <execinfo.h>
#include <cxxabi.h>
#include <dlfcn.h>
#include <iostream>
#include <sstream>
#include "LogUtils.h"

#define DUMP_EXCEPTION(exception_t, lines) do{\
    try{\
        lines\
    }catch(exception_t e){\
		RLOGE("catch %s execption: %s from the lines: ", #exception_t, e.what()); \
        RLOGE("call-stack dumped: \n%s", LogFatalException::BackTrace().c_str()); \
    }\
}while(0);\

class LogFatalException: public std::exception
{
private:
    std::string confess_;

public:
    LogFatalException(){
        confess_ = "@f@ LogFatal exception!";
    }

    LogFatalException(std::string& msg) : confess_(msg)
    {
        LOGPF("@f@ call-stack dumped:\n %s", BackTrace(8, 1).c_str());
    }

    ~LogFatalException(){}

    virtual const char* what() const noexcept {
        return confess_.c_str();
    }

    static std::string BackTrace(const int nMaxFrames = 32, const int skip = 1){
        void *callstack[nMaxFrames];
        char buf[1024*8];
        int nFrames = backtrace(callstack, nMaxFrames);
        char **symbols = backtrace_symbols(callstack, nFrames);

        std::ostringstream trace_buf;
        for (int i = skip; i < nFrames; i++) {
            Dl_info info;
            if (dladdr(callstack[i], &info)) {
                char *demangled = NULL;
                int status;
                demangled = abi::__cxa_demangle(info.dli_sname, NULL, 0, &status);
                snprintf(buf, sizeof(buf), "%-2d: %p\t%s\n",
                        i, callstack[i], status == 0 ? demangled : info.dli_sname);
                free(demangled);
            } else {
                snprintf(buf, sizeof(buf), "%-2d: %p\t%s\n", i, callstack[i], symbols[i]);
            }
            trace_buf << buf;
        }
        free(symbols);
        if (nFrames >= nMaxFrames)
            trace_buf << "[truncated]\n";
        return trace_buf.str();
	}
};

#endif