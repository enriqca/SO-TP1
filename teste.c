#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>
#include <pwd.h>
#include <curses.h>

#define MAX_PROCESSES 1024

// Função para obter o nome do usuário pelo UID
const char *get_username(uid_t uid)
{
  struct passwd *pw = getpwuid(uid);
  return (pw != NULL) ? pw->pw_name : "unknown";
}

// Função para obter o estado do processo a partir do arquivo /proc/[pid]/stat
char get_process_state(const char *pid)
{
  char path[64], state;
  snprintf(path, sizeof(path), "/proc/%s/stat", pid);
  FILE *file = fopen(path, "r");
  if (file)
  {
    fscanf(file, "%*d %*s %c", &state); // Lê o estado do processo
    fclose(file);
  }
  else
  {
    state = '?';
  }
  return state;
}

// Função para obter o UID do processo
uid_t get_process_uid(const char *pid)
{
  char path[64], line[256];
  snprintf(path, sizeof(path), "/proc/%s/status", pid);
  FILE *file = fopen(path, "r");
  if (file)
  {
    while (fgets(line, sizeof(line), file))
    {
      if (strncmp(line, "Uid:", 4) == 0)
      {
        uid_t uid;
        sscanf(line, "Uid:\t%u", &uid);
        fclose(file);
        return uid;
      }
    }
    fclose(file);
  }
  return -1;
}

// Função para exibir os processos em uma tabela
void display_processes()
{
  DIR *dir = opendir("/proc");
  struct dirent *entry;
  int row = 2;

  clear();
  mvprintw(0, 0, " PID      User            State");
  mvprintw(1, 0, "----------------------------------");

  while ((entry = readdir(dir)) != NULL)
  {
    if (entry->d_type == DT_DIR && atoi(entry->d_name) > 0)
    {
      const char *pid = entry->d_name;
      uid_t uid = get_process_uid(pid);
      const char *user = get_username(uid);
      char state = get_process_state(pid);

      mvprintw(row++, 0, "%-8s %-15s %-5c", pid, user, state);
      if (row > MAX_PROCESSES)
        break;
    }
  }
  closedir(dir);
  refresh();
}

int main()
{
  initscr();             // Inicializa o modo ncurses
  cbreak();              // Desabilita o buffer de linha
  noecho();              // Não exibe o input do usuário
  nodelay(stdscr, TRUE); // Não bloqueia ao esperar por entrada

  while (1)
  {
    display_processes(); // Atualiza a lista de processos
    sleep(1);            // Aguarda 1 segundo
    if (getch() == 'q')  // Encerra o programa ao pressionar 'q'
      break;
  }

  endwin(); // Encerra o modo ncurses
  return 0;
}