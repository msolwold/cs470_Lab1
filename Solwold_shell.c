#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#include <sys/wait.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <sys/vmmeter.h>
#include <mach/notify.h>
#include <mach/mach_host.h>
#include <signal.h>

char shell_prompt[30] = "cwushell";
int previous_exit_status = 0;

/**
 * Cleans the input from user
 * Strips trailing new line
 *
 * @param input - line read from terminal
 * @return - int
 *  1 : valid
 */
int cleanInput(char *input)
{
    for (int i = 0; i < strlen(input); i++)
    {
        if (input[i] == '\n')
        {
            input[i] = '\0';
            break;
        }
    }
    return 1;
}

/**
 * Verifies if valid input is given
 *
 * Valid :
 *  No double spaces
 *
 * @param input - cleaned input from user
 * @return - int
 *  1 : valid
 *  2 : too many spaces
 */
int verifyInput(char *input)
{
    for (int i = 0; i < strlen(input); i++)
    {
        if (input[i] == ' ' && input[i + 1] == ' ')
            return 2;
    }
    return 1;
}

/**
 * Parse user input into char * [<program>, <switch>]
 *
 * @param input - input from user
 * @param parsedInput - char * that holds parsed info
 */
void parseInput(char *input, char *parsedInput[])
{
    char **ap, *argv[2];

    for (ap = argv; (*ap = strsep(&input, " ")) != NULL;)
        if (**ap != '\0')
            if (++ap >= &argv[2])
                break;

    parsedInput[0] = argv[0];
    parsedInput[1] = argv[1];
}

/**
 * Manuals for all commands
 *
 * help | man | manual
 * prompt help | -h
 * cpuinfo help | -h
 * meminfo help | -h
 * exit help | -h
 */
int help()
{
    printf("\nThis shell was created by Michael Solwold for a CS470 Lab.\n");
    printf("This shell is intended to be ran on macOS\n\n");
    printf("System Commands : \n\n");
    printf("\t System commands that output information such as ls or pwd are available\n");
    printf("\t This shell is unable to change directory\n\n");
    printf("Custom Commands : use '[command] -help'. (eg. 'prompt -help')\n\n");
    printf("\t prompt [OPTION]\t\t\t\tChanges the prompt preceding the carrot\n");
    printf("\t cpuinfo -<switch>\t\t\t\tProvides information related to the systems CPU\n");
    printf("\t meminfo -<switch>\t\t\t\tProvides information related to the systems Memory\n");
    printf("\t exit [n]\t\t\t\t\tExit the shell\n\n");
    return 1;
}

void promptHelp()
{
    printf("prompt [OPTION]\n");
    printf("\tChanges the prompt preceding the carrot.\n");
    printf("\tDefault Prompt : 'cwushell'\n");
    printf("\tValid Options : single words\n");
    printf("\tIf no option is given, the default prompt will be set\n");
}

void cpuInfoHelp()
{
    printf("cpuinfo -<switch>\n");
    printf("\tProvides information related the systems CPU\n");
    printf("\tValid switches :\n");
    printf("\t\t-c\t\t\tCPU Clock Speed\n");
    printf("\t\t-t\t\t\tCPU Type\n");
    printf("\t\t-n\t\t\tNumber of cores\n");
}

void memInfoHelp()
{
    printf("cpuinfo -<switch>\n");
    printf("\tProvides information related the systems memory\n");
    printf("\tValid switches :\n");
    printf("\t\t-t\t\t\tTotal system RAM\n");
    printf("\t\t-u\t\t\tAmount of RAM in use\n");
    printf("\t\t-c\t\t\tSize of L2 cache / core\n");
}

void exitHelp()
{
    printf("exit [n]\n");
    printf("\tExits the shell with the return code given ([n])\n");
    printf("\tIf no code is given, the exit code from the previous program will be used\n");
}

/**
 * exit [n]
 *
 * terminates the shell with value n
 * if no value is given, exit with value of last executed command
 *
 * @return - int
 *  4 - use previous
 *  5 - user provided
 */
