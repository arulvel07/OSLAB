# Week 2 - Question 1: Web Server Log Analysis Pipeline

## Problem Statement
You have a web server log file `access.log` where each line follows some format. Write a command pipeline (use a set of commands) using `grep` to extract all IP addresses that received a `401` or `403` HTTP status code:
- Count how many times each such IP address appears.
- Display the top 5 offending IP addresses in descending order of count.

![Question](./Screenshot%202026-09-22%20135905.png)

---

## Command Pipeline

```bash
grep -E '" (401|403) ' access.log | grep -oE '^[0-9]+\.[0-9]+\.[0-9]+\.[0-9]+' | sort | uniq -c | sort -nr | head -5
```

### Breakdown of the Pipeline:
1. `grep -E '" (401|403) ' access.log`: Filters lines containing HTTP status `401` (Unauthorized) or `403` (Forbidden).
2. `grep -oE '^[0-9]+\.[0-9]+\.[0-9]+\.[0-9]+'`: Extracts only the IPv4 address at the start of each line.
3. `sort`: Sorts IP addresses alphabetically so identical IPs are adjacent.
4. `uniq -c`: Counts consecutive occurrences of each IP address.
5. `sort -nr`: Sorts numerically in descending (`-r`) order based on count.
6. `head -5`: Selects the top 5 offending IPs.

---

## Output Screenshot
![Output](./Screenshot%20from%202026-08-19%2014-39-33.png)
