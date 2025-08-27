#include <stdio.h>
#include <unistd.h>

#include "cgroup.h"
#include "logio.h"

void printFirstLine(const char *path, const char *option) {
    FILE *file = fopen(path, "r");
    if (!file) {
        crinitErrnoPrint("Could not open file %s.", path);
        return;
    }

    char buffer[4096];
    if (fgets(buffer, sizeof(buffer), file)) {
        crinitInfoPrint("%s: %s.", option, buffer);
    }
    fclose(file);
}

int main(int argc, char **argv) {
    (void)(argc);
    (void)(argv);

    crinitCgroup_t cgroupParent = {0};
    cgroupParent.name = "myGlobalCgroup";
    crinitCgroupParam_t *parentParam[] = {NULL};
    crinitCgroupConfiguration_t parentConfig = {.param = parentParam, .paramCount = 0};
    cgroupParent.config = &parentConfig;

    crinitCgroup_t cgroup = {0};
    cgroup.name = "myCgroup";
    cgroup.parent = &cgroupParent;
    crinitCgroupParam_t param1 = {"cgroup.freeze", "0"};
    crinitCgroupParam_t *param[] = {&param1};
    crinitCgroupConfiguration_t config = {.param = param, .paramCount = sizeof(param) / sizeof(param[0])};
    cgroup.config = &config;

    pid_t pid = getpid();

    crinitInfoPrint("pid is %d", pid);
    if (crinitCGroupConfigure(&cgroupParent) == 0) {
        if (crinitCGroupConfigure(&cgroup) == 0) {
            crinitCGroupAssignPID(&cgroup, pid);
        }
    }

    char path[4096] = {0};
    snprintf(path, sizeof(path), "%s/%s/%s/%s", (char *)CRINIT_CGROUP_PATH, cgroupParent.name, cgroup.name,
             "cgroup.procs");
    printFirstLine(path, "cgroup.procs");

    snprintf(path, sizeof(path), "%s/%s/%s/%s", (char *)CRINIT_CGROUP_PATH, cgroupParent.name, cgroup.name,
             param1.filename);
    printFirstLine(path, param1.filename);

    return 0;
}
