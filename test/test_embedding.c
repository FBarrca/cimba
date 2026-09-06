/* Regression checks for embedding applications and portable trial recovery. */
#include "cimba.h"

static void empty_trial(void *arg)
{
    ++*(unsigned *)arg;
}

static void *abandon_process(struct cmb_process *process, void *context)
{
    cmb_unused(process);
    ++*(unsigned *)context;
    cimba_trial_abandon();
    return NULL;
}

static void abandoning_trial(void *arg)
{
    cmb_event_queue_initialize(0.0);
    struct cmb_dataset *data = cmb_dataset_create();
    cmb_dataset_initialize(data);
    cmb_dataset_add(data, 42.0);
    struct cmb_process *process = cmb_process_create();
    cmb_process_initialize(process, "abandon", abandon_process, arg, 0);
    cmb_process_start(process);
    /* Recovery must leave the coroutine stack before reclaiming these objects. */
    cmb_event_queue_execute();
    cmb_assert_always(false);
}

int main(void)
{
    unsigned trials[3] = {0};
    cimba_threads_use(1);
    cmb_assert_always(cimba_run(trials, 3, sizeof trials[0], empty_trial) == 0);
    cmb_assert_always(cimba_run(trials, 3, sizeof trials[0], abandoning_trial) == 3);
    cmb_assert_always(cimba_run(trials, 3, sizeof trials[0], empty_trial) == 0);
    for (unsigned i = 0; i < 3; ++i) {
        cmb_assert_always(trials[i] == 3);
    }
    return 0;
}
