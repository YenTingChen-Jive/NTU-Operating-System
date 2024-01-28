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
  struct proc *p = myproc();
  pte_t *pte = walk(p -> pagetable, va, 0);
  if (*pte & PTE_S) return madvise(va, PGSIZE, 1);
  char *pa = kalloc();
  if (!pa) return -1;
  memset(pa, 0, PGSIZE);
  //if(mappages(p -> pagetable, va, PGSIZE, (uint64)pa, PTE_U | PTE_R | PTE_W | PTE_X) kfree(pa);
  mappages(p -> pagetable, va, PGSIZE, (uint64)pa, PTE_U | PTE_R | PTE_W | PTE_X);
  return 0;
}

