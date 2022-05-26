#include <assert.h>
#include <ctype.h>
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

char PROCFS_ROOT[] = "/proc/";

static int print_pstree(bool should_show_pids, bool should_sort_numerically);
static int dir_is_pid(const struct dirent *d);
static bool string_is_number(const char *d_name, int max_length);
static void print_version_info();
static void print_help_text();
static void run_tests();

static pstree_node_t *build_pstree(pstree_node_t **pstree_nodes, int max_index);

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

static FILE *open_process_status_file(const char *pid_str) {
  FILE *fp;
  char p_file_name[512] = {0};
  char *status_file_name = "status";
  assert(strncat(p_file_name, PROCFS_ROOT, strlen(PROCFS_ROOT) + 1));
  assert(strncat(p_file_name, pid_str, sizeof(p_file_name) - 1));
  assert(strncat(p_file_name, "/", 2));
  assert(strncat(p_file_name, "status", strlen("status") + 1));
  // printf("[Debug] process status file full path: %s\n", p_file_name);
  fp = fopen(p_file_name, "r");
  return fp;
}

static void try_fill_in_process_name(pstree_node_t *pstree_node,
                                     const char *line) {
  // printf("[Debug] dealing with line: %s\n", line);
  const char *name_str = "Name:";
  if (strncmp(name_str, line, strlen(name_str)) == 0) {
    // printf("[Debug] chosen line: %s\n", line);
    // printf("[Debug] remaining line: %s\n", line + strlen(name_str) + 1);
    int i;
    const char *c;
    for (i = 0, c = line + strlen(name_str) + 1;
         *c != '\n' && *c != '\0' && i < 128; i++, c++) {
      pstree_node->name[i] = *c;
    }
    pstree_node->name[i] = '\0';
    pstree_node->name[127] = '\0';
    // printf("[Debug] name is %s\n", pstree_node->name);
  }
}

static void try_fill_in_ppid(pstree_node_t *pstree_node, const char *line) {
  const char *ppid_str = "PPid:";
  if (strncmp(ppid_str, line, strlen(ppid_str)) == 0) {
    int i;
    const char *c;
    char buf[128];
    for (i = 0, c = line + strlen(ppid_str) + 1;
         *c != '\n' && *c != '\0' && i < 128; i++, c++) {
      buf[i] = *c;
    }
    int ppid = atoi(buf);
    pstree_node->parent_pid = ppid;
  }
}

static void try_fill_in_pstree_node(pstree_node_t *pstree_node,
                                    const char *line) {
  try_fill_in_process_name(pstree_node, line);
  try_fill_in_ppid(pstree_node, line);
}

static void print_pstree_node(pstree_node_t *pstree_node) {
  printf("pstree node\n Name: %s\n pid: %d\n ppid: %d\n", pstree_node->name,
         pstree_node->pid, pstree_node->parent_pid);
  printf("\n");
}

static void print_pstree_nodes_list(pstree_node_t **pstree_nodes, int n_nodes) {
  for (int i = 0; i < n_nodes; i++) {
    print_pstree_node(pstree_nodes[i]);
  }
}

static void print_pstree_tree(pstree_node_t *root) {
  printf("(pid %d) %s --- ", root->pid, root->name);
  for(pstree_node_t **cur = root->children; *cur != NULL; cur++) {
    print_pstree_tree(*cur);
    printf("|---");
  }
  printf("\n");
}