int exitShell(char *command[])
{
    if (command[1] == NULL)
        return 4;
    else if (
        strcmp(command[1], "-help") == 0 ||
        strcmp(command[1], "-h") == 0)
    {
        exitHelp();
        return 1;
    }
    else
        return 5;
}

/**
 * prompt <new_prompt>
 *
 * Change the current shell prompt to the new_prompt
 * Typing prompt should change back to default
 *
 * default: cwushell
 *
 * @return int -
 *  1 - Successful
 *  2 - Unsuccessful (Error)
 *  14 - Set Default Prompt
 *  15 - Set User Provided Prompt
 */
int changePrompt(char *command[])
{
    if (command[1] == NULL)
        return 14;
    else if (
        strcmp(command[1], "-help") == 0 ||
        strcmp(command[1], "-h") == 0)
    {
        promptHelp();
        return 1;
    }
    else if (strlen(command[1]) > 30)
    {
        printf("The value provided is too long.\n");
        return 2;
    }
    else
        return 15;
}

/**
 * cpuinfo -<switch>
 *
 * prints cpu info based on switch
 * -c cpu clock
 * -t cpu type
 * -n number of cores
 *
 * @return - int
 *  1 - Successful
 *  2 - Unsuccessful (Error)
 *  3 - Unsuccessful (Bad Switch)
 */
int cpuInfo(char *command[])
{
    if (command[1] == NULL)
    {
        printf("No switch given. Format: cpuinfo -<switch>. Use 'cpuinfo -help' for help.\n");
        return 3;
    }

    if (strcmp(command[1], "-c") == 0)
    {
        int mib[2] = {CTL_HW, HW_CPU_FREQ};
        size_t len;
        unsigned int info;
        len = sizeof(info);
        if (sysctl(mib, 2, &info, &len, NULL, 0) >= 0)
        {
            printf("CPU Clock Speed: %.1f GHz  \n", ((float)info / 1000000000));
        }
        else
        {
            printf("Failed to get hardware statistics: %s\n", strerror(errno));
            return 2;
        }
    }
    else if (strcmp(command[1], "-t") == 0)
    {
        size_t len;
        sysctlbyname("machdep.cpu.vendor", NULL, &len, NULL, 0);
        if (len)
        {
            char info[len];
            sysctlbyname("machdep.cpu.vendor", &info, &len, NULL, 0);
            printf("CPU Type: %s\n", info);
        }
        else
        {
            printf("Failed to get hardware statistics: %s\n", strerror(errno));
            return 2;
        }
    }
    else if (strcmp(command[1], "-n") == 0)
    {
        printf("Number of Cores: %ld\n", sysconf(_SC_NPROCESSORS_CONF));
    }
    else if (
        strcmp(command[1], "-help") == 0 ||
        strcmp(command[1], "-h") == 0)
    {
        cpuInfoHelp();
    }
    else
    {
        printf("%s is an invalid switch. Use '%s -help' for help\n", command[1], command[0]);
        return 3;
    }
    return 1;
}

/**
 * Something I found online when debugging meminfo
 * 
 * 10 iterations of printing out # of missing memory pages from host_statistics()
 */
