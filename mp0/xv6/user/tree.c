#include "../kernel/types.h"
#include "../kernel/fs.h"
#include "../kernel/stat.h"
#include "../user/user.h"

// reference: 助教們, COOL討論區, b09902104, b09902134

int level = 0;             // level
int total_count = 0;       // 總共的file + dir數 (不會超過20)
// int current_count = 0;  // 目前這一層的file + dir數 -> 放到迴圈裡面才不用一直重置！
int file_num = 0;          // 總共file數
int dir_num = 0;           // 總共dir數
int flag = 0;              // flag = 0時, 代表第一層

typedef struct dirent DIR;
typedef struct stat STAT;

char *fmtname(char *path) {         // 擷取最末的檔案名
    static char buf[DIRSIZ + 1];
    char *p;

    // Find first character after last slash.
    for (p = path + strlen(path); p >= path && *p != '/'; p--)
        ;
    p++;

    // Return blank-padded name.
    if (strlen(p) >= DIRSIZ)                
        return p;
    memmove(buf, p, strlen(p));
    memset(buf + strlen(p), ' ', DIRSIZ - strlen(p));
    return buf;
}

void ls(char *path, char indent[]) {          
    char buf[128]; 
    char *p;
    int fd;
    struct dirent de;
    struct stat st;

    if ((fd = open(path, 0)) < 0) {
        fprintf(2, "ls: cannot open %s\n", path);
        return;
    }

    if (fstat(fd, &st) < 0) {
        fprintf(2, "ls: cannot stat %s\n", path);
        close(fd);
        return;
    }

    char *name_array[20 - total_count];                  // 用來跑ls的
    for (int i = 0; i < 20 - total_count; i++)
        name_array[i] = malloc(75);                      // bug: 15*5夠嗎？
    int name_ptr = 0;
    DIR dir[20 - total_count];                           // 用來print (dir[i].name)
    int dir_ptr = 0;

    int current_count = 0;             // 目前這一層的file + dir數

    switch(st.type) {
        case T_FILE:
            if (level == 0)                              // 如果argv[1]就是file的話，直接print！
                printf("%s\n", fmtname(path));
            file_num += 1;
            // current_count += 1;
            break;

        case T_DIR:
            if ((strlen(path) + 1 + DIRSIZ + 1) > sizeof buf) {
                fprintf(2, "ls: path too long\n");
                break;
            }

            if (flag == 0)        // 第一層不算進去
                flag = 1;
            else
                dir_num += 1;

            strcpy(buf, path);
            p = buf + strlen(buf);
            *p++ = '/';                    // bug:檢查一下後面的反斜線！

            while (read(fd, &de, sizeof(de)) == sizeof(de)) {    // 先用while迴圈跑完所有子file, dir
                if (de.inum == 0)
                    continue;
                memmove(p, de.name, DIRSIZ);
                p[DIRSIZ] = 0;
                if (stat(buf, &st) < 0) {
                    printf("ls: cannot stat %s\n", buf);
                    continue;
                }

                if (de.name[0] - '.' != 0) {       // 不是.或..，就存下來！
                    dir[dir_ptr] = de;
                    dir_ptr += 1;
                    strcpy(name_array[name_ptr], buf);
                    name_ptr += 1;
                    current_count += 1;
                }
            }
            break;
    }
    for (int i = current_count; i < 20 - total_count; i++)    // 可以free的就先free！
        free(name_array[i]);
    total_count += current_count;
    
    level += 1;
    for (int i = 0; i < current_count; i++) {      // 處理前綴字串的reference: b09902134
        char *cur_ind;
        if (i == (current_count - 1))
            cur_ind = "    ";
        else
            cur_ind = "|   ";
        char result_indent[75];                    // bug: 15*5夠嗎？
        strcpy(result_indent, indent);
        strcpy(result_indent + strlen(result_indent), cur_ind);
        printf("%s|\n", indent);
        printf("%s", indent);
        printf("+-- %s\n", dir[i].name);

        ls(name_array[i], result_indent);
    }
    level -= 1;

    for (int i = 0; i < 20 - total_count && i < current_count; i++)
        free(name_array[i]);
    close(fd);
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(2, "not enough arguments!\n");
        exit(0);
    }

    int pfd[2];
    pipe(pfd);

    if (fork() == 0) {      // child
        close(pfd[0]);

        int fd;
        if ((fd = open(argv[1], 0)) < 0) {
            printf("%s [error opening dir]\n", argv[1]);
        }
        else {
            printf("%s\n", argv[1]);
            ls(argv[1], "");
        }
        write(pfd[1], &dir_num, sizeof(int));
        write(pfd[1], &file_num, sizeof(int));
    }
    else {                 // parent
        close(pfd[1]);

        char result_buf[10];
        int output_file, output_dir;
        read(pfd[0], result_buf, sizeof(int));
        memcpy(&output_dir, result_buf, sizeof(int));
        read(pfd[0], result_buf, sizeof(int));
        memcpy(&output_file, result_buf, sizeof(int));

        printf("\n");
        printf("%d directories, %d files\n", output_dir, output_file);
    }

    exit(0);
}
