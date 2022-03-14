#import <Foundation/NSThread.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSString.h>
#include <string>
#include <dispatch/dispatch.h>

namespace sysutils
{
    std::string getStackTrace()
    {
        return std::string([[[NSThread callStackSymbols] description] UTF8String]);
    }
    bool isMainThread()
    {
        return [[NSThread currentThread] isMainThread];
    }
    void performOnMainThreadAndWait(std::function<void()> f)
    {
        dispatch_sync(dispatch_get_main_queue(),
        ^{
            f();
        });
    }
    void performOnMainThreadAsync(std::function<void()> f)
    {
        dispatch_async(dispatch_get_main_queue(),
        ^{
            f();
        });
    }
    
    void setThreadName(const char * utf8name)
    {
        [[NSThread currentThread] setName:[NSString stringWithUTF8String:utf8name]];
    }
}
