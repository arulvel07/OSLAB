# Week 1 - Question 6: Linux CLI Directory Navigation & Pattern Searching

## Problem Statement
Create 2 directories named `Folder1` and `Folder2` on desktop. Inside `Folder1`, create a text file containing the text `INDIAN INSTITUTE OF INFORMATION TECHNOLOGY`. From `Folder2` use the `grep` command to search for the pattern `INDIAN` in the file and then display the complete contents of the file.

![Question](./Screenshot%202026-09-22%20135755.png)

---

## Terminal Commands & Execution

```bash
cd Desktop
mkdir Folder1 Folder2
echo "INDIAN INSTITUTE OF INFORMATION TECHNOLOGY" > Folder1/file.txt
cd Folder2
grep "INDIAN" ../Folder1/file.txt
cat ../Folder1/file.txt
```

---

## Output Screenshot
![Output](./Screenshot%20from%202026-08-12%2017-20-03.png)
