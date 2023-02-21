#include <cstdlib>
#include <unistd.h>
#include <fcntl.h>
#include <android/log.h>
#include <cstring>
#include <string>
#include <iostream>
#include <dirent.h>
#include <fstream>
#include <signal.h>

#include "zygisk.hpp"

using zygisk::Api;
using zygisk::AppSpecializeArgs;
using zygisk::ServerSpecializeArgs;

#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, "Magisk", __VA_ARGS__)

class MyModule : public zygisk::ModuleBase
{
public:
    void onLoad(Api *api, JNIEnv *env) override
    {
        this->api = api;
        this->env = env;
    }

    void preAppSpecialize(AppSpecializeArgs *args) override
    {
        // Use JNI to fetch our process name
        const char *process = env->GetStringUTFChars(args->nice_name, nullptr);
        const int uid = (int)(args->uid);
        char *buf = (char *)malloc(27);
        strcpy(buf, "com.example.forloop_native");
        const char *app_name = env->GetStringUTFChars(env->NewStringUTF(buf), nullptr);

        LOGD("[i] preAppSpecialize: %i ", uid);
        LOGD("[i] preAppSpecialize: %s ", process);

        // int getProcIdByName(char* procName)
        // {
        //     int pid = -1;

        //     LOGD("[i] preAppSpecialize: Getting proceID");

        //     // Open the /proc directory
        //     DIR *dp = opendir("/proc");
        //     if (dp != NULL)
        //     {
        //         // Enumerate all entries in directory until process found
        //         struct dirent *dirp;
        //         while (pid < 0 && (dirp = readdir(dp)))
        //         {
        //             // Skip non-numeric entries
        //             int id = atoi(dirp->d_name);
        //             if (id > 0)
        //             {
        //                 // Read contents of virtual /proc/{pid}/cmdline file
        //                 char* cmdPath = string("/proc/") + dirp->d_name + "/cmdline";
        //                 ifstream cmdFile(cmdPath.c_str());
        //                 char* cmdLine;
        //                 getline(cmdFile, cmdLine);
        //                 if (!cmdLine.empty())
        //                 {
        //                     // Keep first cmdline item which contains the program path
        //                     size_t pos = cmdLine.find('\0');
        //                     if (pos != string::npos)
        //                         cmdLine = cmdLine.substr(0, pos);
        //                     // Keep program name only, removing the path
        //                     pos = cmdLine.rfind('/');
        //                     if (pos != string::npos)
        //                         cmdLine = cmdLine.substr(pos + 1);
        //                     // Compare against requested process name
        //                     if (procName == cmdLine)
        //                         pid = id;
        //                 }
        //             }
        //         }
        //     }
        //     closedir(dp);

        //     LOGD("[i] preAppSpecialize: Getting proceID - %i",pid );
        //     return pid;
        // }

        //
        // TODO
        // - saprastk ka strada komunikacijas ar companion
        // - uzakstit ka komunice un pieprasa pec tiem pid
        //

        if (!strcmp(app_name, process))
        {

            unsigned pid = 0;
            int fd = api->connectCompanion();
            read(fd, &pid, sizeof(pid));
            close(fd);
            // LOGD("[i] preAppSpecialize: Getting system_server proceID - %i", pid);
            LOGD("[i] preAppSpecialize: You can attach with IDA now");
            raise(19); // SIGSTOP
        }

        // int count = 0;
        // int br = 0;
        // int br2 = 0;
        // if (!strcmp(app_name, process))
        // {
        //     while (br == 0)
        //     {
        //         if ((count % 100000) == 0)
        //         {
        //            LOGD("[i] preAppSpecialize: %i ", uid);
        //            LOGD("[i] preAppSpecialize: %s ", process);
        //            LOGD("[i] preAppSpecialize br2: %i ", br2);
        //            br2 = br2 + 1;
        //         };

        //        //count = count + 1;

        //        if (br2 > 10000)
        //        {
        //            break;
        //        }
        //     };
        // }

        env->ReleaseStringUTFChars(args->nice_name, process);
    }

private:
    Api *api;
    JNIEnv *env;

    // void preSpecialize(const char *process) {
    //     // Demonstrate connecting to to companion process
    //     // We ask the companion for a random number
    //     unsigned r = 0;
    //     int fd = api->connectCompanion();
    //     read(fd, &r, sizeof(r));
    //     close(fd);
    //     LOGD(" preSpecialize() -> example: process=[%s], r=[%u]\n", process, r);

    //     // Since we do not hook any functions, we should let Zygisk dlclose ourselves
    //     api->setOption(zygisk::Option::DLCLOSE_MODULE_LIBRARY);
    // }
};

// Companion handler example - returns random num from zygisk
// static int urandom = -1;
// static void companion_handler(int i) {
//     if (urandom < 0) {
//         urandom = open("/dev/urandom", O_RDONLY);
//     }
//     unsigned r;
//     read(urandom, &r, sizeof(r));
//     LOGD("example: companion r=[%u]\n", r);
//     write(i, &r, sizeof(r));
// }

static void companion_handler(int i)
{

    // Finds pid of system_server and stops it

    std::string procName = "system_server";
    pid_t pid = -1;

    // Open the /proc directory
    DIR *dp = opendir("/proc");
    if (dp != NULL)
    {
        // Enumerate all entries in directory until process found
        struct dirent *dirp;
        while (pid < 0 && (dirp = readdir(dp)))
        {
            // Skip non-numeric entries
            pid_t id = atoi(dirp->d_name);
            if (id > 0)
            {
                // Read contents of virtual /proc/{pid}/cmdline file
                std::string cmdPath = std::string("/proc/") + std::string(dirp->d_name) + std::string("/cmdline");

                std::ifstream cmdFile(cmdPath.c_str());
                std::string cmdLine;
                getline(cmdFile, cmdLine);
                if (!cmdLine.empty())
                {
                    if (procName.find(cmdLine.c_str()) != std::string::npos)
                    {
                        pid = id;
                    }
                }
            }
        }
    }
    closedir(dp);

    LOGD("[i] In Zygisk (Companion) proces: found system_server pid=[%u]\n", pid);
    LOGD("[i] In Zygisk (Companion) proces: trying to STOPsystem_server pid=[%u]\n", pid);
    int killStatus = kill(pid, 19); // SIGSTOP 19
    LOGD("[i] In Zygisk (Companion) proces: kill status %i\n", killStatus);
    write(i, &pid, sizeof(pid));
}

REGISTER_ZYGISK_MODULE(MyModule)
REGISTER_ZYGISK_COMPANION(companion_handler)
// REGISTER_ZYGISK_COMPANION(getProcIdByName) // registering to run in zygisk
