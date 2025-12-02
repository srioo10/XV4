#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"
#define MAX_LINES 100
#define MAX_LINE_LEN 128
#define MAX_CONTENT 6000 // Enough for VNODE_DATA_BLOCKS * BSIZE

// Simple line-based text parser
int parse_lines(char *content, char lines[MAX_LINES][MAX_LINE_LEN],
                int *line_count) {
  int i = 0, j = 0, count = 0;

  while (content[i] && count < MAX_LINES) {
    j = 0;
    while (content[i] && content[i] != '\n' && j < MAX_LINE_LEN - 1) {
      lines[count][j++] = content[i++];
    }
    lines[count][j] = '\0';
    count++;

    if (content[i] == '\n')
      i++;
    if (content[i] == '\0')
      break;
  }

  *line_count = count;
  return 0;
}

// Compare two strings
int streq(char *a, char *b) {
  while (*a && *b && *a == *b) {
    a++;
    b++;
  }
  return *a == *b;
}

int main(int argc, char *argv[]) {
  static char content1[MAX_CONTENT];
  static char content2[MAX_CONTENT];
  static char lines1[MAX_LINES][MAX_LINE_LEN];
  static char lines2[MAX_LINES][MAX_LINE_LEN];
  int line_count1 = 0, line_count2 = 0;
  int ver1, ver2;
  struct version_info versions[MAX_VERSIONS_PER_FILE];
  int version_count;

  if (argc < 4) {
    printf(2, "Usage: diff <filename> <version1> <version2>\n");
    exit();
  }

  char *filename = argv[1];
  ver1 = atoi(argv[2]);
  ver2 = atoi(argv[3]);

  // Get version list to validate and get timestamps
  version_count = version_list(filename, versions, MAX_VERSIONS_PER_FILE);
  if (version_count < 0) {
    printf(2, "diff: failed to get versions for %s\n", filename);
    exit();
  }

  if (version_count == 0) {
    printf(2, "diff: no versions found for %s\n", filename);
    exit();
  }

  // Validate version numbers
  if (ver1 < 0 || ver1 >= version_count) {
    printf(2, "diff: version %d does not exist (valid: 0-%d)\n", ver1,
           version_count - 1);
    exit();
  }

  if (ver2 < 0 || ver2 >= version_count) {
    printf(2, "diff: version %d does not exist (valid: 0-%d)\n", ver2,
           version_count - 1);
    exit();
  }

  // Get content for both versions
  memset(content1, 0, MAX_CONTENT);
  memset(content2, 0, MAX_CONTENT);

  if (version_get_content(filename, ver1, content1, MAX_CONTENT) < 0) {
    printf(2, "diff: failed to read version %d\n", ver1);
    exit();
  }

  if (version_get_content(filename, ver2, content2, MAX_CONTENT) < 0) {
    printf(2, "diff: failed to read version %d\n", ver2);
    exit();
  }

  // Parse into lines
  parse_lines(content1, lines1, &line_count1);
  parse_lines(content2, lines2, &line_count2);

  // Print header
  printf(1, "Comparing %s: version %d -> version %d\n", filename, ver1, ver2);
  printf(1, "Version %d timestamp: %d (size: %d bytes)\n", ver1,
         versions[ver1].timestamp, versions[ver1].file_size);
  printf(1, "Version %d timestamp: %d (size: %d bytes)\n", ver2,
         versions[ver2].timestamp, versions[ver2].file_size);
  printf(1, "----------------------------------------\n");

  // Simple diff: show all lines with +/- prefix
  // This is a basic implementation - not a true LCS diff algorithm
  if (ver1 == ver2) {
    printf(1, "Versions are identical.\n");
    exit();
  }

  int added = 0, removed = 0;
  int i, j;

  // Find removed lines (in ver1 but not in ver2)
  for (i = 0; i < line_count1; i++) {
    int found = 0;
    for (j = 0; j < line_count2; j++) {
      if (streq(lines1[i], lines2[j])) {
        found = 1;
        break;
      }
    }
    if (!found && lines1[i][0] != '\0') {
      printf(1, "- %s\n", lines1[i]);
      removed++;
    }
  }

  // Find added lines (in ver2 but not in ver1)
  for (i = 0; i < line_count2; i++) {
    int found = 0;
    for (j = 0; j < line_count1; j++) {
      if (streq(lines2[i], lines1[j])) {
        found = 1;
        break;
      }
    }
    if (!found && lines2[i][0] != '\0') {
      printf(1, "+ %s\n", lines2[i]);
      added++;
    }
  }

  printf(1, "----------------------------------------\n");
  printf(1, "Summary: %d line(s) added, %d line(s) removed\n", added, removed);

  exit();
}

