// SPDX-License-Identifier: MIT
/**
 * @file case-success.c
 * @brief Unit test for crinitExpandPIDVariablesInSingleCommand(), successful execution.
 */

#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "confhdl.h"
#include "unit_test.h"
#include "utest-crinit-expand-pid-variables.h"

int crinitExpandPIDVariablesInSingleCommand(char *input, const pid_t pid, char **result);
void crinitExpandPIDVariablesInCommands(crinitTaskCmd_t *commands, size_t cmdsSize, const pid_t pid);

void crinitExpandPIDVariablesInSingleCommandOneVariableReplaced(void **state) {
    CRINIT_PARAM_UNUSED(state);

    char *result = NULL;
    assert_int_equal(crinitExpandPIDVariablesInSingleCommand("This is a test ${TASK_PID}.", 4711, &result), 1);

    assert_string_equal(result, "This is a test 4711.");

    free(result);
}

void crinitExpandPIDVariablesInSingleCommandTwoVariablesReplaced(void **state) {
    CRINIT_PARAM_UNUSED(state);

    char *result = NULL;
    assert_int_equal(crinitExpandPIDVariablesInSingleCommand(
                         "This is a test ${TASK_PID} with two occurences ${TASK_PID}. Blubb.", 4711, &result),
                     1);

    assert_string_equal(result, "This is a test 4711 with two occurences 4711. Blubb.");

    free(result);
}

void crinitExpandPIDVariablesInCommandsOneVariableInThreeArgv(void **state) {
    CRINIT_PARAM_UNUSED(state);

    char *argv1 = "TestCommand_1";
    char *argv2 = "TestCommand_2 ${TASK_PID}";
    char *argv3 = "TestCommand_3";
    const size_t argvLen = strlen(argv1) + strlen(argv2) + strlen(argv3) + 3;
    char *argvBuffer = calloc(argvLen, sizeof(char));
    assert_non_null(argvBuffer);
    strcpy(argvBuffer, argv1);
    strcpy(argvBuffer + strlen(argv1) + 1, argv2);
    strcpy(argvBuffer + strlen(argv1) + strlen(argv2) + 2, argv3);

    crinitTaskCmd_t cmd;
    cmd.argc = 3;
    cmd.argv = (char **)calloc(cmd.argc, sizeof(char *));
    cmd.argv[0] = argvBuffer;
    cmd.argv[1] = strchr(argvBuffer, '\0') + 1;
    cmd.argv[2] = strchr(cmd.argv[1], '\0') + 1;

    crinitExpandPIDVariablesInCommands(&cmd, 1, 4711);

    assert_string_equal(cmd.argv[0], "TestCommand_1");
    assert_string_equal(cmd.argv[1], "TestCommand_2 4711");
    assert_string_equal(cmd.argv[2], "TestCommand_3");

    free(cmd.argv[0]);
    free(cmd.argv);
}

void crinitExpandPIDVariablesInCommandsOneVariableInThreeCommands(void **state) {
    CRINIT_PARAM_UNUSED(state);

    const size_t cmdsSize = 3;
    crinitTaskCmd_t *cmds = (crinitTaskCmd_t *)calloc(cmdsSize, sizeof(crinitTaskCmd_t));
    cmds[0].argc = 1;
    cmds[0].argv = (char **)calloc(cmds[0].argc, sizeof(char *));
    cmds[0].argv[0] = strdup("TestCommand_1");
    cmds[1].argc = 1;
    cmds[1].argv = (char **)calloc(cmds[1].argc, sizeof(char *));
    cmds[1].argv[0] = strdup("TestCommand_2 ${TASK_PID}");
    cmds[2].argc = 1;
    cmds[2].argv = (char **)calloc(cmds[2].argc, sizeof(char *));
    cmds[2].argv[0] = strdup("TestCommand_3");

    crinitExpandPIDVariablesInCommands(cmds, cmdsSize, 4711);

    assert_string_equal(cmds[0].argv[0], "TestCommand_1");
    assert_string_equal(cmds[1].argv[0], "TestCommand_2 4711");
    assert_string_equal(cmds[2].argv[0], "TestCommand_3");

    free(cmds[0].argv[0]);
    free(cmds[0].argv);
    free(cmds[1].argv[0]);
    free(cmds[1].argv);
    free(cmds[2].argv[0]);
    free(cmds[2].argv);
    free(cmds);
}
