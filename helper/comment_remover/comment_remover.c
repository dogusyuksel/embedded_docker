#include <assert.h>
#include <dirent.h>
#include <err.h>
#include <errno.h>
#include <fts.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/queue.h>
#include <sys/stat.h>
#include <sys/types.h>

#define VERSION "1.0.0"

#define MAX_LINE_CHAR_LEN 1024

#define PRINT_ERROR(s) fprintf(stderr, "(%s - %d): %s\n", __func__, __LINE__, #s)
#define PRINT_ERROR_PRM1(s, prm1) fprintf(stderr, "(%s - %d): %s %s\n", __func__, __LINE__, #s, prm1)

struct file_queue_entry {
    char *filename;
    TAILQ_ENTRY(file_queue_entry) entries;
};
TAILQ_HEAD(file_queue_head, file_queue_entry);

static struct option parameters[] = {
    {"help", no_argument, 0, 'h'},
    {"directory", required_argument, 0, 'd'},
    {"file", required_argument, 0, 'f'},
    {NULL, 0, 0, 0},
};

static void print_help(char *app_name) {
    int i = 0;

    if (!app_name) {
        return;
    }

    printf("usage of %s (version %s)\n", app_name, VERSION);
    while (parameters[i].name != NULL) {
        printf("\t--%s or -%c, %s\n", parameters[i].name, (char)parameters[i].val,
               (parameters[i].has_arg == no_argument)
                   ? ("no_argument")
                   : ((parameters[i].has_arg == required_argument) ? ("required_argument") : ("optional_argument")));
        i++;
    }
}

static bool is_dir_exist(char *directory) {
    DIR *dir = NULL;

    if (!directory) {
        return false;
    }

    dir = opendir(directory);
    if (dir) {
        closedir(dir);
        return true;
    }

    return false;
}

static void add_filename_to_queue(char *filename, struct file_queue_head *queue) {
    struct file_queue_entry *fentry;

    if (!filename || !queue) {
        return;
    }

    fentry = malloc(sizeof(struct file_queue_entry));
    if (!fentry) {
        PRINT_ERROR("malloc() failed");
        return;
    }

    fentry->filename = strdup(filename);
    if (!(fentry->filename)) {
        PRINT_ERROR("strdup() failed");
        return;
    }

    TAILQ_INSERT_HEAD(queue, fentry, entries);
}

static void print_queue(struct file_queue_head *queue) {
    int i = 0;
    struct file_queue_entry *np = NULL;

    if (!queue) {
        return;
    }

    TAILQ_FOREACH(np, queue, entries) {
        printf("entry %d\n", i);
        if (np->filename) {
            printf("\tfilename: %s\n", np->filename);
        }
        i++;
    }
}

static void free_queue(struct file_queue_head *queue) {
    struct file_queue_entry *n1, *n2 = NULL;

    if (!queue) {
        return;
    }

    n1 = TAILQ_FIRST(queue);
    while (n1 != NULL) {
        n2 = TAILQ_NEXT(n1, entries);
        if (n1->filename) {
            free(n1->filename);
        }
        if (n1) {
            free(n1);
        }
        n1 = n2;
    }
    TAILQ_INIT(queue);
}

