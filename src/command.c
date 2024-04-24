/*
 * command.c
 *  	
 *  Author: Joseph Erlinger
 * 		Date: April 17, 2024
 */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <libgen.h>
#include <limits.h>
#include "command.h"
#include "string_parser.h"


#define STDOUT 1


/* listdir() lists all files and directories on a single line
and not in any order. */
void listDir() 
{
    char *cwd;    // The absolute path that represents the current directory.
    DIR *dirp;    // A directory pointer that points to the current directory.
    struct dirent *next_dir; // Data structure of a file or directory.
    char *name;   // The name of a file or directory.

    /* Allocate memory for the current directory buffer */
    cwd = (char *) malloc(PATH_MAX*sizeof(char));
    
    /* Get the current directory */
    if (getcwd(cwd, PATH_MAX) == NULL) // Fill the cwd buffer with the absPath.
        goto cleanup;                                         // Exit on error.
    
    /* Open the current directory */
    dirp = opendir(cwd);
    if (dirp == NULL)
        goto cleanup;                                         // Exit on error.
    
    /* Write each file/directory name in the current directory to stdout. */
	for (struct dirent* next_dir; next_dir = readdir(dirp); next_dir != NULL)
    {
        char* name = next_dir->d_name;
		write(STDOUT, name, strlen(name));
		write(STDOUT, " ", 1);
    }
    write(STDOUT, "\n", 1);
    closedir(dirp);

    /* Free allocated memory before exiting */
    cleanup:
    free(cwd);
}   /* listDir */


void showCurrentDir() 
{
    char *cwd;    // The absolute path that represents the current directory.

    /* Allocate memory for the current directory buffer */
    cwd = (char *) malloc(PATH_MAX*sizeof(char));
    
    /* Get the current directory */
    if (getcwd(cwd, PATH_MAX) == NULL) // Fill the cwd buffer with the absPath.
        goto cleanup;                                         // Exit on error.
    
    /* Display the current directory to stdout */
    write(STDOUT, cwd, strlen(cwd));
    write(STDOUT, "\n", 1);
    
    /* Free allocated memory before exiting */
    cleanup:
    free(cwd);
}   /* showCurrentDir */


/* Files are created with the default mode (755) which grants the user read,
write, and execute privileges, and the group and others read and execute
privileges. */
void makeDir(char *dirName)
{
    mode_t mode;    // mode sets the privileges of the new directory.

    mode = S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH;    // Default mode.
    mkdir(dirName, mode);                     // Try to create a new directory.
}  


void changeDir(char *dirName)
{
    chdir(dirName);                               // Try to change directories.
}  


void deleteFile(char *filename)
{
    unlink(filename);                                // Try to remove the file.
} 


void displayFile(char *filename)
{
    int fd;                 // File descriptors for source file.
    ssize_t filepos;        // Variable to keep track of a file's position.
    ssize_t nwrite;         // Variable to keep track of # bytes written.
    char *line_buf;         // Buffer for transfering data from src to dst.

    /* Initialize file descriptor */
    fd = -1;
    
    /* Allocate memory for the line buffer. */
    line_buf = (char *) malloc(BUFSIZ*sizeof(char));

    /* Open the source file */
    fd = open(filename, O_RDONLY);      // Get file desciptor from source file.
    if (fd == -1)
        goto cleanup;                                         // Exit on error.

    /* Read each line from src file, and write the line to stdout */
    for(filepos=0; filepos=read(fd, line_buf, BUFSIZ); filepos!=0) {
        if (filepos == -1)
            goto cleanup;                                     // Exit on error.
        nwrite = write(STDOUT, line_buf, filepos);
        if (nwrite == -1)
            goto cleanup;                                     // Exit on error.
    }
    
    /* Close open files and free allocated memory before exiting. */
    cleanup:
    if (fd != -1)
        close(fd);
    free(line_buf);
    return;
}   /* displayFile */


