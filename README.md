## Student Information Management

A terminal application in C for keeping students, departments and grades,
with the data structures written from scratch rather than taken from a
library.

Each of the three record types lives in a **sorted singly linked list** whose
operations — insert, search, delete and merge — take comparison and match
callbacks, so one list implementation serves all three without knowing what a
student or a department is. Listing students in order goes through a **heap**
used as an iterator: `sorted_student_init` builds it, then
`sorted_student_next` yields the next student until it runs out.

The menu is a tree assembled at run time, each entry holding a label, an
optional submenu and a function pointer to the action it runs, so adding a
command is one `add_menu` call rather than another branch in a switch.

Input runs in raw mode: options are chosen with the arrow keys, and text
fields take a filter as a function pointer so each field accepts only the
characters that belong in it.

Records are saved to and loaded from `data/*.dat`.

### Usage

1. **Compile the Code:**

   ```bash
   gcc main.c src/*.c -Iinclude -o main
   ```

   `make` does the same thing.

2. **Create the data directory** — *Save Data* writes `data/departments.dat`,
   `data/students.dat` and `data/grades.dat`, and will not create the folder
   for you.

   ```bash
   mkdir -p data
   ```

3. **Run the Program:**

   ```bash
   ./main
   ```

`fake_data.py` generates a populated database — 100 students across 11
departments, with grades — writing the same binary record layout the C code
reads, which is useful for testing the list and heap paths against something
larger than hand-typed input.

Terminal handling uses `termios` raw mode, so this builds on Linux, macOS and
WSL rather than natively on Windows.
