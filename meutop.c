#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/stat.h>

#define MAX_PATH 512
#define MAX_LINE 256
#define MAX_NAME 64
#define MAX_PROCESSES 40 // Limita a impressão aos 20 primeiros processos

// Função para obter o nome de usuário a partir do UID
const char* get_username(uid_t uid) {
    struct passwd *pwd = getpwuid(uid);
    return (pwd != NULL) ? pwd->pw_name : "unknown";
}

// Função para imprimir os 20 primeiros processos
void print_processes() {
    struct dirent *entry;
    DIR *proc_dir = opendir("/proc");
    int count = 0;

    if (proc_dir == NULL) {
        perror("Erro ao abrir /proc");
        exit(EXIT_FAILURE);
    }

    printf("%-10s %-10s %-15s %-10s\n", "PID", "USUÁRIO", "NOME", "ESTADO");
    printf("-----------------------------------------------------------\n");

    while ((entry = readdir(proc_dir)) != NULL && count < MAX_PROCESSES) {
        if (entry->d_type == DT_DIR && atoi(entry->d_name) > 0) {
            char status_path[MAX_PATH];
            snprintf(status_path, MAX_PATH, "/proc/%.250s/status", entry->d_name);

            FILE *status_file = fopen(status_path, "r");
            if (status_file != NULL) {
                char line[MAX_LINE];
                char state = '?';
                uid_t uid = 0;
                char name[MAX_NAME] = "unknown";
                int pid = atoi(entry->d_name);

                // Lê as linhas do arquivo status para obter o nome, UID e estado
                while (fgets(line, MAX_LINE, status_file)) {
                    if (strncmp(line, "Name:", 5) == 0) {
                        sscanf(line, "Name: %s", name);
                    } else if (strncmp(line, "State:", 6) == 0) {
                        sscanf(line, "State: %c", &state);
                    } else if (strncmp(line, "Uid:", 4) == 0) {
                        sscanf(line, "Uid: %d", &uid);
                    }
                }

                fclose(status_file);

                // Imprime as informações do processo
                printf("%-10d %-10s %-15s %-10c\n", pid, get_username(uid), name, state);
                count++;
            }
        }
    }

    closedir(proc_dir);
}

// Função para enviar sinais a um processo
void send_signal() {
    int pid, signal;
    printf("\nDigite o PID e o número do sinal (ex: 2131 1 para SIGHUP): ");
    if (scanf("%d %d", &pid, &signal) == 2) {
        if (kill(pid, signal) == 0) {
            printf("Sinal %d enviado ao processo %d com sucesso.\n", signal, pid);
        } else {
            perror("Erro ao enviar sinal");
        }
    } else {
        printf("Entrada inválida.\n");
    }
}

int main() {
    while (1) {
        system("clear"); // Limpa a tela para atualizar a tabela
        print_processes();

        // Permite que o usuário envie um sinal
        printf("\nDigite 's' para enviar um sinal ou 'q' para sair: ");
        char option;
        scanf(" %c", &option);

        if (option == 's') {
            send_signal();
        } else if (option == 'q') {
            break;
        }

        sleep(1); // Espera 1 segundo antes de atualizar novamente
    }

    return 0;
}