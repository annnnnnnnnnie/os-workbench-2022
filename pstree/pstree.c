#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

static int print_pstree(bool should_show_pids, bool should_sort_numerically);
static void print_version_info();
static void run_tests();

bool matched(const char *s1, const char *s2, const char *input) {
  return (strncmp(s1, input, strlen(s1) + 1) == 0 ||
          strncmp(s2, input, strlen(s2) + 1) == 0);
}

int main(int argc, char *argv[]) {
  run_tests();
  bool should_show_pids = false;
  bool should_sort_numerically = false;
  for (int i = 0; i < argc; i++) {
    assert(argv[i]);

    if (matched("-p", "--show-pids", argv[i])) {
      should_show_pids = true;
    }

    if (matched("-n", "--numeric-sort", argv[i])) {
      should_sort_numerically = true;
    }

    // If -V or --version is set, print version info to stderr and exit
    if (matched("-V", "--version", argv[i])) {
      print_version_info();
      return 0;
    }


  }
  assert(!argv[argc]);
  
  int result = print_pstree(should_show_pids, should_sort_numerically);

  return result;
}

static int print_pstree(bool should_show_pids, bool should_sort_numerically) {
  printf("[Debug] printing pstree with arg %d %d\n", should_show_pids, should_sort_numerically);
  return 0;
}

// Prints version info to stderr
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

static void test_match_p_show_pid_ok() {
  const char *s1 = "-p";
  const char *s2 = "--show-pids";
  const char *input_short = "-p";
  const char *input_long = "--show-pids";
  assert(matched(s1, s2, input_short));
  assert(matched(s1, s2, input_long));
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
  test_match_p_show_pid_ok();
  test_no_match_extra();
}