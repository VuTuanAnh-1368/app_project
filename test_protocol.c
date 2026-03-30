#include "protocol.h"

static void print_command(const Command *cmd)
{
    printf("type=%s, arg1='%s', arg2='%s'\n", command_type_to_string(cmd->type), cmd->arg1, cmd->arg2);
}

int main(void)
{
    const char *tests[] = {
        "CONNECT A",
        "SUB topic/value",
        "UNSUB topic/value",
        "PUB topic/value 123",
        "SEND B hello world",
        "PING",
        "QUIT",
        NULL
    };

    int i;
    Command cmd;

    for (i = 0; tests[i] != NULL; i++) {
        printf("INPUT: %s\n", tests[i]);

        if (parse_command(tests[i], &cmd) == 0) {
            print_command(&cmd);
        } else {
            printf("parse failed\n");
        }

        printf("\n");
    }

    return 0;
}
