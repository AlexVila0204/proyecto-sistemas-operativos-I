/**
 * @author: Alberth Godoy 12111345
 * @file shell.c
 * @brief Un shell personalizado
 *
 * Este programa es un shell personalizado que permite al usuario ejecutar comandos en la terminal.
 *
 */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <dirent.h>
#include <limits.h>
#include <math.h>
#include <ctype.h>
#include <signal.h>
#include <errno.h>
#include <bits/sigaction.h>  

#define MAX_LINE 80 
#define BIN_PATH "/usr/bin/"
#define BUFFER_SIZE 400
#define MAX_FRAME_SIZE 10000
#define MAX_FRAMES 100
#define MAX_LINE_LENGTH 100
#define ANIMATION_X 20 
#define ANIMATION_Y 5  
#define MAX_COMMANDS 1024  

static char buffer[BUFFER_SIZE];
static char frames[MAX_FRAMES][MAX_FRAME_SIZE];
static int num_frames = 0;
static char* command_list[MAX_COMMANDS];  
static int num_commands = 0; 

void setup(char inputBuffer[], char* args[], int* background);
char* find_closet_command(const char* input);
void show_welcome_banner(void);
void show_goodbye_banner(void);
void handle_SIGINT(int sig);
void load_end_animation();
void show_end_animation();


void clear_screen() {
    write(STDOUT_FILENO, "\033[H\033[J", 6);
}

void move_cursor(int x, int y) {
    char cmd[32];
    sprintf(cmd, "\033[%d;%dH", y, x);
    write(STDOUT_FILENO, cmd, strlen(cmd));
}

void skip_whitespace(FILE *file) {
    int c;
    while ((c = fgetc(file)) != EOF && (isspace(c) || c == ',' || c == '\n' || c == '\r'));
    if (c != EOF) ungetc(c, file);
}

void load_animation() {
    FILE *file = fopen("ascii-frames.json", "r");
    if (file == NULL) {
        printf("Error al abrir el archivo ascii-frames.json\n");
        return;
    }

    char line[MAX_LINE_LENGTH];
    int current_frame = 0;
    frames[0][0] = '\0';

    skip_whitespace(file);
    if (fgetc(file) != '[') {
        printf("Error: formato JSON inválido\n");
        fclose(file);
        return;
    }

    while (current_frame < MAX_FRAMES && fgets(line, sizeof(line), file)) {
        char *trimmed = line;
        while (*trimmed && isspace(*trimmed)) trimmed++;
        
        if (strstr(trimmed, "[") == trimmed) {
            if (current_frame > 0) strcat(frames[current_frame - 1], "\n");
            current_frame++;
            frames[current_frame - 1][0] = '\0';
            continue;
        }
        
        if (strstr(trimmed, "]") == trimmed) {
            continue;
        }

        if (*trimmed == '"') {
            char *content = trimmed + 1;
            char *end = strrchr(content, '"');
            if (end) *end = '\0';
            
            strcat(frames[current_frame - 1], content);
            strcat(frames[current_frame - 1], "\n");
        }
    }

    num_frames = current_frame;
    fclose(file);
}

void show_animation() {
    if (num_frames == 0) {
        printf("No se cargaron frames de animación\n");
        return;
    }

    for (int i = 0; i < 74; i++) {
        move_cursor(ANIMATION_X, ANIMATION_Y + i);
        write(STDOUT_FILENO, "                                                                                ", 80);
    }

    for (int i = 0; i < num_frames; i++) {
        move_cursor(ANIMATION_X, ANIMATION_Y);
        int line_count = 0;
        char *frame_copy = strdup(frames[i]);
        char *line = strtok(frame_copy, "\n");
        while (line != NULL && line_count < 74) {
            move_cursor(ANIMATION_X, ANIMATION_Y + line_count);
            write(STDOUT_FILENO, line, strlen(line));
            line = strtok(NULL, "\n");
            line_count++;
        }
        free(frame_copy);
        usleep(100000);
    }
}

