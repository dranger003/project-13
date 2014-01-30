#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>

#define UNUSED(x)           (void)(x)
#define NSEC_PER_SEC        1000000000

volatile sig_atomic_t quit = 0;

static void signal_handler(int signal)
{
    UNUSED(signal);
    quit = 1;
}

static void code()
{
//    //                         1001001001
//    struct timespec wait = { 0, 075000000 };
//    nanosleep(&wait, NULL);

    usleep(40000);
}

static void run()
{
    system("setterm -cursor off");

    double target_rate = 23.976216;
    long ts = 0;
    struct timespec time_slice = { 0 }, time_slice_l = { 0 }, time_slice_t = { 0, 1 / target_rate * NSEC_PER_SEC }, time_sync = { 0 };

    printf("%.6f\n", time_slice_t.tv_sec + (double)time_slice_t.tv_nsec / NSEC_PER_SEC);

    int frame_count = 0;
    struct timespec a, b;
    clock_gettime(CLOCK_MONOTONIC, &a);

    while (!quit) {
        clock_gettime(CLOCK_MONOTONIC, &time_slice);

        if (frame_count > 0) {
            fprintf(stdout,
                    "F: %d, E: %.6f, R: %.6f%20s\r",
                    frame_count,
                    time_sync.tv_sec - a.tv_sec + (double)(time_sync.tv_nsec - a.tv_nsec) / NSEC_PER_SEC,
                    1.0 / (time_sync.tv_sec - time_slice_l.tv_sec + (double)(time_sync.tv_nsec - time_slice_l.tv_nsec) / NSEC_PER_SEC),
                    " ");
            fflush(stdout);
        }

        code();

        ++frame_count;

        // Last time slice
        time_slice_l = time_slice;

        ts = (time_slice.tv_sec + time_slice_t.tv_sec) * NSEC_PER_SEC + time_slice.tv_nsec + time_slice_t.tv_nsec;
        while (time_sync.tv_sec * NSEC_PER_SEC + time_sync.tv_nsec < ts)
            clock_gettime(CLOCK_MONOTONIC, &time_sync);
    }

    clock_gettime(CLOCK_MONOTONIC, &b);
    printf("\nF: %d, E: %.6f, R: %.6f",
           frame_count,
           (b.tv_sec - a.tv_sec + (double)(b.tv_nsec - a.tv_nsec) / NSEC_PER_SEC),
           frame_count / (b.tv_sec - a.tv_sec + (double)(b.tv_nsec - a.tv_nsec) / NSEC_PER_SEC));

    printf("\n");

    system("setterm -cursor on");
}

int main(void)
{
    struct sigaction sa;
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    assert(sigaction(SIGINT, &sa, NULL) != -1);

    run();

    return 0;
}