static int ptree(char *curpath, char *const path, struct file_queue_head *queue) {
    char ep[512];
    char p[512];
    DIR *dirp;
    struct dirent entry;
    struct dirent *endp;
    struct stat st;

    if (curpath != NULL)
        snprintf(ep, sizeof(ep), "%s/%s", curpath, path);
    else
        snprintf(ep, sizeof(ep), "%s", path);

    if (stat(ep, &st) == -1)
        return EXIT_FAILURE;

    if ((dirp = opendir(ep)) == NULL)
        return EXIT_FAILURE;

    for (;;) {
        endp = NULL;
        if (readdir_r(dirp, &entry, &endp) == -1) {
            closedir(dirp);
            return EXIT_FAILURE;
        }
        if (endp == NULL)
            break;
        assert(endp == &entry);
        if (strcmp(entry.d_name, ".") == 0 || strcmp(entry.d_name, "..") == 0)
            continue;
        if (curpath != NULL)
            snprintf(ep, sizeof(ep), "%s/%s/%s", curpath, path, entry.d_name);
        else
            snprintf(ep, sizeof(ep), "%s/%s", path, entry.d_name);
        if (stat(ep, &st) == -1) {
            closedir(dirp);
            return EXIT_FAILURE;
        }
        if (S_ISREG(st.st_mode) || S_ISDIR(st.st_mode)) {
            if (!S_ISDIR(st.st_mode)) {
                if (ep[strlen(ep) - 2] == '.' && (ep[strlen(ep) - 1] == 'c' || ep[strlen(ep) - 1] == 'C'))
                    add_filename_to_queue(ep, queue);
                if (ep[strlen(ep) - 2] == '.' && (ep[strlen(ep) - 1] == 'h' || ep[strlen(ep) - 1] == 'H'))
                    add_filename_to_queue(ep, queue);
                if (ep[strlen(ep) - 2] == '.' && (ep[strlen(ep) - 1] == 'hpp' || ep[strlen(ep) - 1] == 'HPP'))
                    add_filename_to_queue(ep, queue);
                if (ep[strlen(ep) - 2] == '.' && (ep[strlen(ep) - 1] == 'cpp' || ep[strlen(ep) - 1] == 'CPP'))
                    add_filename_to_queue(ep, queue);
            }
        }

        if (S_ISDIR(st.st_mode) == 0)
            continue;
        if (curpath != NULL)
            snprintf(p, sizeof(p), "%s/%s", curpath, path);
        else
            snprintf(p, sizeof(p), "%s", path);
        snprintf(ep, sizeof(ep), "%s", entry.d_name);
        ptree(p, ep, queue);
    }
    closedir(dirp);

    return EXIT_SUCCESS;
}

static int find_files_in_directory(char *directory, struct file_queue_head *queue) {
    if (!directory || !queue) {
        return EXIT_FAILURE;
    }

    return ptree(NULL, directory, queue);
}

static void trim_beginning(char *input) {
    char *beg = NULL;
    int i = 0, len = 0;

    if (!input) {
        return;
    }

    len = strlen(input);
    for (i = 0; i < len; i++) {
        if (input[i] == '\r' || input[i] == '\n' || input[i] == '\t' || input[i] == ' ') {
            input[i] = ' ';
        } else {
            break;
        }
    }

    i = 0, len = 0;
    while ((beg = strstr(input, " ")) != NULL) {
        len = strlen(input);
        memmove(beg, beg + 1, len - (beg - input));
        beg = NULL;
    }
}

static void trim_end(char *input) {
    int i = 0, len = 0;

    if (!input) {
        return;
    }

    len = strlen(input);
    for (i = len - 1; i >= 0; i--) {
        if (input[i] == '\r' || input[i] == '\n' || input[i] == '\t' || input[i] == ' ') {
            input[i] = 0;
        } else {
            break;
        }
    }
}

static bool is_whole_unprintable(char *input) {
    int i = 0, len = 0;

    if (!input) {
        return;
    }

    len = strlen(input);

    for (i = 0; i < len; i++) {
        if (input[i] >= 0x20 && input[i] <= 0x7e) {
            return false;
        }
    }

    return true;
}

