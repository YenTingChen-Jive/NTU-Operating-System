#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/vm.h"
#include "user/user.h"

#define PGSIZE 4096

int main(int argc, char *argv[]) {
  int result = 0;
  printf("# Append 3 new pages on the process with lazy allocation.\n");
  char *page1 = sbrk(PGSIZE * 3);
  char *page2 = page1 + PGSIZE;
  char *page3 = page2 + PGSIZE;
  printf("# Before any page fault\n");
  vmprint();
  printf("\n");

  /* Trigger page fault */
  *page2 = 1;
  printf("# After page fault on page2\n");
  vmprint();
  printf("\n");

  /* Run madvise(.., MADV_DONTNEED) */
  result = madvise(page1, PGSIZE * 3, MADV_DONTNEED);
  if (result != 0) {
    printf("error: return value of madvise() is incorrect.\n");
    exit(-1);
  }
  printf("*page2 = %d\n", *page2);    // 

  printf("# After MADV_DONTNEED(page1~page3)\n");
  vmprint();
  printf("\n");

  /* Trigger page fault */
  //page fault on page2(swapped page)
  printf("*page2 = %d\n", *page2);    // 
  if (*page2 != 1) {
    printf("error: *page 2 != 1\n");
    exit(-1);
  }
  printf("# After page fault on page2(swapped page)\n");
  vmprint();
  printf("\n");

  //page fault on page3
  *page3 = *page2 + 2;
  printf("# After page fault on page3\n");
  vmprint();
  printf("\n");

  //page fault on page1
  *page1 = 1;
  printf("# After page fault on page1\n");
  vmprint();
  printf("\n");

  //swap out page1
  result = madvise(page1, PGSIZE, MADV_DONTNEED);
  if (result != 0) {
    printf("error: return value of madvise() is incorrect.\n");
    exit(-1);
  }

  printf("# After MADV_DONTNEED(page1)\n");
  vmprint();
  printf("\n");


  exit(0);
}
