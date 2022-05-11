#include <assert.h>
#include <stdio.h>
#include <string.h>

static void print_version_info();

int main(int argc, char *argv[]) {
  for (int i = 0; i < argc; i++) {
    // If -V or --version is set, print version info to stderr and exit
    assert(argv[i]);
    if (strncmp("-V", argv[i], 2) || strncmp("--version", argv[i], strlen("--version"))){
      print_version_info();
    }
    printf("argv[%d] = %s\n", i, argv[i]);
  }
  assert(!argv[argc]);
  return 0;
}

static void print_version_info() {
  char *VERSION_TEXT = "pstree\n"
                    "Copyleft (C) 0000-0000\n";
  fprintf(stderr, "%s", VERSION_TEXT);
}