static int remove_comments_in_a_file(char *filename) {
    bool case3_started = false;
    bool last_line_was_unprintable = false;
    FILE *fs = NULL, *fs_new = NULL;
    char buffer[MAX_LINE_CHAR_LEN] = {0};
    char line_buffer[MAX_LINE_CHAR_LEN] = {0};
    char line_buffer_backup[MAX_LINE_CHAR_LEN] = {0};
    char new_file_name[MAX_LINE_CHAR_LEN] = {0};

    if (!filename) {
        return EXIT_FAILURE;
    }

    fs = fopen(filename, "r");
    if (!fs) {
        PRINT_ERROR("fopen() failed");
        EXIT_FAILURE;
    }

    snprintf(new_file_name, sizeof(new_file_name), "%s.edited", filename);
    fs_new = fopen(new_file_name, "w+");
    if (!fs_new) {
        PRINT_ERROR("fopen() failed");
        EXIT_FAILURE;
    }

    while (fgets(line_buffer, MAX_LINE_CHAR_LEN, fs) != NULL) {
        memcpy(line_buffer_backup, line_buffer, sizeof(line_buffer_backup));
        trim_beginning(line_buffer);
        trim_end(line_buffer);

        if (case3_started) {
            if (strlen(line_buffer) >= 2 && line_buffer[strlen(line_buffer) - 1] == '/' &&
                line_buffer[strlen(line_buffer) - 2] == '*') {
                case3_started = false;
            }
            goto clean_continue;
        }

        if (strlen(line_buffer) >= 2 && line_buffer[0] == '/' && line_buffer[1] == '/') {
            goto clean_continue;
        }

        if (strlen(line_buffer) >= 2 && line_buffer[0] == '/' && line_buffer[1] == '*' &&
            line_buffer[strlen(line_buffer) - 1] == '/' && line_buffer[strlen(line_buffer) - 2] == '*') {
            goto clean_continue;
        }

        if (strlen(line_buffer) >= 2 && line_buffer[0] == '/' && line_buffer[1] == '*') {
            case3_started = true;
            goto clean_continue;
        }

        if (is_whole_unprintable(line_buffer_backup) && last_line_was_unprintable == false) {
            fputs("\n", fs_new);
            last_line_was_unprintable = true;
            goto clean_continue;
        }

        if (is_whole_unprintable(line_buffer_backup) == false) {
            last_line_was_unprintable = false;
            fputs(line_buffer_backup, fs_new);
        }

    clean_continue:
        memset(line_buffer, 0, sizeof(line_buffer));
    }

    sync();

    if (fs) {
        fclose(fs);
    }

    if (fs_new) {
        fclose(fs_new);
    }

    snprintf(buffer, sizeof(buffer), "mv -f %s %s", new_file_name, filename);
    system(buffer);

    return EXIT_SUCCESS;
}

static int remove_comments(struct file_queue_head *queue) {
    struct file_queue_entry *np = NULL;

    if (!queue) {
        return EXIT_FAILURE;
    }

    TAILQ_FOREACH(np, queue, entries) {
        if (remove_comments_in_a_file(np->filename)) {
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}

int main(int argc, char **argv) {
    int ret = EXIT_SUCCESS;
    int c, o;
    char *directory = NULL;
    char *file = NULL;
    struct file_queue_head head;

    while ((c = getopt_long(argc, argv, "hd:f:", parameters, &o)) != -1) {
        switch (c) {
        case 'h':
            print_help(argv[0]);
            goto success;
        case 'd':
            directory = strdup(optarg);
            if (!directory) {
                PRINT_ERROR("strdup() failed");
            } else {
                if (directory[strlen(directory) - 1] == '/') {
                    directory[strlen(directory) - 1] = '\0';
                }
            }
            break;
        case 'f':
            file = strdup(optarg);
            if (!file) {
                PRINT_ERROR("strdup() failed");
            }
            break;
        default:
            PRINT_ERROR("option cannot be found");
            print_help(argv[0]);
            goto fail;
        }
    }

    if (!directory && !file) {
        PRINT_ERROR("you should point one directory or file out");
        print_help(argv[0]);
        goto fail;
    }

    TAILQ_INIT(&head);

    if (directory) {
        if (find_files_in_directory(directory, &head)) {
            PRINT_ERROR("find_files_in_directory() failed");
            print_help(argv[0]);
            goto fail;
        }
    } else if (file) {
        add_filename_to_queue(file, &head);
    } else {
        PRINT_ERROR("you should point one directory or file out");
        print_help(argv[0]);
        goto fail;
    }

    if (remove_comments(&head)) {
        PRINT_ERROR("remove_comments() failed");
        print_help(argv[0]);
        goto fail;
    }

    goto success;

fail:
    ret = EXIT_FAILURE;

success:

    free_queue(&head);
    if (file) {
        free(file);
    }
    if (directory) {
        free(directory);
    }

    return ret;
}