void load_end_animation() {
    FILE *file = fopen("ascii-frames-end.json", "r");
    if (file == NULL) {
        printf("Error al abrir el archivo ascii-frames-end.json\n");
        return;
    }

    char line[MAX_LINE_LENGTH];
    int current_frame = 0;
    frames[0][0] = '\0';

    skip_whitespace(file);
    if (fgetc(file) != '[') {
        printf("Error: formato JSON inválido\n");
        fclose(file);
        return;
    }

    while (current_frame < MAX_FRAMES && fgets(line, sizeof(line), file)) {
        char *trimmed = line;
        while (*trimmed && isspace(*trimmed)) trimmed++;
        
        if (strstr(trimmed, "[") == trimmed) {
            if (current_frame > 0) strcat(frames[current_frame - 1], "\n");
            current_frame++;
            frames[current_frame - 1][0] = '\0';
            continue;
        }
        
        if (strstr(trimmed, "]") == trimmed) {
            continue;
        }

        if (*trimmed == '"') {
            char *content = trimmed + 1;
            char *end = strrchr(content, '"');
            if (end) *end = '\0';
            
            strcat(frames[current_frame - 1], content);
            strcat(frames[current_frame - 1], "\n");
        }
    }

    num_frames = current_frame;
    fclose(file);
}

void show_end_animation() {
    load_end_animation();
    show_animation();
}


void load_commands() {
    DIR* dir;
    struct dirent* entry;

  
    for (int i = 0; i < num_commands; i++) {
        free(command_list[i]);
    }
    num_commands = 0;



 
    if ((dir = opendir("/usr/bin")) != NULL) {
        while ((entry = readdir(dir)) != NULL && num_commands < MAX_COMMANDS) {
            if (entry->d_name[0] != '.') {  
         
                int exists = 0;
                for (int i = 0; i < num_commands; i++) {
                    if (strcmp(command_list[i], entry->d_name) == 0) {
                        exists = 1;
                        break;
                    }
                }
                if (!exists) {
                    command_list[num_commands] = strdup(entry->d_name);
                    num_commands++;
                }
            }
        }
        closedir(dir);
    }
}

int main(void) {
    char inputBuffer[MAX_LINE]; /* buffer to hold the command entered */
    int background;             /* equals 1 if a command is followed by '&' */
    char* args[MAX_LINE / 2 + 1];/* command line (of 80) has max of 40 arguments */


    struct sigaction sa;
    sa.sa_handler = handle_SIGINT;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);

    load_commands();

    show_welcome_banner();

    while (1) {            /* Program terminates normally inside setup */
        background = 0;
        printf("dwimsh> ");
        fflush(stdout);
        
        // Manejar la interrupción de read
        ssize_t length = read(STDIN_FILENO, inputBuffer, MAX_LINE);
        if (length == 0) {
            clear_screen();
            show_goodbye_banner();
            show_end_animation();
            exit(0);
        }
        if (length < 0) {
            if (errno == EINTR) {  // Si fue interrumpido por una señal
                continue;
            }
            perror("error reading the command");
            exit(1);
        }
        
        setup(inputBuffer, args, &background);       /* get next command */

        if (args[0] == NULL) continue;
        if (strcmp(args[0], "exit") == 0) {
            clear_screen();
            show_goodbye_banner();
            show_end_animation();
            break;
        }

        pid_t pid = fork();
        if (pid == 0) {
            
            

            execvp(args[0], args);
      
            char* suggested_command = find_closet_command(args[0]);
            if (suggested_command) {
                printf("Quisiste decir \"%s\"? [s/n]\n", suggested_command);
                fflush(stdout);
                char response;
                scanf(" %c", &response);
                getchar();
                if (response == 'y' || response == 'Y' || response == 's' || response == 'S') {
                    execvp(suggested_command, args);
                }
                free(suggested_command);
            }
            printf("Comando no encontrado\n");
            exit(1);
        }
        else if (pid > 0) {
            if (!background) {
                wait(NULL);
            }
        }
    }
    return 0;
}

/**
* setup() reads in the next command line, separating it into distinct tokens
* using whitespace as delimiters. setup() sets the args parameter as a
* null-terminated string.
*/

void setup(char inputBuffer[], char* args[], int* background) {
    int i,      /* loop index for accessing inputBuffer array */
        start,  /* index where beginning of next command parameter is */
        ct;     /* index of where to place the next parameter into args[] */

    ct = 0;
    start = -1;

    /* examine every character in the inputBuffer */
    for (i = 0; inputBuffer[i] != '\0'; i++) {
        switch (inputBuffer[i]) {
        case ' ':
        case '\t':               /* argument separators */
            if (start != -1) {
                args[ct] = &inputBuffer[start];    /* set up pointer */
                ct++;
            }
            inputBuffer[i] = '\0'; /* add a null char; make a C string */
            start = -1;
            break;

        case '\n':                 /* should be the final char examined */
            if (start != -1) {
                args[ct] = &inputBuffer[start];
                ct++;
            }
            inputBuffer[i] = '\0';
            args[ct] = NULL; /* no more arguments to this command */
            break;

        default:             /* some other character */
            if (start == -1)
                start = i;
            if (inputBuffer[i] == '&') {
                *background = 1;
                inputBuffer[i] = '\0';
            }
        }
    }
    args[ct] = NULL; /* just in case the input line was > 80 */
}

