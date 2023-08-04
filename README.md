# Crinit -- Configurable Rootfs Init

## In a Nutshell

Crinit is an init daemon executed as PID 1 by the Linux Kernel. It reads a global configuration ("series"-file) either
specified on the command line or from `/etc/crinit/default.series`. The series file in turn may reference further
configuration files or a whole directory from which all configs shall be loaded. Each one defines a task potentially
containing a set of commands and dependencies on other tasks.

The specified tasks are then started with as much parallelism as dependencies allow, i.e. tasks without any dependencies
are spawned ASAP after Crinit has been started. Once a task is spawned, finished, or has failed its dependent tasks are
updated and spawned as necessary.

## Concept

The below diagram shows the overall concept of the finished Crinit.

![Rootfs Init Daemon Components](images/rootfs_init_comp.svg)

Not indicated in the diagram are planned cryptographic features of the Config Parser intended to provide
verification/integrity checking of the global and task configurations. For this, each config file will need to include a
signature.

## Current Implementation

The central Task Data Structure (`crinitTaskDB` in `taskdb.h`), Config Parser (`confparse.h/.c`), Process Dispatcher
(`procdip.h/.c`), Notification/Service interface (`notiserv.h/.c`, `rtimcmd.h/.c`), Client library
(`crinit-client.h/.c`) have been preliminarily implemented and are functioning. In addition, the implementation contains
a simple encapsulated storage for global options (`globopt.h/.c`), some minimal PID 1 setup code which cannot be
"sourced out" into a task configuration (`minsetup.h/.c`), debug/log output functionality (`logio.h/.c`), a CLI
control program showcasing the the client API (`crinit-ctl.c`) including `reboot` and `poweroff` functionality.
For detailed explanations of the inner workings please refer to the Doxygen-generated documentation of the individual
header and source files.

The client API is documented in the Doxygen documentation of `crinit-client.h`. The API is implemented as a shared
library (`libcrinit-client.so`).

Not currently implemented is the SafeSystemStartup-like jailing functionality of the Process Dispatcher, as well as the
cryptographic features of the Config Parser. Also, the Notification and Service interface does not yet support handling
of elos events.

This repository also includes an example application to generate a `/etc/machine-id` file, which is used to uniquely
identify the system (for example by `elosd`). The `machine-id-gen` tool is supposed to be called on boot, for example
from `earlysetup.crinit` as in the example configuration files contained in this repository. Its implementation either
uses the value for `systemd.machine_id` given on the Kernel command line or -- on an NXP S32G-based board -- the unique
ID burned to on-chip OTP memory. If the Kernel command line value is set, it always takes precedence and any physical
memory OTP reads are omitted. This means that while the application has special functionality for S32G SoCs, it can work
on any target as long as the Kernel command line contains the necessary value.

## Configuration

