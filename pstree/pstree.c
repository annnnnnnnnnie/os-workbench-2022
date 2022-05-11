#include <stdio.h>
#include <assert.h>

static void print_help_info();

int main(int argc, char *argv[]) {
  for (int i = 0; i < argc; i++) {
    // If -V or --version is set, print version info to stderr and exit
    assert(argv[i]);
    printf("argv[%d] = %s\n", i, argv[i]);
    print_help_info();
  }
  assert(!argv[argc]);
  return 0;
}

static void print_help_info() {
  char *HELP_TEXT = "pstree\n"
                    "Copyleft (C) 0000-0000";
  fprintf(stderr, HELP_TEXT);
}