void missingPages()
{
    struct vm_statistics64 stats;
    mach_port_t host = mach_host_self();
    natural_t count = HOST_VM_INFO64_COUNT;
    natural_t missing = 0;
    int debug = 0;
    kern_return_t ret;
    int mib[2];
    long ram;
    natural_t pages;
    size_t length;
    int i;

    mib[0] = CTL_HW;
    mib[1] = HW_MEMSIZE;
    length = sizeof(long);
    sysctl(mib, 2, &ram, &length, NULL, 0);
    pages = ram / getpagesize();

    for (i = 0; i < 10; i++)
    {
        if ((ret = host_statistics64(host, HOST_VM_INFO64, (host_info64_t)&stats, &count)) != KERN_SUCCESS)
        {
            printf("oops\n");
            exit(10);
        }

        /* updated for 10.9 */
        missing = pages - (stats.free_count +
                           stats.active_count +
                           stats.inactive_count +
                           stats.wire_count +
                           stats.compressor_page_count);

        if (debug)
        {
            printf(
                "%11d pages (# of pages)\n"
                "%11d free_count (# of pages free) \n"
                "%11d active_count (# of pages active) \n"
                "%11d inactive_count (# of pages inactive) \n"
                "%11d wire_count (# of pages wired down) \n"
                "%11lld zero_fill_count (# of zero fill pages) \n"
                "%11lld reactivations (# of pages reactivated) \n"
                "%11lld pageins (# of pageins) \n"
                "%11lld pageouts (# of pageouts) \n"
                "%11lld faults (# of faults) \n"
                "%11lld cow_faults (# of copy-on-writes) \n"
                "%11lld lookups (object cache lookups) \n"
                "%11lld hits (object cache hits) \n"
                "%11lld purges (# of pages purged) \n"
                "%11d purgeable_count (# of pages purgeable) \n"
                "%11d speculative_count (# of pages speculative (also counted in free_count)) \n"
                "%11lld decompressions (# of pages decompressed) \n"
                "%11lld compressions (# of pages compressed) \n"
                "%11lld swapins (# of pages swapped in (via compression segments)) \n"
                "%11lld swapouts (# of pages swapped out (via compression segments)) \n"
                "%11d compressor_page_count (# of pages used by the compressed pager to hold all the compressed data) \n"
                "%11d throttled_count (# of pages throttled) \n"
                "%11d external_page_count (# of pages that are file-backed (non-swap)) \n"
                "%11d internal_page_count (# of pages that are anonymous) \n"
                "%11lld total_uncompressed_pages_in_compressor (# of pages (uncompressed) held within the compressor.) \n",
                pages, stats.free_count, stats.active_count, stats.inactive_count,
                stats.wire_count, stats.zero_fill_count, stats.reactivations,
                stats.pageins, stats.pageouts, stats.faults, stats.cow_faults,
                stats.lookups, stats.hits, stats.purges, stats.purgeable_count,
                stats.speculative_count, stats.decompressions, stats.compressions,
                stats.swapins, stats.swapouts, stats.compressor_page_count,
                stats.throttled_count, stats.external_page_count,
                stats.internal_page_count, stats.total_uncompressed_pages_in_compressor);
        }
        printf("%i\n", missing);
        sleep(1);
    }
}

/**
 * meminfo -<switch>
 *
 * prints memory related info based on switch
 * -t total RAM
 * -u RAM used
 * -c size of L2 cache / core (in bytes)
 *
 *  * @return - int
 *  1 - Successful
 *  2 - Unsuccessful (Error)
 *  3 - Unsuccessful (Bad Switch)
 */
int memInfo(char *command[])
{
    if (command[1] == NULL)
    {
        printf("No switch given. Format: meminfo -<switch>. Use 'meminfo -help' for help.\n");
        return 3;
    }

    if (strcmp(command[1], "-t") == 0)
    {
        int mib[2] = {CTL_HW, HW_MEMSIZE};
        size_t len;
        long info;
        if (sysctl(mib, 2, &info, &len, NULL, 0) >= 0)
        {
            printf("This system has %li bytes (%ldGB) of Total RAM \n", info, info / 1073741824);
        }
        else
        {
            printf("Failed to get hardware statistics: %s\n", strerror(errno));
            return 2;
        }
    }
    else if (strcmp(command[1], "-u") == 0)
    {
        mach_msg_type_number_t count = HOST_VM_INFO_COUNT;
        vm_statistics64_data_t vmstat;
        if (KERN_SUCCESS != host_statistics64(mach_host_self(), HOST_VM_INFO, (host_info_t)&vmstat, &count))
        {
            printf("Failed to get vm_statistics: %s\n", strerror(errno));
            return 2;
        }
        else
        {
            double total = vmstat.active_count +     /* Pages in physical memory that have been recently used */
                           vmstat.inactive_count +   /* Pages in physical memory that have not been recently used */
                           vmstat.wire_count +       /* Pages that are locked in place */
                           vmstat.speculative_count; /* Pages that have not been used but are speculative */
            total *= getpagesize();
            printf("This system has %.0f bytes (%.2fGB) of RAM in use\n", total, total / 1073741824);
        }
    }
    else if (strcmp(command[1], "-c") == 0)
    {
        int mib[2] = {CTL_HW, HW_L2CACHESIZE};
        size_t len;
        int info;
        len = sizeof(info);
        if (sysctl(mib, 2, &info, &len, NULL, 0) == 0)
        {
            printf("This system has %d bytes of L2 Cache per core \n", info);
        }
        else
        {
            printf("Failed to get L2 Cache information: %s\n", strerror(errno));
            return 2;
        }
    }
    else if (
        strcmp(command[1], "-help") == 0 ||
        strcmp(command[1], "-h") == 0)
    {
        memInfoHelp();
    }
    else
    {
        printf("%s is an invalid switch. Use '%s -help' for help\n", command[1], command[0]);
        return 3;
    }
    return 1;
}

