# EB BaseOS Crinit -- Configurable Rootfs Init

## In a Nutshell
Crinit, the Configurable Rootfs Init, is an init daemon executed after the rootfs has been mounted and switched into by
[cominit](https://gitlabintern.emlix.com/elektrobit/base-os/eb-baseos-cominit). It reads a global configuration
("series"-file) either specified on the command line or from `/etc/crinit/default.series`. The series file in turn
specifies further configuration files, each for a task potentially containing a set of commands and depenencies on other
tasks (see [Configuration](#configuration) below for examples).

The specified tasks are then started with as much parallelism as dependencies allow, i.e. tasks without any dependencies
are spawned asap after Crinit has been started. Once a task is spawned, finished, or has failed its dependent tasks are
updated and spawned as necessary.

## Concept
The below diagram shows the overall concept of the finished Crinit.

![Rootfs Init Daemon Components](images/rootfs_init_comp.svg)

Not indicated in the diagram are planned cryptographic features of the Config Parser intended to provide
verification/integrity checking of the global and task configurations. For this, each config file will need to include a
signature.

## Current Implementation

The central Task Data Structure (`ebcl_TaskDB` in `taskdb.h`), Config Parser (`confparse.h/.c`), and Process Dispatcher
(`procdip.h/.c`) have been preliminarily implemented and are functioning. In addition, the implementation contains a
simple encapsulated storage for global options (`globopt.h/.c`), some minimal PID 1 setup code which cannot be "sourced
out" into a task configuration (`minsetup.h/.c`), and debug/log output functionality (`logio.h/.c`). For detailed
explanations of the inner workings please refer to the Doxygen-generated documentation of the individual header and
source files.

Not currently implemented is the Notification/Service Interface and Library, the SafeSystemStartup-like
jailing functionality of the Process Dispatcher, as well as the cryptographic features of the Config Parser. 

## Configuration

As described above, Crinit needs a global series-file containing global configuration options as well as a list of task
configurations. Examples for a local demonstration inside the build environment (see [Build
Instructions](#build-instructions) below) are available in `config/test/` and examples for use on the S32G board are
available in `config/s32g/`.

### Example Global Configuration
An example, as used to boot a minimal environment on the S32G board, may look like this:
```
# BaseOS crinit series file, specifying which files to parse

TASKS = earlysetup.crinit check_qemu.crinit network-dhcp.crinit sshd.crinit getty.crinit

TASKDIR = /etc/crinit
DEBUG = NO
FILE_SIGS_NEEDED = NO

# not yet implemented
SIG = ""
```
#### Explanation
- **TASKS** -- The task configurations to load
- **TASKDIR** -- Where to find the task configurations, will be prepended to the filenames in **TASKS**.
- **DEBUG** -- If crinit should be verbose in its ouput. Either `YES` or `NO`.
- **FILE_SIGS_NEEDED** -- If each task configuration needs its own signature. As signature checking is not yet
  implemented, this is parsed but does nothing.
- **SIG** -- The signature of this file. Currently unimplemented and can be left empty.

### Example Task Configuration
The `network-dhcp.crinit` from above looks like this:
```
# DHCP config for S32G board

NAME = network-dhcp

COMMAND[0] = /bin/mkdir -p /var/lib/dhcpcd
COMMAND[1] = /bin/mount -t tmpfs none /var/lib/dhcpcd
COMMAND[2] = /bin/touch /var/lib/dhcpcd/resolv.conf
COMMAND[3] = /bin/mount -o bind /var/lib/dhcpcd/resolv.conf /etc/resolv.conf
COMMAND[4] = /sbin/ifconfig lo up
COMMAND[5] = /sbin/ifconfig lo 127.0.0.1
COMMAND[6] = /sbin/dhcpcd -j /var/log/dhcpcd.log eth0


# So we only run if we are on the actual S32G board
DEPENDS = check_qemu:fail earlysetup:wait

RESPAWN = NO
# features below not yet implemented
EXEC = NO
QM_JAIL = NO
SIG = ""
```
#### Explanation
- **NAME** -- The name given to this task configuration. Relevant if other tasks want to depend on this one.
- **COMMAND[n]** -- The commands to be executed in series. Executable paths must be absolute. Execution will stop if
  one of them fails and the whole task will be considered failed. The whole task is considered finished (i.e.
  `the network-dhcp:wait` dependency is fulfilled) if the last command has successfully returned.
- **DEPENDS** -- A list of dependencies which need to be fulfilled before this task is considered "ready-to-start".
  Semantics are `<taskname>:{fail,wait,spawn}`, where `spawn` is fulfilled when (the first command of) a task has been
  started, `wait` if it has successfully completed, and `fail` if it has failed somewhere along the way. Here we can see
  this task is only run if and after the `earlysetup` (setup of system directories, etc.) has fully completed and the
  `check_qemu` task has determined we are _not_ running inside the emulator and therefore exited with an error code.
  _Not yet implemented:_ Once the interface to the
  [Monitor](https://gitlabintern.emlix.com/elektrobit/base-os/corbos-tools) has been implemented, it will be possible to
  depend on a monitor event by adding `@ebclmon:<event_name>` to `DEPENDS`.
- **RESPAWN** -- If set to `YES`, the task will be restarted on failure or completion. Useful for daemons like `getty`.
- **EXEC** -- If set to `YES`, `crinit` will exec into the first `COMMAND` of this task instead of spawning a process.
  (Not yet implemented.)
- **QM_JAIL** -- If set to `YES`, all `COMMAND`s will be spawned inside a restricted environment such as the non-SIL/QM
  environment created by SafeSystemStartup. (Not yet implemented.)
- **SIG** -- The signature of this file. Currently unimplemented and can be left empty.

## Build Instructions
Executing
```
ci/docker-run.sh
```
will start a Docker container with all necessary programs to build Crinit and its Doxygen documentation.

Inside the container, it is sufficient to run
```
ci/build.sh
```
which will compile `crinit` for ARM64, as used for the S32G board, as well as an RPM for the native architecture the
build is run on. The script will copy relevant build artifacts to `result/`.

If a quick native test build is desired, it is fine to simply run 
```
make clean && make
```
inside the container. After the native build a local demonstration is possible by executing
```
./crinit /base/config/test/local.series
```
inside the container.

The Doxygen documentation alone can be built using
```
make doxygen
```
