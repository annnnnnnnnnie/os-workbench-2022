#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

typedef struct pstree_node {
  char name[128];
  pid_t pid;
  pid_t parent_pid;
  struct pstree_node *parent;
  struct pstree_node *children[128];
} pstree_node_t;

static int print_pstree(bool should_show_pids, bool should_sort_numerically);
static void print_version_info();
static void print_help_text();
static void run_tests();

bool matched(const char *s1, const char *s2, const char *input) {
  return (strncmp(s1, input, strlen(s1) + 1) == 0 ||
          strncmp(s2, input, strlen(s2) + 1) == 0);
}

int main(int argc, char *argv[]) {
  run_tests();

  bool should_show_pids = false;
  bool should_sort_numerically = false;

  if (argc <= 1) {
    int result = print_pstree(should_show_pids, should_sort_numerically);
    return result;
  }

  for (int i = 1; i < argc; i++) {
    assert(argv[i]);

    if (matched("-p", "--show-pids", argv[i])) {
      should_show_pids = true;
    } else if (matched("-n", "--numeric-sort", argv[i])) {
      should_sort_numerically = true;
    } else if (matched("-V", "--version", argv[i])) {
      // If -V or --version is set, print version info to stderr and exit
      print_version_info();
      return 0;
    } else {
      char bad_option[21];
      int buf_size = sizeof(bad_option) / sizeof(*bad_option);
      printf("[Debug] buf size is %d\n", buf_size);
      strncpy(bad_option, argv[i], buf_size);
      bad_option[20] = '\0';
      fprintf(stderr, "pstree: invalid option %s\n", bad_option);
      print_help_text();
      return EXIT_FAILURE;
    }
  }

  assert(!argv[argc]);

  int result = print_pstree(should_show_pids, should_sort_numerically);

  return result;
}

static int print_pstree(bool should_show_pids, bool should_sort_numerically) {
  printf("[Debug] printing pstree with arg %d %d\n", should_show_pids,
         should_sort_numerically);
  char PROCFS_ROOT[] = "/proc/";
  DIR *dir;
  struct dirent *dp;
  dir = opendir(PROCFS_ROOT);
  if (dir == NULL) {
    fprintf(stderr, "[Error] failed to open %s (%s)\n", PROCFS_ROOT,
            strerror(errno));
    return 1;
  }
  char buf[500];

  buf[500 - 1] = '\0';
  printf("%s\n", buf);

  return 0;
}

// Prints version info to stderr
static void print_version_info() {
  char *VERSION_TEXT = "pstree\n"
                       "Copyleft (C) 0000-0000";
  fprintf(stderr, "%s\n", VERSION_TEXT);
}

// Prints help text to stderr
static void print_help_text() {
  char *HELP_TEXT =
      "Usage: pstree [-p | --show-pids] [-n | --numeric-sort] [-V | --version]";
  fprintf(stderr, "%s\n", HELP_TEXT);
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