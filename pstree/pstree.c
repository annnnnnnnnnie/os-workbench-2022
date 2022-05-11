#include <stdio.h>
#include <assert.h>
#define RED "\e[0;31m"
#define NC "\e[0m"

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
  char *HELP_TEXT = RED "pstree\n"
                    NC "Copyleft (C) 0000-0000\n";
  fprintf(stderr, "%s", HELP_TEXT);
}