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
    usleep(40000); // 40ms
}

static void run()
{
    system("setterm -cursor off");

    long double target_rate = 24000 / 1001.0;
    long long t_ts;
    struct timespec time_slice = { 0 }, time_slice_l = { 0 }, time_slice_t = { 0, 1 / target_rate * NSEC_PER_SEC }, time_sync = { 0 };

    printf("%.6Lf\n", time_slice_t.tv_sec + (long double)time_slice_t.tv_nsec / NSEC_PER_SEC);

    int frame_count = 0;
    struct timespec time_avg_b, time_avg_e;
    clock_gettime(CLOCK_MONOTONIC, &time_avg_b);

    while (!quit) {
        clock_gettime(CLOCK_MONOTONIC, &time_slice);

        // Stats aren't valid on first loop
        if (frame_count > 0) {
            fprintf(stdout,
                    "F: %d, E: %.6Lf, R: %.6Lf\r",
                    frame_count,
                    time_sync.tv_sec - time_avg_b.tv_sec + (long double)(time_sync.tv_nsec - time_avg_b.tv_nsec) / NSEC_PER_SEC,
                    1.0 / (time_sync.tv_sec - time_slice_l.tv_sec + (long double)(time_sync.tv_nsec - time_slice_l.tv_nsec) / NSEC_PER_SEC));
            fflush(stdout);
        }

        code();

        ++frame_count;

        // Last time slice
        time_slice_l = time_slice;

        t_ts = (time_slice.tv_sec + time_slice_t.tv_sec) * (long long)NSEC_PER_SEC + time_slice.tv_nsec + time_slice_t.tv_nsec;
        while (time_sync.tv_sec * (long long)NSEC_PER_SEC + time_sync.tv_nsec < t_ts)
            clock_gettime(CLOCK_MONOTONIC, &time_sync);
    }

    clock_gettime(CLOCK_MONOTONIC, &time_avg_e);
    printf("\nF: %d, E: %.6Lf, R: %.6Lf\n",
           frame_count,
           (time_avg_e.tv_sec - time_avg_b.tv_sec + (long double)(time_avg_e.tv_nsec - time_avg_b.tv_nsec) / NSEC_PER_SEC),
           frame_count / (time_avg_e.tv_sec - time_avg_b.tv_sec + (long double)(time_avg_e.tv_nsec - time_avg_b.tv_nsec) / NSEC_PER_SEC));

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