/**
 * @brief Calcula la distancia de Levenshtein entre dos cadenas
 * @param s1 La primera cadena
 * @param s2 La segunda cadena
 * @return La distancia de Levenshtein entre las dos cadenas
 */
int levenshtein(const char* s1, const char* s2) {
    int len1 = strlen(s1);
    int len2 = strlen(s2);
    int matrix[len1 + 1][len2 + 1];

    for (int i = 0; i <= len1; i++) {
        matrix[i][0] = i;
    }
    for (int j = 0; j <= len2; j++) {
        matrix[0][j] = j;
    }

    for (int i = 1; i <= len1; i++) {
        for (int j = 1; j <= len2; j++) {
            int cost = (s1[i - 1] == s2[j - 1]) ? 0 : 1;
            matrix[i][j] = fmin(fmin(matrix[i - 1][j] + 1, matrix[i][j - 1] + 1), matrix[i - 1][j - 1] + cost);
        }
    }
    return matrix[len1][len2];
}

char* find_closet_command(const char* input) {
    char* closest_command = NULL;
    int min_distance = INT_MAX;

    if (strlen(input) < 2) {
        return NULL;
    }

    for (int i = 0; i < num_commands; i++) {
        int len_diff = abs(strlen(input) - strlen(command_list[i]));
        if (len_diff > 2) continue;

        int distance = levenshtein(input, command_list[i]);
        if (distance < min_distance && distance <= (strlen(input) / 3 + 1)) {
            min_distance = distance;
            free(closest_command);
            closest_command = strdup(command_list[i]);
        }
    }
    return closest_command;
}

void show_welcome_banner(void) {
    load_animation();
    show_animation();
    
    printf("\n"
    "                    ___                       ___           ___           ___     \n"
    "     _____         /\\  \\                     /\\  \\         /\\__\\         /\\  \\    \n"
    "    /::\\  \\       _\\:\\  \\       ___         |::\\  \\       /:/ _/_        \\:\\  \\   \n"
    "   /:/\\:\\  \\     /\\ \\:\\  \\     /\\__\\        |:|:\\  \\     /:/ /\\  \\        \\:\\  \\  \n"
    "  /:/  \\:\\__\\   _\\:\\ \\:\\  \\   /:/__/      __|:|\\:\\  \\   /:/ /::\\  \\   ___ /::\\  \\ \n"
    " /:/__/ \\:|__| /\\ \\:\\ \\:\\__\\ /::\\  \\     /::::|_\\:\\__\\ /:/_/:\\:\\__\\ /\\  /:/\\:\\__\\\n"
    " \\:\\  \\ /:/  / \\:\\ \\:\\/:/  / \\/\\:\\  \\__  \\:\\~~\\  \\/__/ \\:\\/:/ /:/  / \\:\\/:/  \\/__/\n"
    "  \\:\\  /:/  /   \\:\\ \\::/  /   ~~\\:\\/\\__\\  \\:\\  \\        \\::/ /:/  /   \\::/__/     \n"
    "   \\:\\/:/  /     \\:\\/:/  /       \\::/  /   \\:\\  \\        \\/_/:/  /     \\:\\  \\     \n"
    "    \\::/  /       \\::/  /        /:/  /     \\:\\__\\         /:/  /       \\:\\__\\    \n"
    "     \\/__/         \\/__/         \\/__/       \\/__/         \\/__/         \\/__/    \n"
    "\n"
    "        Bienvenido a dwimsh - Shell personalizada creada por Alberth Godoy\n");
}

void show_goodbye_banner(void) {
    printf("\n"
    "        ¡Gracias por usar dwimsh! ¡Hasta la próxima!\n"
    "                   ¡Que tengas un excelente día!\n\n");
}

void handle_SIGINT(int sig) {
    printf("\n"); 
    clear_screen();  
    show_goodbye_banner();
    show_end_animation();
    exit(0);  
}