static int print_pstree(bool should_show_pids, bool should_sort_numerically) {
  printf("[Debug] printing pstree with arg %d %d\n", should_show_pids,
         should_sort_numerically);

  struct dirent *ent;
  struct dirent **files;
  int n;
  if (should_sort_numerically) {
    n = scandir(PROCFS_ROOT, &files, dir_is_pid, alphasort);
  } else {
    n = scandir(PROCFS_ROOT, &files, dir_is_pid, NULL);
  }
  if (n < 0) {
    perror("Cannot open /proc");
    return 1;
  }

  int N_MAX_PSTREE_NODES = 1024;
  pstree_node_t **pstree_nodes =
      malloc(N_MAX_PSTREE_NODES * sizeof(*pstree_nodes));
  assert(pstree_nodes && "Failed to allocate memory for pstree");
  int pstree_node_index = 0;

  /* Loop through file names */
  for (int i = 0; i < n; i++) {
    /* Get pointer to file entry */
    struct dirent *ent = files[i];
    assert(ent->d_type == DT_DIR && "Should not encounter non pid dirs");
    // printf("[Debug] %s/\n", ent->d_name);

    /* get the file /proc/pid/status */
    FILE *fp;
    fp = open_process_status_file(ent->d_name);

    if (fp == NULL) {
      perror("Failed to open process status");
    } else {
      pstree_node_t *pstree_node = malloc(1 * sizeof(*pstree_node));
      if (!pstree_node) {
        perror("Failed to malloc for one pstree node");
        continue;
      }

      pstree_node->pid = atoi(ent->d_name);

      /* Read one line from /proc/pid/status */
      char buf[512];
      while (fgets(buf, sizeof(buf), fp)) {
        try_fill_in_pstree_node(pstree_node, buf);
      }
      // Add to the list
      pstree_nodes[pstree_node_index++] = pstree_node;
    }
  }

  /* Release file names */
  for (int i = 0; i < n; i++) {
    free(files[i]);
  }
  free(files);
  print_pstree_nodes_list(pstree_nodes, pstree_node_index);

  pstree_node_t *root = build_pstree(pstree_nodes, pstree_node_index);
  print_pstree_tree(root);

  free(root);

  /* Free pstree nodes*/
  for (int i = 0; i < pstree_node_index; i++) {
    free(pstree_nodes[i]);
  }
  free(pstree_nodes);

  return 0;
}

static void fill_in_children(pstree_node_t *root,
                             pstree_node_t **pstree_nodes, int max_index) {
  int root_pid = root->pid;
  int i = 0;
  for(int j = 0; j < max_index; j++) {
    pstree_node_t *current_pstree_node = pstree_nodes[j];
    int cur_ppid = current_pstree_node->parent_pid;
    if (cur_ppid == root_pid) {
      root->children[i++] = current_pstree_node;
      fill_in_children(current_pstree_node, pstree_nodes, max_index);
    }
  }
  root->children[i] = NULL;
}

static pstree_node_t *build_pstree(pstree_node_t **pstree_nodes, int max_index) {
  pstree_node_t *root = malloc(1 * sizeof(*root));
  strncpy(root->name, "root", sizeof("root") + 1);
  root->pid = 0;
  root->parent_pid = -1;
  fill_in_children(root, pstree_nodes, max_index);
  return root;
}

static int dir_is_pid(const struct dirent *d) {
  if (string_is_number(d->d_name, 256)) {
    return 1;
  }
  return 0;
}

static bool string_is_number(const char *str, int max_length) {
  const char *c;
  int i;
  for (c = str, i = 0; *c != '\0' && i < max_length; c++, i++) {
    if (!isdigit(*c)) {
      return false;
    }
  }
  return true;
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

/*=========== Tests ===================*/

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

static void test_numbers_can_be_recognized() {
  const char *s1 = "123";
  assert(string_is_number(s1, 3));
  const char *s2 = "123456789";
  assert(string_is_number(s2, 9));
}

static void test_non_numbers_can_be_recognized() {
  const char *s1 = "123a";
  assert(!string_is_number(s1, 4));
  const char *s2 = "123456789b";
  assert(!string_is_number(s2, 10));
}

static void run_tests() {
  test_matched_ok();
  test_match_p_show_pid_ok();
  test_no_match_extra();
  test_numbers_can_be_recognized();
  test_non_numbers_can_be_recognized();
}