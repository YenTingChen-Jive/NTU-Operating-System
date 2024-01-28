// reference: 兩位助教, COOL交流版, B09902104

#include "param.h"
#include "types.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "defs.h"
#include "proc.h"

/* NTU OS 2022 */
/* Page fault handler */
int handle_pgfault() {
  /* Find the address that caused the fault */
  uint64 va = PGROUNDDOWN(r_stval());
  pte_t *pte = walk(myproc()->pagetable, va, 0);

  if ((*pte & 512) != 0)
    return madvise(va, PGSIZE, 2);

  char *pa = kalloc();
  memset(pa, 0, PGSIZE);

  mappages(myproc()->pagetable, va, PGSIZE, (uint64)pa, PTE_U | PTE_R | PTE_W | PTE_X);
  return 0;
  /* TODO */
  // panic("not implemented yet\n");
}
