# Intelligent Traffic Signal Management System

## About

This repository contains the complete development of our ARM-based Intelligent Traffic Signal Management System.

The purpose of this repository is not only to store source code but also to organize documentation, project ideas, hardware information, and project progress throughout development.

---

# Repository Structure

```
Docs/
    Project documentation

Ideas/
    Individual project ideas from each team member

Hardware/
    Wiring diagrams, component list, images

Firmware/
    Source code

Testing/
    Test reports and observations

Resources/
    Reference material
```

---

# GitHub Workflow

This project follows a **Pull Request (PR) workflow**.

No one should push directly to the `main` branch.

Every feature or bug fix should be developed in its own branch and submitted as a Pull Request for review.

---

# First Time Setup

## Step 1 : Fork the Repository

Click the **Fork** button on GitHub.

This creates your own copy of the repository under your GitHub account.

Example

```
Original Repository
        │
        ▼
Your Fork
```

---

## Step 2 : Clone Your Fork

Open your fork.

Click **Code → HTTPS**

Copy the repository URL.

Open **Command Prompt**, **PowerShell**, or **Git Bash**.

Navigate to the folder where you want to keep the project.

Example

```bash
cd Documents
```

Clone your fork

```bash
git clone https://github.com/<your-username>/<repository>.git
```

Example

```bash
git clone https://github.com/john/Traffic-System.git
```

Move into the project

```bash
cd Traffic-System
```

---

## Step 3 : Open the Repository

If using VS Code

```bash
code .
```

If the `code` command is unavailable, simply open VS Code and choose

```
File → Open Folder
```

Select the cloned repository.

---

# Connecting to the Original Repository

Your cloned repository is currently connected only to **your fork**.

We also need to connect it to the original repository so you can receive updates.

Check current remotes

```bash
git remote -v
```

Add the original repository

```bash
git remote add upstream https://github.com/<organization>/<repository>.git
```

Verify

```bash
git remote -v
```

You should now see something similar to

```
origin
origin

upstream
upstream
```

- **origin** → Your fork
- **upstream** → Main project repository

---

# Daily Workflow

Before starting any work, always update your local repository.

### Step 1

Switch to main

```bash
git checkout main
```

### Step 2

Download the latest changes

```bash
git fetch upstream
```

### Step 3

Update your local main branch

```bash
git pull upstream main
```

### Step 4

Update your fork

```bash
git push origin main
```

Now your fork, local repository, and main repository are synchronized.

---

# Creating a New Feature

Never work directly on the main branch.

Create a new branch.

Example

```bash
git checkout -b feature/sensor-driver
```

Other examples

```bash
feature/motor-driver

feature/traffic-algorithm

feature/lcd

feature/documentation
```

---

# Saving Your Work

See which files changed

```bash
git status
```

Add your changes

```bash
git add .
```

Commit

```bash
git commit -m "Implemented IR sensor driver"
```

Push your branch

```bash
git push origin feature/sensor-driver
```

---

# Creating a Pull Request

After pushing your branch

1. Open your fork on GitHub.
2. GitHub will usually display a **Compare & Pull Request** button.
3. Click it.
4. Write a short description of what you changed.
5. Submit the Pull Request.

Your code will be reviewed before being merged into the main repository.

---

# Updating an Existing Branch

If the main repository changes while you are working

```bash
git checkout main

git fetch upstream

git pull upstream main

git checkout feature/sensor-driver

git merge main
```

Resolve conflicts if necessary.

Continue working.

---

# Helpful Commands

Current branch

```bash
git branch
```

Current status

```bash
git status
```

View commit history

```bash
git log --oneline
```

List remotes

```bash
git remote -v
```

Switch branches

```bash
git checkout branch-name
```

Delete local branch

```bash
git branch -d branch-name
```

---

# Repository Rules

- Do **not** push directly to `main`.
- Create a new branch for every feature or bug fix.
- Keep commits small and meaningful.
- Write clear commit messages.
- Test your changes before creating a Pull Request.
- Update your local repository before starting new work.
- If you are unsure about a change, ask before implementing it.

---

# Goal

This repository is intended to help every team member contribute in an organized manner while learning good software development practices. By following this workflow, we can keep the project stable, easy to review, and simple to maintain.