/**
 * Execute system command
 *
 * @param cmd
 *
 * @return - int
 *  1 - Successful
 *  2 - Unsuccessful (errno code)
 * */
int sysCommands(char *command[])
{
    pid_t pid;
    int status;

    char *args[3];
    args[0] = command[0];
    args[1] = command[1];
    args[2] = NULL;

    if ((pid = fork()) < 0)
    {
        printf("*** ERROR: forking child process failed\n");
        exit(1);
    }
    else if (pid == 0)
    {
        if (execvp(command[0], args) < 0)
        {
            printf("*** ERROR: exec failed: %s\n", strerror(errno));
            printf("Type 'help' for help\n");
            exit(errno);
        }
        exit(100);
    }
    else
        wait(&status);
    return WEXITSTATUS(status);
}

/**
 * Router
 * @param input
 */
int routeProgram(char *input[2])
{
    if (strcmp(input[0], "exit") == 0)
    {
        return exitShell(input);
    }
    else if (strcmp(input[0], "prompt") == 0)
    {
        return changePrompt(input);
    }
    else if (strcmp(input[0], "cpuinfo") == 0)
    {
        return cpuInfo(input);
    }
    else if (strcmp(input[0], "meminfo") == 0)
    {
        return memInfo(input);
    }
    else if (
        strcmp(input[0], "help") == 0 ||
        strcmp(input[0], "man") == 0 ||
        strcmp(input[0], "manual") == 0)
    {
        return help();
    }
    else
    {
        return sysCommands(input);
    }
}

/**
 * forks and executes command
 * @param input
 */
void executeCommand(char *input[2])
{
    pid_t pid;
    int status;

    if ((pid = fork()) < 0)
    {
        printf("*** ERROR: forking child process failed\n");
        exit(1);
    }
    else if (pid == 0)
    {
        exit(routeProgram(input));
    }
    else
        wait(&status);
    if (WIFEXITED(status))
    {
        int exitStatus = WEXITSTATUS(status);
        if (exitStatus == 4)
            exit(previous_exit_status);
        if (exitStatus == 5)
            exit(atoi(input[1]));
        if (exitStatus == 14)
            strncpy(shell_prompt, "cwushell", 30);
        if (exitStatus == 15)
            strncpy(shell_prompt, input[1], 30);
        else
        {
            previous_exit_status = exitStatus;
            return;
        }
    }
}

int main(int argc, char **argv)
{
    while (1)
    {
        char line[30];
        printf("%s> ", shell_prompt);
        fgets(line, 30, stdin);
        fflush(stdin);

        if (cleanInput(line) > 1 || verifyInput(line) != 1)
        {
            printf("'%s' is an invalid command. Use '-help' for help \n", line);
            continue;
        }

        // Limit to one switch
        char *parsedInput[2];
        parseInput(line, parsedInput);

        if (parsedInput[0] != NULL)
            executeCommand(parsedInput);
    }
}