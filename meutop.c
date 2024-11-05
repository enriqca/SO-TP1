#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>
#include <pwd.h>
#include <sys/types.h>

#define MAX_PROCESSES 1024

// Function to get the username from UID
const char *get_user_name(uid_t uid)
{
  struct passwd *pw = getpwuid(uid);
  if (pw != NULL) {
    return pw->pw_name;
  } else {
    return "Unknown";
  }
}

void print_process_info(const char *pid)
{
  char path[256];
  FILE *status_file;
  char line[256];
  char state[2] = "N"; // Default state as "N" (Not running)

  // Build the path to the process status file
  snprintf(path, sizeof(path), "/proc/%s/status", pid);

  status_file = fopen(path, "r");
  if (status_file == NULL)
  {
    perror("fopen");
    return;
  }

  uid_t uid = -1;
  char *username = NULL;

  // Print PID and read other process info
  while (fgets(line, sizeof(line), status_file) != NULL) {
    // Extract UID
    if (strncmp(line, "Uid:", 4) == 0){
      sscanf(line, "Uid:\t%u", &uid);
    }
    // Extract process state
    if (strncmp(line, "State:", 6) == 0)
    {
      sscanf(line, "State:\t%c", state);
    }
    // Extract process name
    if (strncmp(line, "Name:", 5) == 0)
    {
      // Print PID, user, state, and process name
      printf("%s\t|%s\t|%s\t|%s\n", pid, get_user_name(uid), line + 6, state);
      break;
    }
  }

  fclose(status_file);
}

int main() {
  struct dirent *entry;
  DIR *dir = opendir("/proc");

  printf("PID\t|User\t|PROCNAME\t|Estado\t|\n");
  printf("--------|-------|---------------|-------|\n");
  while ((entry = readdir(dir)) != NULL) {
    // Check if entry name is a number (process ID)
    if (atoi(entry->d_name) > 0) {
      print_process_info(entry->d_name);
    }
  }
}