void copyFile(char *sourcePath, char *destinationPath)
{
    int fd1, fd2;           // File descriptors for src file and dst file.
    ssize_t filepos;        // Variable to keep track of a file's position.
    ssize_t nwrite;         // Variable to keep track of # bytes written.
    int flags = O_WRONLY|O_CREAT|O_TRUNC;  // For opening the destination file.
    mode_t m1;              // Mode of source file.
    struct stat sb1, sb2;   // Stat buffers for source and destination.
    struct stat tb;         // Temp stat buffer.
    char *sp, *dp;          // Temp copies of sourcePath and destinationPath.
    char *bs, *dd;          // Basename of src file and dirname of dst file.
    int pathsize;           // Number of bytes in path string.
    char *path;             // path = "dirname_of_dstPath/basename_of_srcPath".
    char *line_buf;         // Buffer for transfering data from src to dst.

    /* Initialize file descriptors */
    fd1, fd2 = -1;

    /* Allocate memory for the line buffer. */
    line_buf = (char *) malloc(BUFSIZ*sizeof(char));

    /* Get basename of srcFile and directory path of dstFile */
    sp = strdup(sourcePath);
    bs = basename(sp);  //< Note: might modify sp.
    dp = strdup(destinationPath);

    pathsize = strlen(destinationPath) + strlen(bs) + 2;    // +2 for / and \0.
    path = (char *) malloc(pathsize*sizeof(char));

    /* If destinationPath ends with a backslash, then it explicitiy declares 
    itself as a directory. Otherwise, whether destinationPath is a directory
    or file is unknown. So extract the dirname from destination path. */
    if (dp[strlen(dp)-1] == '/') {
        dd = destinationPath;
        sprintf(path, "%s%s", destinationPath, bs);     // Concatenate strings.
    } else {
        dd = dirname(dp);
        sprintf(path, "%s/%s", destinationPath, bs);    // Concatenate strings.
    }

    /* Open the source file */
    fd1 = open(sourcePath, O_RDONLY);   // Get file desciptor from source file.
    if (fd1 == -1)
        goto cleanup;                                         // Exit on error.    
    if (stat(sourcePath, &sb1) == -1)         // Get metadata from source file.
        goto cleanup;                                         // Exit on error.
    m1 = sb1.st_mode;                           // Get mode of the source file.

    /* Open the destination file */
    if (stat(dd, &tb) != 0)                // Check if parent directory exists.
        goto cleanup;                                         // Exit on error.
    if (stat(destinationPath, &sb2) == -1) {   // Check if file at dstPath DNE.
        fd2 = open(destinationPath, flags, m1);                 // Create file.
        if (fd2 == -1)        // Common error when writing to a restricted dir.
            goto cleanup;                                     // Exit on error.
    }
    else if ((sb2.st_mode & S_IFMT) == S_IFREG) {      // Check if dst is file.
        if (unlink(destinationPath) == -1)             // Remove existing file.
            goto cleanup;                                     // Exit on error.
        fd2 = open(destinationPath, flags, m1);                 // Create file.
        if (fd2 == -1)
            goto cleanup;                                     // Exit on error.
    } else if (stat(path, &sb2) == 0) {    // Check if filename exists at path.
        if (unlink(path) == -1)                        // Remove existing file.
            goto cleanup;                                     // Exit on error.
        fd2 = open(path, flags, m1);                            // Create file.
        if (fd2 == -1) 
            goto cleanup;                                     // Exit on error.
    } else {                                     // Else, filename at path DNE.
        fd2 = open(path, flags, m1);                            // Create file.
        if (fd2 == -1)
            goto cleanup;                                     // Exit on error.
    }
    
    /* Supress the "File not found error" that occurs when creating a file. */
    if (errno == 2)
        errno = 0;                                // Set errno to "Successful".

    /* Copy data from source file to destination file */
    for(filepos=0; filepos=read(fd1, line_buf, BUFSIZ); filepos!=0) {
        if (filepos == -1)
            goto cleanup;                                     // Exit on error.
        nwrite = write(fd2, line_buf, filepos);
        if (nwrite == -1)
            goto cleanup;                                     // Exit on error.
    }

    /* Close any open files and free allocated memory before exiting. */
    cleanup:
    if (fd1 != -1)
        close(fd1); 
    if (fd2 != -1)
        close(fd2);
    free(sp); free(dp); free(path); free(line_buf);
    return;
}   /* copyFile */


void moveFile(char *sourcePath, char *destinationPath)
{
    char *sp1, *sp2;                              // Temp copies of sourcePath.
    char *dp1, *dp2;                         // Temp copies of destinationPath.
    char *bs, *ds;                       // basename and dirname of sourcePath.
    char *bd, *dd;                  // basename and dirname of destinationPath.

    sp1 = strdup(sourcePath);
    sp2 = strdup(sourcePath);
    dp1 = strdup(destinationPath);
    dp2 = strdup(destinationPath);
    bs = basename(sp1);
    bd = basename(dp1);
    ds = dirname(sp2);
    dd = dirname(dp2);

    if (strcmp(ds, dd) == 0 && strcmp(bd, ".") == 0)
        goto cleanup;       // Exit early if the new file replaced the old one.

    copyFile(sourcePath, destinationPath);
    if (errno != 0)
        goto cleanup;                                         // Exit on error.

    deleteFile(sourcePath);
    if (errno != 0)
        goto cleanup;                                         // Exit on error.
    
    cleanup:
    free(sp1); free(sp2); free(dp1); free(dp2);
    return;
}