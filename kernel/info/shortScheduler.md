flowchart TD
    A[run_short_scheduler()] --> B{Is CPU free?}
    B -- Yes --> C[return]
    B -- No --> D[get CPU by algorithm]
    D --> E{Is SJF?}
    E -- Yes --> F[Sort CPUs]
    E -- No --> G[Skip]
    F --> H[Return first CPU]
    G --> H
    H --> I[lock_cpu()]
    I --> J[lock_ready()]
    J --> K[lock_exec()]
    K --> L{PCB in exec()?}
    L -- Yes --> M[executing = pid]
    L -- No --> N[executing = -1]
    M --> O[unlock_exec()]
    N --> O
    O --> P{CPU processing AND preemption disabled?}
    P -- Yes --> Q[unlock_cpu()]
    Q --> R[unlock_ready()]
    R --> S[return]
    P -- No --> T[get next from ready()]
    T --> U{Is SJF?}
    U -- Yes --> V[Sort ready queue]
    U -- No --> W[Skip]
    V --> X[Return next process]
    W --> X
    X --> Y{Next is NULL?}
    Y -- Yes --> Z[unlock_cpu()]
    Z --> AA[unlock_ready()]
    AA --> AB[return]
    Y -- No --> AC{Should preempt current?}
    AC -- No --> AD[lock_exec()]
    AC -- Yes --> AE{Preemption disabled?}
    AE -- Yes --> AF[return false]
    AE -- No --> AG[compare CPU burst]
    AG --> AH[send & recv interrupt()]
    AH --> AI[lock_exec()]
    AI --> AJ{Same PID as executing?}
    AJ -- Yes --> AK[remove from exec()]
    AK --> AL[add to ready()]
    AL --> AM[executing = -1]
    AM --> AN[add to exec()]
    AD --> AN
    AN --> AO[send dispatch()]
    AO --> AP[current = next PID]
    AP --> AQ[unlock_ready()]
    AQ --> AR[unlock_exec()]
    AR --> AS[unlock_cpu_connections()]
    AS --> AT[unlock_cpu(&cpu->cpu_exec_sem)]
