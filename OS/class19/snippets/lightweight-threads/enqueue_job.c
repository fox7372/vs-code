j1 = enqueue_job(f); j2 = enqueue_job(g); j3 = enqueue_job(h);
wait_job_complete(j1); wait_job_complete(j2); wait_job_complete(j3);
