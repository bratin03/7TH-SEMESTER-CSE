# Percentage of instructions consumed by GC

|   | full gc | incremental gc  | generational gc | generational gc 2 |   
|---|---|---|---|---|
|  m = 1000, n = 100 | 21.89%  | 14.33%  | 13.54% | 25.59% |
|  m = 100, n = 100 | 21.76%   | 17.47%  | 16.88% | 23.32% |
| m = 500, n = 100  | 21.87%  | 18.35%  | 17.62% | 24.10% |
| m = 5000, n = 100  | 27.81%  | 14.67%  | 14.18% | 21.46 % |

# Branch Misses, Page Faults, Cache Misses and Instructions per Cycle

|   | Branch Misses | Page Faults  | Cache Misses | Instructions per Cycle |
|---|---|---|---|---|
| full gc | 388033  | 16257  | 1658872 | 3.11 |
| incremental gc | 488390  | 11116  | 5139850 | 2.16 |
| generational gc | 478669  | 10707  | 5813572 | 2.11 |
| generational gc 2 | 487892  | 9279  | 7728757 | 1.56 |
| no gc | 392458  | 16258  | 1684245 | 3.07 |
