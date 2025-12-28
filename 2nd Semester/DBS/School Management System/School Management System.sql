use [School Management System]

CREATE TABLE Students( 
    StudentID INT PRIMARY KEY, 
    FirstName VARCHAR(50), 
    LastName VARCHAR(50), 
    DateOfBirth DATE, 
    Gender VARCHAR(10), 
    ClassID INT, --FOREIGN KEY REFERENCES Classes(ClassID)
    AdmissionDate DATE, 
    Email VARCHAR(100), 
    Phone VARCHAR(20), 
    Address VARCHAR(255) 
); 



CREATE TABLE Teachers ( 
    TeacherID INT PRIMARY KEY, 
    FirstName VARCHAR(50), 
    LastName VARCHAR(50), 
    Email VARCHAR(100) UNIQUE, 
    Phone VARCHAR(20), 
    SubjectID INT FOREIGN KEY REFERENCES Subjects(SubjectID) 
);

CREATE TABLE Subjects ( 
    SubjectID INT PRIMARY KEY, 
    SubjectName VARCHAR(100), 
    ClassLevel VARCHAR(50)
);

CREATE TABLE Enrollments ( 
    EnrollmentID INT PRIMARY KEY , 
    StudentID INT, 
    SubjectID INT, 
    FOREIGN KEY (StudentID) REFERENCES Students(StudentID), 
    FOREIGN KEY (SubjectID) REFERENCES Subjects(SubjectID) 
); 

CREATE TABLE Grades ( 
    GradeID INT PRIMARY KEY , 
    StudentID INT FOREIGN KEY REFERENCES Students(StudentID), 
    SubjectID INT FOREIGN KEY REFERENCES Subjects(SubjectID), 
    ExamScore DECIMAL(5,2), 
    Term VARCHAR(50), 
    Year INT 
); 

CREATE TABLE Attendance ( 
    AttendanceID INT PRIMARY KEY , 
    StudentID INT FOREIGN KEY REFERENCES Students(StudentID), 
    Date DATE, 
    Status VARCHAR(10) CHECK (Status IN ('Present', 'Absent', 'Late')), 
    RecordedBy INT FOREIGN KEY REFERENCES Teachers(TeacherID) 
); 

CREATE TABLE Users ( 
    UserID INT PRIMARY KEY , 
    Username VARCHAR(50) UNIQUE, 
    PasswordHash VARCHAR(255), 
    Role VARCHAR(20),  
    LinkedID INT  
); 

CREATE TABLE Timetable ( 
    TimetableID INT PRIMARY KEY , 
    ClassID INT, 
    SubjectID INT, 
    TeacherID INT, 
    DayOfWeek VARCHAR(20), 
    StartTime TIME, 
    EndTime TIME, 
    FOREIGN KEY (ClassID) REFERENCES Classes(ClassID), 
    FOREIGN KEY (SubjectID) REFERENCES Subjects(SubjectID), 
    FOREIGN KEY (TeacherID) REFERENCES Teachers(TeacherID) 
); 

CREATE TABLE SchoolYears (
    AcademicYearID INT PRIMARY KEY,
    YearStart DATE NOT NULL,
    YearEnd DATE NOT NULL,
    Term VARCHAR(20) NOT NULL, -- e.g., 'Term 1', 'Spring Semester', etc.
    Description VARCHAR(100) NULL, -- Optional: additional context like "2024–2025 Academic Year"
    CreatedDate DATETIME DEFAULT GETDATE()
);

CREATE TABLE Classes (
    ClassID INT PRIMARY KEY,
    ClassName VARCHAR(50) NOT NULL,
    AcademicYearID INT NOT NULL,
    RoomNumber VARCHAR(20) NULL,
    ClassTeacherID INT NULL, -- Optional: Link to Teachers table if each class has a designated teacher
    CreatedDate DATETIME DEFAULT GETDATE(),

    FOREIGN KEY (AcademicYearID) REFERENCES SchoolYears(AcademicYearID),
    FOREIGN KEY (ClassTeacherID) REFERENCES Teachers(TeacherID)
);
CREATE TABLE School(
    NAME VARCHAR(20)
);


SELECT * FROM Students;
SELECT * FROM Teachers;
SELECT * FROM Classes;
SELECT * FROM Attendance;
SELECT * FROM Enrollments;
SELECT * FROM Grades;
SELECT * FROM Subjects;
SELECT * FROM Timetable;
SELECT * FROM SchoolYears;
SELECT * FROM Users;

