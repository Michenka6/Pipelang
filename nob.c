#define NOB_IMPLEMENTATION
#define NOB_EXPERIMENTAL_DELETE_OLD
#include "include/nob.h"

Cmd cmd = {0};

int main(int argc, char **argv)
{
	NOB_GO_REBUILD_URSELF(argc, argv);


	cmd_append(&cmd, "cc");
	cmd_append(&cmd, "-Wall");
	cmd_append(&cmd, "-Wextra");
	cmd_append(&cmd, "-Wpedantic");
	cmd_append(&cmd, "-Werror");
	cmd_append(&cmd, "-Wswitch-enum");
	cmd_append(&cmd, "-ggdb");
	cmd_append(&cmd, "-O0");
	cmd_append(&cmd, "main.c");
	cmd_append(&cmd, "src/lexer.c", "src/parser.c", "src/util.c", "src/pretty_printer.c", "src/cli_flags.c", "src/types.c", "src/ir.c", "src/codegen.c");
	cmd_append(&cmd, "-o", "pipe");
	cmd_append(&cmd, "-lm");
	if (!cmd_run(&cmd)) return 1;

	cmd_append(&cmd, "cc");
	cmd_append(&cmd, "-Wall");
	cmd_append(&cmd, "-Wextra");
	cmd_append(&cmd, "-Wpedantic");
	cmd_append(&cmd, "-Werror");
	cmd_append(&cmd, "-Wswitch-enum");
	cmd_append(&cmd, "-ggdb");
	cmd_append(&cmd, "-O0");
	cmd_append(&cmd, "-o", "benchmarks/c_benchmarks", "benchmarks/c_baseline_benchmarks.c");
	cmd_append(&cmd, "-lm");
	if (!cmd_run(&cmd)) return 1;

	return 0;
}