As described above, Crinit needs a global series-file containing global configuration options as well as a list of task
configurations. Examples for a local demonstration inside the build environment (see [Build
Instructions](#build-instructions) below) are available in `config/test/` and examples to use as a starting point for a
minimal system boot are available in `config/example/`.

The general format of crinit configuration files is INI-style `KEY = value` pairs. Some settings may be array-like,
meaning they can be specified multiple times to append values. Leaving out the `KEY =` part at the start of the line in
favor of at least one whitespace character is shorthand for appending to the last key, e.g.
```
ARRAY_LIKE_KEY = value 1
ARRAY_LIKE_KEY = value 2
```
is equivalent to
```
ARRAY_LIKE_KEY = value 1
                 value 2
```

### Example Global Configuration

An example to boot a minimal environment may look like this:
```
# Crinit global configuration file

TASKS = earlysetup.crinit check_qemu.crinit network-dhcp.crinit
        sshd.crinit getty.crinit

TASKDIR = /etc/crinit
TASK_FILE_SUFFIX = .crinit
TASKDIR_FOLLOW_SYMLINKS = YES
INCLUDEDIR = /etc/crinit
INCLUDE_SUFFIX = .crincl
DEBUG = NO

SHUTDOWN_GRACE_PERIOD_US = 100000

USE_SYSLOG = NO

ENV_SET = FOO "foo"
ENV_SET = FOO_BAZ "${FOO} baz"
ENV_SET = GREETING "Good morning!"
```
#### Explanation
- **TASKS** -- The task configurations to load. This is an optional setting. If unset, **TASKDIR** will be scanned for
               task configuration files. (*array-like*)
- **TASKDIR** -- Where to find the task configurations, will be prepended to the filenames in **TASKS**.
                 Default: `/etc/crinit`
- **TASK_FILE_SUFFIX** -- Filename suffix of task configurations. Only relevant if **TASKS** is not set.
                          Default: `.crinit`
- **TASKDIR_FOLLOW_SYMLINKS** -- If symbolic links should be followed during scanning of **TASKDIR**. Only relevant if
                                 **TASKS** is not set. Default: YES
- **INCLUDEDIR** -- Where to find include files referenced from task configurations. Default: Same as **TASKDIR**.
- **INCLUDE_SUFFIX** -- Filename suffix of include files referenced from task configurations. Default: `.crincl`
- **DEBUG** -- If crinit should be verbose in its output. Either `YES` or `NO`. Default: `NO`
- **SHUTDOWN_GRACE_PERIOD_US** -- The amount of microseconds to wait between `SIGTERM` and `SIGKILL` on shutdown/reboot.
                                  Default: 100000
- **USE_SYSLOG** -- If syslog should be used for output if it is available. If set to `YES`, Crinit will switch to
                    syslog for output as soon as a task file `PROVIDES` the `syslog` feature. Ideally this should be
                    a task file loading a syslog server such as syslogd or elosd. Default: `NO`
- **ENV_SET** -- See section **Setting Environment Variables** below. (*array-like*)

### Example Task Configuration
The `network-dhcp.crinit` from above could for example look like this:
```
# DHCP config for a minimal system

NAME = network-dhcp

INCLUDE = daemon_env_preset

COMMAND = /bin/mkdir -p /var/lib/dhcpcd
          /bin/mount -t tmpfs none /var/lib/dhcpcd
          /bin/touch /var/lib/dhcpcd/resolv.conf
          /bin/mount -o bind /var/lib/dhcpcd/resolv.conf /etc/resolv.conf
          /sbin/ifconfig lo up
          /sbin/ifconfig lo 127.0.0.1
          /sbin/dhcpcd -j /var/log/dhcpcd.log eth0

DEPENDS = check_qemu:fail earlysetup:wait @provided:writable_var

PROVIDES = ipv4_dhcp:wait resolvconf:wait

RESPAWN = NO
RESPAWN_RETRIES = -1

ENV_SET = FOO_BAR "${FOO} bar"
          ESCAPED_VAR "Global variable name: \${FOO}"
          VAR_WITH_ESC_SEQUENCES "hex\t\x68\x65\x78"
          GREETING "Good evening!"

IO_REDIRECT = STDOUT "/var/log/net-dhcp.log" APPEND 0644
IO_REDIRECT = STDERR STDOUT
```
#### Explanation
- **NAME** -- The name given to this task configuration. Relevant if other tasks want to depend on this one. This is a
  mandatory setting.
- **INCLUDE** -- See section **Include files** below. (*array-like*)
- **COMMAND** -- The commands to be executed in series. Executable paths must be absolute. Execution will stop if
  one of them fails and the whole task will be considered failed. The whole task is considered finished (i.e.
  the `network-dhcp:wait` dependency is fulfilled) if the last command has successfully returned. If no **COMMAND** is
  given, the task is treated as a dependency group or "meta-task", see below. (*array-like*)
- **DEPENDS** -- A list of dependencies which need to be fulfilled before this task is considered "ready-to-start".
  Semantics are `<taskname>:{fail,wait,spawn}`, where `spawn` is fulfilled when (the first command of) a task has been
  started, `wait` if it has successfully completed, and `fail` if it has failed somewhere along the way. Here we can see
  this task is only run if and after the `earlysetup` (setup of system directories, etc.) has fully completed and the
  `check_qemu` task has determined we are _not_ running inside the emulator and therefore exited with an error code.
  This may be left out or ecplicitly set to empty using `""` which is interpreted as "no dependencies".
  There is also the special `@provided:feature` syntax where we can define we want to depend on a specific feature
  another task may implement (see **PROVIDES**). In this case `@provided:writable_var` could mean that another task
  may have mounted a tmpfs or a writable partition there which we need for the first `mkdir`. That task would need to
  advertise the `writable_var` feature in its `PROVIDES` config value. (*array-like*)
- **PROVIDES** -- As we have seen above, a task may depend on features and also provide them. In this case we advertise
  that after completion of this task (`wait`), the features `ipv4_dhcp` and `resolvconf` are provided. Another task may
  then depend e.g. on `@provided:resolvconf`. While the feature names chosen here reflect the functional intention, they
  can be chosen arbitrarily. (*array-like*)
- **RESPAWN** -- If set to `YES`, the task will be restarted on failure or completion. Useful for daemons like `getty`.
- **RESPAWN_RETRIES** -- Number of times a respawned task may fail *in a row* before it is not started again. The
  special value `-1` is interpreted as "unlimited". Default: -1
- **ENV_SET** -- See section **Setting Environment Variables** below. (*array-like*)
- **IO_REDIRECT** -- See section **IO Redirections** below. (*array-like*)

### Setting Environment Variables

Crinit supports setting environment variables in the global and task configurations as shown above. The variables in the
global config are valid for all tasks and may be locally overriden or referenced. The above examples together would
result in the following list of environment variables for the task `network-dhcp` (with explanatory comments).

```
FOO=foo
FOO_BAZ=foo baz                            # Expansion of variable set before in the same config.
FOO_BAR=foo bar                            # Expansion of global variable in task-local variable.
GREETING=Good evening!                     # Override of global variable.
ESCAPED_VAR=Global variable name: ${FOO}   # Avoid variable expansion through escaping.
VAR_WITH_ESC_SEQUENCES=hex  hex            # Support for escape sequences including hexadecimal bytes.
```

#### Ruleset

* A configuration file may have an unlimited number of `ENV_SET` statements, each specifying a single environment
  variable.
* `ENV_SET` statements must be of the form `ENV_SET = VARIABLE_NAME "variable content"`. The quotes around the variable
  content are mandatory and will not appear in the environment variable itself.
* Setting the same variable twice overrides the first instance.
* Variables can be referenced/expanded using sh-like `${VARIABLE_NAME}` syntax.
    + Expansion can be avoided by escaping with a `\`.
* Variables can reference all other variables set before it, globally and locally.
* Variables are set/processed in the order they appear in the config file.
* Common escape sequences are supported: `\a, \b, \n, \t, \$, \\, \x<two digit hex number>`.

### Include files

A crinit task configuration may reference multiple include files at any point in the task file. The effect is the same
as manually copying the contents of the include file at exactly that point in the task config, similar to C incluedes.
Additionally, it is possible to define an import list if not all settings in the include file should be applied.

An INCLUDE statement looks like
```
INCLUDE = <include_name> [list,of,imported,settings]
```
where `<include_name>` is the filename of the include without the ending and without the leading path. The imported
settings are defined as a comma-separated list of possible configuration options. If omitted, everything in the include
file is taken.

Currently only `IO_REDIRECT`, `DEPENDS`, and `ENV_SET` are supported in include files.

Example:
```
INCLUDE = server_settings ENV_SET,IO_REDIRECT
```
Imports only the `ENV_SET` and `IO_REDIRECT` settings from (assuming default values for include dir and suffix)
`/etc/crinit/server_settings.crincl`.

`server_settings.crincl` could look like
```
ENV_SET = HTTP_PORT "8080"
IO_REDIRECT = STDOUT /some/file.txt
IO_REDIRECT = STDERR STDOUT
DEPENDS = @provided:network
```
In the above case, the DEPENDS setting would be ignored.

### IO Redirections

Crinit supports per-task IO redirection to/from file and between STDOUT/IN/ERR using `IO_REDIRECT` statements in the
task configurations. The statements are of the form
```
<REDIRECT_FROM> <REDIRECT_TO> [ APPEND | TRUNCATE | PIPE ] [ OCTAL_MODE ]
```
Where `REDIRECT_FROM` is one of `{ STDOUT, STDERR, STDIN }`, and `REDIRECT_TO` may either also be one of those streams
or an absolute path to a file. `APPEND` or `TRUNCATE` signify whether an existing file at that location should be
appended to or truncated. Default is `TRUNCATE`. The special value `PIPE` is discussed below (see *Named Pipes*).
`OCTAL_MODE` sets the permission bits if the file is newly created. Default is `0644`.

Accordingly the statements in the example configuration above will result in `stdout` being redirected to the file
`/var/log/net-dhcp.log` in append mode. If the file does not yet exist, it will be created with permission bits `0644`.
The second statement then redirects stderr to stdout, capturing both in the log.

Other examples could be
```
IO_REDIRECT = STDERR "/var/log/err.log" APPEND
IO_REDIRECT = STDOUT "/dev/null"
```
to silence stdout and log stderr, or
```
IO_REDIRECT = STDIN /opt/data/backup.tar
IO_REDIRECT = STDOUT /opt/data/backup.tar.gz
```
to read stdin from file and capture stdout to another file. Stderr will go to console as normal.

#### Named pipes

As indicated above, `crinit` also accepts the setting `PIPE` instead of `APPEND` or `TRUNCATE`. With this setting,
`crinit` will ensure that the given path is a named pipe (also called a FIFO special file). This is useful to pipe the
output of one task to another.

A common example would be redirection of output to syslog. For that we would create two task files, one as the sender
and another as the receiver (using the `logger` utility provided by e.g. busybox and others).

**Sending Task**

```
NAME = some_task

COMMAND = /bin/echo "This output will be redirected to syslog!"
RESPAWN = NO
DEPENDS = ""

IO_REDIRECT = STDOUT "/tmp/some_task_log_pipe" PIPE 0640
```

**Receiving Task**

```
NAME = some_task_logger

COMMAND = /usr/bin/logger -t some_task
RESPAWN = YES
DEPENDS = ""

IO_REDIRECT = STDIN "/tmp/some_task_log_pipe" PIPE 0640
```

This will redirect the output of the `echo` command to the input of the `logger` utility and thereby to syslog. As is
shown, it is also possible to set permissions for named pipes (default will also be `0644`). It should be kept in mind
to keep the permissions the same in both tasks. Setting different permissions in the sending and receiving task of a
pipe creates a race condition where one of the tasks will be able to create the named pipe with its settings. The other
task will take it as-is, as long as it can access the file.

#### A note on buffering

By default, glibc will switch from line- to block-buffered mode when redirecting a stream to file. This may make it
hard to use e.g. `tail -f ...` to monitor the output in parallel. To get around that problem, one may use the `stdbuf`
utility (part of GNU coreutils).

In a `crinit` task, the following
```
NAME = line_buffered_task

COMMAND = /usr/bin/stdbuf -oL -eL <SOME_EXECUTABLE>

IO_REDIRECT = STDOUT "/some/where.txt"
              STDERR "/some/where/else.txt"
```

will result in line-buffered output to the files which can be monitored easily. For more details, see the `stdbuf` man
page.

### Dependency groups (meta-tasks)

A dependency group or meta-task is a task without any **COMMAND**s. The provided dependencies of the meta-task will be
fulfilled immediately once its own dependencies are fulfilled. This can be used to semantically combine different
dependencies into one. Reasons to do that can be semantic readability of the configs or to provide hook dependencies
for third-party applications having an opaque view of their target system.

As an example a dependency group and a task using it could look like

```
NAME = dep_grp_server

DEPENDS = @provided:sql-db @provided:network @provided:firewall-open-port httpd:spawn

PROVIDES = server:wait
```

```
NAME = local_http_client

COMMAND = /usr/bin/some-http-request localhost

DEPENDS = @provided:server
# or, with same effect: DEPENDS = dep_grp_server:wait
```

Semantically, this would mean `local_http_client` only cares about the server stuff being set up and running. This could
also be delivered by a third party with only the interface knowledge "You need to wait for the `server` dependency". How
to provide this dependency, with which tasks, and in what order is then up to the system integrator who maintains
`dep_grp_server`.

## crinit-ctl Usage Info

`crinit-ctl` is a CLI control program for `crinit` wrapping the client API functionality.

Below is its help output:
```
USAGE: crinit-ctl <ACTION> [OPTIONS] <PARAMETER> [PARAMETERS...]
  where ACTION must be exactly one of (including specific options/parameters):
     addtask [-f/--overwrite] [-i/--ignore-deps] [-d/--override-deps "depA:eventA depB:eventB [...]"] <PATH>
             - Will add a task defined in the task configuration file at <PATH> (absolute) to Crinit's task database.
               '-f/--overwrite' - Lets Crinit know it is fine to overwrite if it has already loaded a task
                    with the same name.
               '-d/--override-deps <dependency-list>' - Will override the DEPENDS field of the config file
                    with what is given as the parameter.
               '-i/--ignore-deps' - Shortcut for '--override-deps ""'.
   addseries [-f/--overwrite] <PATH>
             - Will load a series file from <PATH>. Options set in the new series file take precedence over
               current settings.
               '-f/--overwrite' - Lets Crinit know it is fine to overwrite if it has already loaded tasks
                    with the same name as those in the new series file.
      enable <TASK_NAME>
             - Removes dependency '@ctl:enable' from the dependency list of <TASK_NAME> if it is present.
     disable <TASK_NAME>
             - Adds dependency '@ctl:enable' to the dependency list of <TASK_NAME>.
        stop <TASK_NAME>
             - Sends SIGTERM to the PID of <TASK_NAME> if the PID is currently known.
        kill <TASK_NAME>
             - Sends SIGKILL to the PID of <TASK_NAME> if the PID is currently known.
     restart <TASK_NAME>
             - Resets the status bits of <TASK_NAME> if it is DONE or FAILED.
      status <TASK_NAME>
             - Queries status bits and PID of <TASK_NAME>.
      notify <TASK_NAME> <"SD_NOTIFY_STRING">
             - Will send an sd_notify-style status report to Crinit. Only MAINPID and READY are
               implemented. See the sd_notify documentation for their meaning.
        list
             - Print the list of loaded tasks and their status.
      reboot
             - Will request Crinit to perform a graceful system reboot. crinit-ctl can be symlinked to
               reboot as a shortcut which will invoke this command automatically.
    poweroff
             - Will request Crinit to perform a graceful system shutdown. crinit-ctl can be symlinked to
               poweroff as a shortcut which will invoke this command automatically.
  General Options:
        --verbose/-v - Be verbose.
        --help/-h    - Print this help.
        --version/-V - Print version information about crinit-ctl, the crinit-client library,
                       and -- if connection is successful -- the crinit daemon.
```

## Build Instructions
Executing
```
ci/docker-run.sh
```
will start a Docker container for the native host architecture with all necessary programs to build Crinit and its
Doxygen documentation and to run a short local demonstration.

It is possible to run the Docker container for a foreign architecture such as arm64 with the help of qemu-user-static
and binfmt-support. Make sure these packages are installed on your host system if you want to use this functionality.
All following commands to be run inside the container will be the same regardless of the architecture.
```
ci/docker-run.sh arm64
```

Inside the container, it is sufficient to run
```
ci/build.sh
```
which will compile the release configuration for `crinit`, the client library and crinit-ctl as well as a suite of RPMs.
The doxygen documentation is built as well. The script will copy relevant build artifacts to `result/`.

For debugging purposes, the debug configuration can be built with the following command. Optionally it is also possible
to enable AddressSanitizer (ASAN) for additional runtime checks or static analysis using `-fanalyzer` at compile-time.
```
ci/build.sh Debug --asan --analyzer
```

Afterwards, it is possible to run (also inside the container)
```
ci/demo.sh
```
for a short local demonstration of `crinit`'s client API using `crinit-ctl`.

A `clang-tidy` analysis of the source can be performed using
```
ci/clang-tidy.sh
```
This will also generate a `compile_commands.json`. The output will be saved to `result/clang-tidy`.

Unit tests or smoke tests can be run using the respective commands below. For the debug configuration, either of them
takes an additional `Debug` argument.
```
ci/run-utest.sh
ci/run-smoketests.sh
```

After a successful execution of `ci/build.sh`, it is also possible to create a Debian package using debbuild.
For the debug configuration, this also takes an additional `Debug` argument.
```
ci/package.sh
```

If a manual test build is desired, running the following command sequence
inside the container will setup the build system and build native binaries.
```
mkdir -p build/amd64
cmake -B build/amd64 -DCMAKE_BUILD_TYPE=Debug -DCMAKE_VERBOSE_MAKEFILE=On -DUNIT_TESTS=On
make -C build/amd64
```

The Doxygen documentation alone can be built using
```
make -C build/amd64 doxygen
```
