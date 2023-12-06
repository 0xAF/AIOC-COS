/*
 * Copyright (c) 2023 Stanislav Lechev [0xAF] (af@0xaf.org)
 * This code is licensed under MIT license
 * (see LICENSE or https://af.mit-license.org/ for details)
 */

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>

void usage(char* command)
{
    printf("Usage:\n"
           "  %s [-h] [-q] [-v]\n"
           "  \t<-H /dev/hidraw?>\n"
           "  \t<-R cos_on_script.sh>\n"
           "  \t<-S cos_off_script.sh> [-t 2000]\n\n"
           "  -h\t\tPrint this help.\n"
           "  -q\t\tBe quiet\n"
           "  -v\t\tBe verbose\n"
           "  -H file\thiddev device file\n"
           "  -R file\tscript to run when radio starts Receiving\n"
           "  -S file\tscript to run when radio become Silent\n"
           "  -t msec\ttimeout in milliseconds after the last audio\n"
           "         \tand before calling the Silent script (default 1s)\n"
           "\n"
           "This tool will watch the All-In-One-Cable (AIOC) hidraw device\n"
           "for events. If the radio starts receiving, it will call the\n"
           "cos_on_script. Respectively, when the radio is not receiving\n"
           "anymore it will call cos_off_script.\n"
           "\n"
           "At the time of writing this tool, the AIOC stable firmware is not\n"
           "sending events to the hidraw interface.\n"
           "But you can compile the 'autoptt' branch of AIOC or use the .bin\n"
           "from this repo, to enable this function.\n"
           "The 'autoptt' branch uses 10msec timeout on audio samples and then\n"
           "sends Silent event, even if the radio is still receiving.\n"
           "Because of this, we need our timeout in this tool, before calling\n"
           "the Silent script. Hopefully this will change in the final release\n"
           "of this feature in AIOC.\n"
           "autoptt branch of AIOC: https://github.com/skuep/AIOC/tree/autoptt\n"
           "\n"
           "I'm using this tool to send the audio from my radio to my Mumble\n"
           "server via mumble client. The other direction, when the mumble client\n"
           "receives audio and plays it on AIOC audio interface, will automatically\n"
           "enable the PTT on the radio with the 'autoptt' firmware.\n",
        command);
}

int call_script(const char* script, unsigned int verbose)
{
    FILE* fp = popen(script, "r");
    if (fp == NULL) {
        perror("Error opening pipe");
        return 1;
    }

    if (verbose) {
        char buffer[128];
        while (fgets(buffer, sizeof(buffer), fp) != NULL) {
            printf("%s", buffer);
        }
    }

    int status = pclose(fp);
    if (status == -1) {
        perror("Error closing pipe");
        return 1;
    }

    return 0;
}

int main(int argc, char* argv[])
{
    int opt;
    extern char* optarg;
    int quiet = 0;
    int verbose = 0;
    const char* hiddev = NULL;
    const char* cos_on = NULL;
    const char* cos_off = NULL;
    unsigned int timeout = 1000;

    while ((opt = getopt(argc, argv, "hqH:R:S:t:v")) != -1) {
        switch (opt) {
        case 'v':
            quiet = 0;
            verbose = 1;
            break;
        case 'q':
            quiet = 1;
            verbose = 0;
            break;
        case 'H':
            hiddev = optarg;
            break;
        case 'R':
            cos_on = optarg;
            break;
        case 'S':
            cos_off = optarg;
            break;
        case 't':
            int t = atoi(optarg);
            if (t < 50) {
                printf("timeout should be more than 50msec.");
                exit(EXIT_FAILURE);
            }
            timeout = t;
            break;
        case 'h':
        default:
            if (!quiet)
                usage(argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    if (!quiet) {
        printf("AIOC_COS - Listen for COS (reception) events on AIOC (with the correct firmware).\n"
               "All-In-One-Cable: https://github.com/skuep/AIOC\n\n");
        if (verbose) {
            printf("Timeout: %dmsec\n", timeout);
        }
    }

    if (hiddev == NULL || cos_on == NULL || cos_off == NULL) {
        if (!quiet)
            usage(argv[0]);
        exit(EXIT_FAILURE);
    }
    if (access(cos_on, X_OK) == -1) {
        printf("Receive script '%s' does not exist or is not executable.\n", cos_on);
        exit(EXIT_FAILURE);
    }
    if (access(cos_off, X_OK) == -1) {
        printf("Silent script '%s' does not exist or is not executable.\n", cos_off);
        exit(EXIT_FAILURE);
    }

    int fd = open(hiddev, O_RDONLY);
    if (fd == -1) {
        perror("Cannot open hiddev");
        exit(errno);
    }

    char buffer[32];
    size_t len;
    while (len = read(fd, buffer, sizeof(buffer)) > 0) {
        if (verbose) {
            printf("read=%02d", len);
            for (size_t i = 0; i < len; i++) {
                printf(" %d=[%02X]", i, buffer[i]);
            }
            printf("\n");
        }
        call_script(cos_on, verbose);

        // use select for timeout after last read
        fd_set readSet;
        FD_ZERO(&readSet);
        FD_SET(fd, &readSet);
        struct timeval tout;
        tout.tv_sec = timeout / 1000;
        tout.tv_usec = (timeout % 1000) * 1000;

        int res = select(fd + 1, &readSet, NULL, NULL, &tout);
        if (res == -1) {
            perror("Error in select");
            close(fd);
            exit(EXIT_FAILURE);
        } else if (res == 0) {
            if (verbose)
                printf("Timeout reached.\n");
            call_script(cos_off, verbose);
        }
    }

    close(fd);

    if (len == -1) {
        perror("Error reading from hidraw");
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}
