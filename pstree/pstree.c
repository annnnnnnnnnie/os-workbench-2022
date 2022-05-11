#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

static void print_version_info();
static void run_tests();

bool matched(const char *s1, const char *s2, const char *input) {
  return (strncmp(s1, input, strlen(s1) + 1) == 0 ||
          strncmp(s2, input, strlen(s2) + 1) == 0);
}

int main(int argc, char *argv[]) {
  run_tests();
  for (int i = 0; i < argc; i++) {
    // If -V or --version is set, print version info to stderr and exit
    assert(argv[i]);
    if (matched("-V", "--version", argv[i])) {
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

static void test_matched_ok() {
  const char *s1 = "-V";
  const char *s2 = "--version";
  const char *input = "--version";
  assert(matched(s1, s2, input));
}

static void test_no_match_extra() {
  const char *s1 = "-V";
  const char *s2 = "--version";
  const char *input_extra = "--versions";
  const char *input_short_extra = "-Vs";
  assert(!matched(s1, s2, input_extra));
  assert(!matched(s1, s2, input_short_extra));
}

static void run_tests() {
  test_matched_ok();
  test_no_match_extra();
}