DROP TABLE School;

ALTER TABLE Students
DROP COLUMN Address;

ALTER TABLE Students
ALTER COLUMN FirstName VARCHAR(50) NOT NULL;

ALTER TABLE Students
ALTER COLUMN LastName VARCHAR(50) NOT NULL;

ALTER TABLE Teachers
ALTER COLUMN FirstName VARCHAR(50) NOT NULL;

ALTER TABLE Teachers
ALTER COLUMN LastName VARCHAR(50) NOT NULL;


ALTER TABLE Grades
ADD CONSTRAINT FK_Grades_Students
FOREIGN KEY (StudentID) REFERENCES Students(StudentID)
ON DELETE CASCADE;


ALTER TABLE Students
ADD CONSTRAINT FK_Students_ClassID
FOREIGN KEY (ClassID) REFERENCES Classes(ClassID)
ON DELETE SET NULL;


ALTER TABLE Attendance
ADD CONSTRAINT FK_Attendance_TeacherID
FOREIGN KEY (RecordedBy) REFERENCES Teachers(TeacherID)
ON DELETE SET NULL;



INSERT INTO Students (StudentID, FirstName, LastName, DateOfBirth, Gender, ClassID, AdmissionDate) VALUES
(1, 'Sara', 'Abbasi', '2015-03-03', 'Female', 2, '2020-05-05'),
(2, 'Hassan', 'Khan', '2013-03-09', 'Male', 2, '2023-06-11'),
(3, 'Fatima', 'Shaikh', '2012-01-15', 'Female', 3, '2022-04-12'),
(4, 'Ali', 'Zafar', '2011-07-01', 'Male', 4, '2023-04-10'),
(5, 'Ayan', 'Ali', '2012-09-10', 'Male', 1, '2021-08-20'),
(6, 'Laiba', 'Kamran', '2013-12-15', 'Female', 1, '2022-02-11'),
(7, 'Zoya', 'Mirza', '2014-07-21', 'Female', 2, '2023-01-17'),
(8, 'Hamza', 'Rehman', '2011-05-25', 'Male', 4, '2023-03-05'),
(9, 'Dua', 'Javed', '2013-11-08', 'Female', 3, '2022-09-09'),
(10, 'Saad', 'Farooq', '2012-03-18', 'Male', 3, '2022-11-01'),
(11, 'Aleena', 'Siddiqui', '2015-06-29', 'Female', 2, '2023-04-01'),
(12, 'Fahad', 'Hussain', '2011-10-04', 'Male', 4, '2023-04-10'),
(13, 'Ayesha', 'Yousaf', '2012-02-02', 'Female', 3, '2022-06-15'),
(14, 'Ahmed', 'Nawaz', '2014-04-19', 'Male', 2, '2023-02-12');




INSERT INTO Teachers (TeacherID, FirstName, LastName, Email, Phone) VALUES
(1, 'Hassan', 'Raza', 'hassan.raza@example.com', '03001234567'),
(2, 'Usman', 'Hashmi', 'usman.hashmi@example.com', '03019876543'),
(3, 'Zainab', 'Ahmed', 'zainab.ahmed@example.com', '03211223344'),
(4, 'Noman', 'Shah', 'noman.shah@example.com', '03451231234'),
(5, 'Farah', 'Iqbal', 'farah.iqbal@example.com', '03102345678'),
(6, 'Bilal', 'Chishti', 'bilal.chishti@example.com', '03334567891');





INSERT INTO Subjects (SubjectID, SubjectName, ClassLevel) VALUES
(1, 'Mathematics', 7),
(2, 'English', 8),
(3, 'Urdu', 7),
(4, 'Islamiyat', 7),
(5, 'Science', 8);




INSERT INTO Enrollments (EnrollmentID, StudentID, SubjectID) VALUES
(1, 1, 3),   
(2, 1, 5),   
(3, 2, 1),  
(4, 2, 4),   
(5, 3, 1),   
(6, 3, 3),
(7, 4, 5),  
(8, 4, 2),  
(9, 5, 1),
(10, 6, 3),
(11, 7, 1),  
(12, 7, 5), 
(13, 8, 4),
(14, 9, 2), 
(15, 10, 5), 
(16, 11, 3);




