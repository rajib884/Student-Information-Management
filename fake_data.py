import struct
import random
import string

# Define constants
STUDENT_NAME_SIZE = 20
NUM_DEPARTMENTS = 11
NUM_STUDENTS = 100

GRADE_RANGE = (0, 100)

# Gender and department lists
GENDERS = ['m', 'f']
DEPARTMENTS = [{'id': i, 'name': f'Dept_{i}'} for i in range(NUM_DEPARTMENTS)]

# Function to generate a random name
def random_name(length):
    return ''.join(random.choices(string.ascii_letters, k=length))

def random_grades():
    return {
        'english': random.randint(*GRADE_RANGE),
        'math': random.randint(*GRADE_RANGE),
        'history': random.randint(*GRADE_RANGE)
    }

# Function to create random student data
def create_student_data():
    students = []
    for i in range(NUM_STUDENTS):
        student_id = i + 1
        name_length = random.randint(5, STUDENT_NAME_SIZE - 1) + 1
        name = random_name(name_length - 1)
        gender = random.choice(GENDERS)
        students.append((student_id, name, gender))
    return students

def create_grade_data(num_students):
    grades = []
    for i in range(num_students):
        student_id = i + 1
        grades.append((student_id, random_grades()))
    return grades

# Function to write student data to a binary file
def write_students_to_file(filename, students):
    with open(filename, 'wb') as file:
        for student_id, name, gender in students:
            name_length = len(name) + 1
            dept_id = random.randint(2, NUM_DEPARTMENTS + 1) 
            dept_id  = dept_id if dept_id < NUM_DEPARTMENTS else 0xFFFFFFFF
            file.write(struct.pack('I', student_id))                    # Write student ID
            file.write(struct.pack('B', name_length))                   # Write name length
            file.write(name.encode('utf-8') + b'\0')                    # Write name with null terminator
            file.write(gender.encode('utf-8'))                          # Write gender
            file.write(struct.pack('I', dept_id))                       # Write department ID

def write_grades_to_file(filename, grades):
    with open(filename, 'wb') as file:
        for student_id, grade in grades:
            file.write(struct.pack('I', student_id))                # Write student ID
            file.write(struct.pack('B', grade['english']))          # Write English grade
            file.write(struct.pack('B', grade['math']))             # Write Math grade
            file.write(struct.pack('B', grade['history']))          # Write History grade


# Generate random student data and write it to a file
students = create_student_data()
write_students_to_file('data/students.dat', students)

grades = create_grade_data(int(NUM_STUDENTS * 0.75))
write_grades_to_file('data/grades.dat', grades)


print(f'Generated {NUM_STUDENTS} students in students.bin')
