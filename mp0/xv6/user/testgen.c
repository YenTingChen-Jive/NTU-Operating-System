#include "kernel/types.h"

#include "kernel/fcntl.h"
#include "kernel/stat.h"
#include "user/user.h"
void mkfile(char *filename) {
    int fd = open(filename, O_CREATE | O_RDWR);
    write(fd, "hi", 3);
    close(fd);
}

void mkd(char *dirname) {
    if (mkdir(dirname) < 0) {
        fprintf(2, "mkdir %s failed.", dirname);
        exit(1);
    }
}
void test0() {
    mkd("os2022");

    mkd("os2022/d1");
    mkd("os2022/d2");
    mkd("os2022/d3");

    mkd("os2022/d2/a");
    mkd("os2022/d2/b");
    mkfile("os2022/d2/c");

    mkd("os2022/d3/a");
    mkfile("os2022/d3/b");
}

void test1() {
    mkd("a");
    mkd("a/0");
    mkd("a/1");
    mkd("a/2");
    mkd("a/3");
    mkd("a/4");
    mkd("a/5");
    mkd("a/6");
    mkd("a/7");
    mkd("a/8");
    mkd("a/9");
    mkd("a/10");
}

void test2() {
    mkd("mytest/");
    
    mkd("mytest/d1");
    mkfile("mytest/d1/a.txt");
    mkd("mytest/d1/b");
    mkd("mytest/d1/b/b1");
    mkfile("mytest/d1/b/b2.txt");

    mkd("mytest/d2");
    mkfile("mytest/d2/c.txt");
    mkfile("mytest/d2/d.txt");

    mkd("mytest/d3");
    mkd("mytest/d3/e");
    mkd("mytest/d3/f");
    mkfile("mytest/d3/f/f1.txt");
    mkd("mytest/d3/f/f2");
    mkfile("mytest/d3/f/f2/g1.txt");

}

int main(int argc, char *argv[]) {
    test0();
    test1();
    test2();
    exit(0);
}