INSERT INTO Classes (ClassID, ClassName, AcademicYearID, RoomNumber, ClassTeacherID) VALUES
(1, 'Grade 6-A', 1, 'R-1', 1),
(2, 'Grade 6-B', 2, 'R-2', 5),
(3, 'Grade 7-A', 2, 'R-3', 3),
(4, 'Grade 8-A', 1, 'R-4', 4),
(5, 'Grade 8-B', 1, 'R-5', 2);




INSERT INTO Grades (GradeID, StudentID, SubjectID, ExamScore, Term, Year) VALUES
(1, 1, 5, 78, 'Term 1', 2023),  
(2, 1, 3, 85, 'Term 2', 2024),  
(3, 2, 1, 67, 'Term 2', 2024),  
(4, 2, 4, 88, 'Term 1', 2023),
(5, 3, 1, 91, 'Term 1', 2024),  
(6, 4, 5, 69, 'Term 2', 2024),  
(7, 5, 1, 84, 'Term 2', 2024), 
(8, 6, 3, 73, 'Term 1', 2023), 
(9, 7, 5, 82, 'Term 2', 2024),
(10, 8, 4, 68, 'Term 2', 2024),
(11, 9, 2, 79, 'Term 1', 2023),
(12, 10, 5, 65, 'Term 1', 2023),
(13, 11, 1, 92, 'Term 1', 2024),
(14, 11, 3, 87, 'Term 2', 2024),
(15, 12, 4, 74, 'Term 2', 2024),
(16, 13, 2, 88, 'Term 1', 2024),
(17, 14, 1, 81, 'Term 2', 2024);




INSERT INTO Attendance (AttendanceID, StudentID, Date, Status, RecordedBy) VALUES
(1, 1, '2025-04-15', 'Present', 1),   
(2, 2, '2025-04-15', 'Absent', 2),   
(3, 3, '2025-04-15', 'Present', 5),    
(4, 4, '2025-04-15', 'Present', 1),    
(5, 5, '2025-04-15', 'Present', 1),  
(6, 6, '2025-04-15', 'LATE', 6),  
(7, 7, '2025-04-15', 'Absent', 3), 
(8, 8, '2025-04-15', 'Present', 5),  
(9, 9, '2025-04-15', 'Present', 4),  
(10, 10, '2025-04-15', 'Present', 3),
(11, 11, '2025-04-15', 'Absent', 2),  
(12, 12, '2025-04-15', 'Present', 4),
(13, 13, '2025-04-15', 'LATE', 6),  
(14, 14, '2025-04-15', 'Present', 2); 




INSERT INTO Timetable (TimetableID, ClassID, SubjectID, TeacherID, DayOfWeek, StartTime, EndTime) VALUES
(1, 3, 3, 4, 'Monday', '09:00', '10:00'),
(2, 1, 4, 3, 'Wednesday', '09:00', '10:00'),
(3, 4, 5, 6, 'Thursday', '10:00', '11:00'),
(4, 5, 1, 2, 'Friday', '11:00', '12:00');



INSERT INTO SchoolYears (AcademicYearID, YearStart, YearEnd, Term, Description) VALUES
(1, '2023-04-01', '2024-03-31', 'Term 2', 'Session 2023-2024'),
(2, '2024-04-01', '2025-03-31', 'Term 2', 'Session 2024-2025');



UPDATE Attendance
SET Status = 'Absent'
Where AttendanceID=8;


DELETE FROM Students
Where StudentID=14;


SELECT s.FirstName, s.LastName, s.Gender
FROM Students as s
WHERE ClassID=2;


SELECT s.StudentID, s.FirstName, a.Status
FROM Students as s, Attendance as a
WHERE s.StudentID=a.StudentID AND Status='Present';

SELECT s.StudentID, s.FirstName, a.Status
FROM Students as s, Attendance as a
WHERE s.StudentID=a.StudentID AND Status='Absent';


SELECT s.StudentID, s.FirstName, g.ExamScore
FROM Students as s, Grades as g
WHERE s.StudentID=g.StudentID AND ExamScore>80;


SELECT s.StudentID, s.FirstName, a.Status, a.RecordedBy AS TeacherID, t.FirstName
From Students as s, Teachers as t, Attendance as a
Where s.StudentID=a.StudentID AND t.TeacherID=a.RecordedBy AND Status='LATE';


-- List all students with their class names

SELECT s.StudentID, s.FirstName, s.LastName, c.ClassName
FROM Students s
JOIN Classes c ON s.ClassID = c.ClassID;

