# Attendance Management System

A **terminal-based, production-quality Attendance Management System** written in modern **C++17**. Designed with clean layered architecture, full OOP, STL data structures with justified choices, persistent CSV storage, undo/redo via Command Pattern, analytics, and a thread-safe logging system.

---

## Feature Set

| Feature | Details |
|---|---|
| **Student CRUD** | Add / remove / search students by name or section |
| **Attendance Marking** | Bulk (all students at once) or per-student; PRESENT / ABSENT / LATE / EXCUSED |
| **Duplicate Guard** | Cannot mark twice for same student + date; must use Edit |
| **Edit Attendance** | Change existing status; full undo/redo history |
| **Undo / Redo** | Full command-stack undo/redo for all mark/edit operations |
| **Teacher Comments** | Per-student, per-day remarks — persisted to CSV |
| **Analytics** | Attendance % per student; sorted worst-first |
| **Low Attendance** | Auto-flag students below 75% threshold |
| **File Persistence** | CSV-based save/load with per-row corruption isolation |
| **Report Generation** | `summary.txt` (architecture doc), `analytics_report.txt`, `low_attendance_report.txt` |
| **Logging** | Thread-safe singleton logger; timestamped action log to `data/system.log` |

---

## Architecture (5-Layer)

```
┌─────────────────────────────────────────┐
│  UI Layer          MenuController        │  ← Terminal I/O only
├─────────────────────────────────────────┤
│  Service Layer     Logger, FileManager,  │  ← Infrastructure
│                    AnalyticsEngine,      │
│                    ReportGenerator,      │
│                    UndoRedoStack         │
├─────────────────────────────────────────┤
│  Manager Layer     StudentRegistry,      │  ← Business Logic
│                    AttendanceManager     │
├─────────────────────────────────────────┤
│  Command Layer     ICommand,             │  ← Undo/Redo
│                    MarkAttendanceCommand,│
│                    EditAttendanceCommand │
├─────────────────────────────────────────┤
│  Entity Layer      Date, Person,         │  ← Domain Models
│                    Student, Teacher,     │
│                    AttendanceRecord,     │
│                    Comment               │
└─────────────────────────────────────────┘
```

---

## Project Structure

```
attendance_system/
├── CMakeLists.txt
├── build.ps1               ← PowerShell build script (MinGW)
├── main.cpp
├── include/
│   ├── entity/             Date, Person, Student, Teacher, AttendanceRecord, Comment
│   ├── command/            ICommand, MarkAttendanceCommand, EditAttendanceCommand
│   ├── manager/            StudentRegistry, AttendanceManager
│   ├── service/            Logger, FileManager, UndoRedoStack, AnalyticsEngine, ReportGenerator
│   ├── ui/                 MenuController
│   └── util/               Utils.h (currentTimestamp, trim, iequals)
├── src/                    ← mirrors include/ structure
├── test/
│   └── tests.cpp           ← self-contained test suite (no framework required)
├── data/                   ← auto-created at runtime
│   ├── students.csv
│   ├── attendance.csv
│   ├── comments.csv
│   └── system.log
└── reports/                ← auto-created at runtime
    ├── summary.txt
    ├── analytics_report.txt
    └── low_attendance_report.txt
```

---

## Build & Run

### Prerequisites
- **MinGW g++** with C++17 support (`g++ --version` ≥ 9)  
  OR **MSVC** 2019+ via CMake

### Option 1 — PowerShell script (MinGW, no CMake needed)
```powershell
cd attendance_system
.\build.ps1
.\ams.exe
```

### Option 2 — CMake (any generator)
```powershell
cmake -S . -B build
cmake --build build
.\build\ams.exe        # or .\build\Debug\ams.exe on MSVC
```

### Option 3 — Direct g++ one-liner
```powershell
g++ -std=c++17 -O2 -Iinclude -o ams.exe main.cpp (Get-ChildItem -Recurse src -Filter *.cpp)
.\ams.exe
```

---

## Build & Run Tests
```powershell
g++ -std=c++17 -Iinclude -o tests.exe test/tests.cpp (Get-ChildItem -Recurse src -Filter *.cpp)
.\tests.exe
```

---

## Data Structures — Design Decisions

| Container | Where Used | Why |
|---|---|---|
| `std::map<int, Student>` | StudentRegistry | O(log n) ordered lookup; `rbegin()` gives max ID for generation in O(1) |
| `std::unordered_map<Date, vector<AttendanceRecord>>` | AttendanceManager | O(1) date lookup; custom `std::hash<Date>` via YYYYMMDD integer packing |
| `std::stack<unique_ptr<ICommand>>` × 2 | UndoRedoStack | LIFO = natural reversal order; `unique_ptr` = exclusive ownership, no leaks |
| `std::vector<Comment>` | AttendanceManager | Comments are rare; flat list with on-read filtering is simpler than nested map |
| `std::vector<AttendanceRecord>` | Per-date bucket | Cache-coherent; insertion-ordered; supports SIMD-friendly iteration |

---

## Design Patterns

| Pattern | Implementation |
|---|---|
| **Command** | `ICommand` → `MarkAttendanceCommand`, `EditAttendanceCommand`; managed by `UndoRedoStack` |
| **Singleton** | `Logger::instance()` — Meyer's singleton; mutex-guarded for thread safety |
| **Template Method** | `Person` abstract base; `Student`/`Teacher` implement `role()` + `toCSVRow()` |
| **Repository** | `StudentRegistry` — typed collection with query methods |
| **Facade** | `FileManager::saveAll()` / `loadAll()` — hides all I/O detail |

---

## CSV File Format

```
students.csv:    id,name,role,section,enrollmentYear,rollNumber
attendance.csv:  studentId,date,status,teacherId,timestamp
comments.csv:    studentId,date,authorId,text,timestamp
```

- Each row parsed independently — corrupted rows are **skipped and logged**, not fatal.
- Comment text has embedded commas replaced with `;` before saving.
- Header line is consumed separately to prevent it from being parsed as data.

---

## Known Constraints & Scalability

- Designed for **50–5000 students** comfortably in RAM.
- For >5000: swap `FileManager` for SQLite; add per-student `map<int, vector<Date>>` reverse index.
- `UndoRedoStack` uses `std::stack` — replace with `std::deque` for hard `MAX_HISTORY` cap.
- `localtime_s` (Windows) / `localtime_r` (Linux) guard in all timestamp code.

---

## License
MIT