-- Count of students in each class
SELECT c.ClassName, COUNT(s.StudentID) AS StudentCount
FROM Classes c
LEFT JOIN Students s ON c.ClassID = s.ClassID
GROUP BY c.ClassName;

-- Students who scored below passing mark (e.g., < 50)
SELECT s.FirstName, s.LastName, g.ExamScore
FROM Students s
JOIN Grades g ON s.StudentID = g.StudentID
WHERE g.ExamScore < 50;

-- Students enrolled in 'Mathematics'
SELECT DISTINCT s.FirstName, s.LastName
FROM Students s
JOIN Enrollments e ON s.StudentID = e.StudentID
JOIN Subjects sub ON e.SubjectID = sub.SubjectID
WHERE sub.SubjectName = 'Mathematics';




-- Teachers assigned to which classes and subjects
SELECT t.FirstName AS TeacherFirstName, t.LastName AS TeacherLastName,
       c.ClassName, sub.SubjectName
FROM Timetable tt JOIN Teachers t ON tt.TeacherID = t.TeacherID
JOIN Classes c ON tt.ClassID = c.ClassID
JOIN Subjects sub ON tt.SubjectID = sub.SubjectID;

-- Count how many subjects each teacher teaches
SELECT t.TeacherID, t.FirstName, COUNT(DISTINCT tt.SubjectID) AS TotalSubjects
FROM Teachers t JOIN Timetable tt 
ON t.TeacherID = tt.TeacherID
GROUP BY t.TeacherID, t.FirstName;





-- Total days each student was marked absent
SELECT s.FirstName, s.LastName, COUNT(*) AS AbsenceCount
FROM Students s
JOIN Attendance a ON s.StudentID = a.StudentID
WHERE a.Status = 'Absent'
GROUP BY s.FirstName, s.LastName;

-- Students with perfect attendance (never absent or late)
SELECT s.StudentID, s.FirstName, s.LastName
FROM Students s
WHERE NOT EXISTS (
    SELECT 1
    FROM Attendance a
    WHERE a.StudentID = s.StudentID
    AND a.Status IN ('Absent', 'Late')
);

-- Average score of each student
SELECT s.FirstName, s.LastName, AVG(g.ExamScore) AS AverageScore
FROM Students s
JOIN Grades g ON s.StudentID = g.StudentID
GROUP BY s.FirstName, s.LastName;

-- Top 3 students by average score
SELECT TOP 3 s.StudentID, s.FirstName, AVG(g.ExamScore) AS AvgScore
FROM Students s
JOIN Grades g ON s.StudentID = g.StudentID
GROUP BY s.StudentID, s.FirstName
ORDER BY AvgScore DESC;




-- List subjects taught on Monday
SELECT DISTINCT sub.SubjectName
FROM Timetable tt
JOIN Subjects sub ON tt.SubjectID = sub.SubjectID
WHERE DayOfWeek = 'Monday';

-- All classes running in a given academic year
SELECT c.ClassName, sy.Term, sy.YearStart, sy.YearEnd
FROM Classes c
JOIN SchoolYears sy ON c.AcademicYearID = sy.AcademicYearID
WHERE sy.YearStart = '2023-04-01';




-- Get all usernames with their roles
SELECT Username, Role
FROM Users;

-- Link teachers to users (if Role = 'Teacher')
SELECT u.Username, t.FirstName, t.LastName
FROM Users u
JOIN Teachers t ON u.LinkedID = t.TeacherID
WHERE u.Role = 'Teacher';






-- Students with highest scores in each subject
SELECT s.FirstName, s.LastName, g.SubjectID, g.ExamScore
FROM Grades g
JOIN Students s ON g.StudentID = s.StudentID
WHERE g.ExamScore = (
    SELECT MAX(ExamScore)
    FROM Grades g2
    WHERE g2.SubjectID = g.SubjectID
);

-- Classes that have more than 3 students
SELECT ClassID
FROM Students
GROUP BY ClassID
HAVING COUNT(StudentID) > 3;


-- Students who never had an absence
SELECT FirstName, LastName
FROM Students
WHERE StudentID NOT IN (
    SELECT StudentID
    FROM Attendance
    WHERE Status = 'Absent'
);





--VIEWS
-- View: Student Grades with Subject Names
CREATE VIEW vw_StudentGrades AS
SELECT s.FirstName, s.LastName, sub.SubjectName, g.ExamScore, g.Term, g.Year
FROM Grades g
JOIN Students s ON g.StudentID = s.StudentID
JOIN Subjects sub ON g.SubjectID = sub.SubjectID;

Select* from vw_StudentGrades;


-- View: Attendance Summary
CREATE VIEW vw_AttendanceSummary AS
SELECT s.StudentID, s.FirstName, 
       COUNT(CASE WHEN a.Status = 'Present' THEN 1 END) AS TotalPresent,
       COUNT(CASE WHEN a.Status = 'Absent' THEN 1 END) AS TotalAbsent,
       COUNT(CASE WHEN a.Status = 'Late' THEN 1 END) AS TotalLate
FROM Students s
JOIN Attendance a ON s.StudentID = a.StudentID
GROUP BY s.StudentID, s.FirstName;

Select* from vw_AttendanceSummary;


--View: Student Grades Summary
CREATE VIEW StudentGradesSummary AS
SELECT 
    s.StudentID,
    s.FirstName AS FullName,
    g.SubjectID,
    sub.SubjectName,
    g.ExamScore,
    g.Term,
    g.Year
FROM Grades g
JOIN Students s ON s.StudentID = g.StudentID
JOIN Subjects sub ON sub.SubjectID = g.SubjectID;

Select* from StudentGradesSummary;

--View: Daily Attendance Overview
CREATE VIEW DailyAttendance AS
SELECT 
    a.Date,
    s.StudentID,
    s.FirstName + ' ' + s.LastName AS FullName,
    a.Status,
    t.FirstName + ' ' + t.LastName AS MarkedBy
FROM Attendance a
JOIN Students s ON s.StudentID = a.StudentID
JOIN Teachers t ON t.TeacherID = a.RecordedBy;

Select* from DailyAttendance;


--View: Class List with Teacher Info
CREATE VIEW ClassWithTeachers AS
SELECT 
    c.ClassID,
    c.ClassName,
    t.FirstName + ' ' + t.LastName AS ClassTeacher,
    sy.Term,
    sy.Description
FROM Classes c
JOIN Teachers t ON t.TeacherID = c.ClassTeacherID
JOIN SchoolYears sy ON sy.AcademicYearID = c.AcademicYearID;

Select* from ClassWithTeachers;





--TRIGGERS


-- Trigger to Prevent Duplicate Attendance Entry per Day
CREATE TRIGGER trg_PreventDuplicateAttendance
ON Attendance
INSTEAD OF INSERT
AS
BEGIN
    INSERT INTO Attendance (AttendanceID, StudentID, Date, Status, RecordedBy)
    SELECT i.AttendanceID, i.StudentID, i.Date, i.Status, i.RecordedBy
    FROM inserted i
    WHERE NOT EXISTS (
        SELECT 1
        FROM Attendance a
        WHERE a.StudentID = i.StudentID AND a.Date = i.Date
    );
END;



ALTER TABLE Classes
ADD TeacherID INT NULL;

 -- Assign TeacherID = 1 if it was not provided
CREATE TRIGGER trg_AssignDefaultTeacher
ON Classes
AFTER INSERT
AS
BEGIN
    SET NOCOUNT ON;

    UPDATE c
    SET c.TeacherID = 1
    FROM Classes c
    INNER JOIN inserted i ON c.ClassID = i.ClassID
    WHERE i.TeacherID IS NULL;
END;



-- Insert default grade for each new enrollment
CREATE TRIGGER trg_InsertDefaultGrade
ON Enrollments
AFTER INSERT
AS
BEGIN
    SET NOCOUNT ON;

    INSERT INTO Grades (StudentID, SubjectID, ExamScore, Term, Year)
    SELECT 
        i.StudentID,
        i.SubjectID,
        NULL,                   m,k
        'Term 1',               
        YEAR(GETDATE())         
    FROM inserted i;
END;





--ASSERTIONS

--Simulated Assertion: No student can be enrolled in more than 6 subjects
CREATE TRIGGER trg_MaxEnrollmentsPerStudent
ON Enrollments
AFTER INSERT
AS
BEGIN
    IF EXISTS (
        SELECT StudentID
        FROM Enrollments
        GROUP BY StudentID
        HAVING COUNT(*) > 6
    )
    BEGIN
        RAISERROR ('A student cannot be enrolled in more than 6 subjects.', 16, 1);
        ROLLBACK TRANSACTION;
    END
END;



