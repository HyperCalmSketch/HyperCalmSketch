/**
 * CalmSS algorithm, in phase 2
 * report top-k periodic batches
 */

package hypercalm;

class CalmSSParams {
    public static final int array_len = 20;
    public static final int queue_len = 10;
    public static final int promotion_threshold = 20;
    public static final int heap_size = 500;
    public static final long interval_equal = 10;
    public static final int report_threshold = 20;
}

class Bin implements java.io.Serializable {
    int id;
    long interval;
    int counter;
}

class Batch implements java.io.Serializable {
    int id;
    long timestamp;

    public Batch(int _id, long _time) {
        id = _id;
        timestamp = _time;
    }
    public Batch() {}
}

class CircularArray implements java.io.Serializable {
    public Batch[] array = new Batch[20];

    public CircularArray() {
	for (int i = 0; i < CalmSSParams.array_len; ++i) {
	    array[i] = new Batch();
	}
    }

    public long insert(int id, long timestamp) { // return interval
        long ret = -1;
        Batch b = new Batch(id, timestamp);
        for (int i = 0; i < CalmSSParams.array_len; ++i) {
            if (array[i].id == b.id) {
                ret = b.timestamp - array[i].timestamp;
                for (int j = i; j > 0; --j)
                    array[j] = array[j - 1];
                array[0] = b;
                break;
            }
        }
        if (ret == -1) {
            for (int j = CalmSSParams.array_len - 1; j > 0; --j)
                array[j] = array[j - 1];
            array[0] = b;
        }
        return ret;
    }
}

class LRU_queue implements java.io.Serializable {
    public Bin[] queue = new Bin[100];

    public LRU_queue() {
	for (int i = 0; i < CalmSSParams.queue_len; ++i) {
	    queue[i] = new Bin();
	}
    }

    private boolean same_interval(long interval0, long interval1) {
        return (interval0 / CalmSSParams.interval_equal) == (interval1 / CalmSSParams.interval_equal);
    }

    public boolean insert(int id, long interval) {
        int pos = -1;
        for (int i = 0; i < CalmSSParams.queue_len; ++i) {
            if (queue[i].id == id && same_interval(queue[i].interval, interval)) {
                ++queue[i].counter;
                if (queue[i].counter >= CalmSSParams.promotion_threshold) {
                    // remove id from LRU queue
                    for (int j = i + 1; j < CalmSSParams.queue_len; ++j)
                        queue[j - 1] = queue[j];
                    queue[CalmSSParams.queue_len - 1].counter = 0;
                    return true;
                }
                pos = i;
                break;
            }
            else if (queue[i].counter == 0) {
                queue[i].id = id;
                queue[i].interval = interval;
                queue[i].counter = 1;
                pos = i;
                break;
            }
        }
        if (pos != -1 && pos != 0) {
            Bin tmp_bin = queue[pos];
            for (int i = pos; i > 0; --i) {
                queue[i] = queue[i - 1];
            }
            queue[0] = tmp_bin;
        }
        else if (pos == -1) {
            for (int i = CalmSSParams.queue_len - 1; i > 0; --i)
                queue[i] = queue[i - 1];
            queue[0].id = id;
            queue[0].interval = interval;
            queue[0].counter = 1;
        }
        return false;
    }
}

class Min_Heap implements java.io.Serializable {
    public Bin[] heap = new Bin[500];

    public Min_Heap() {
	for (int i = 0; i < CalmSSParams.heap_size; ++i) {
	    heap[i] = new Bin();
	}
    }

    public boolean insert(int id, long interval) {
        int min_cnt = Integer.MAX_VALUE, min_pos = -1;
        for (int i = 0; i < CalmSSParams.heap_size; ++i) {
            if (heap[i].id == id && heap[i].interval == interval) {
                heap[i].counter += CalmSSParams.promotion_threshold;
                return heap[i].counter >= CalmSSParams.report_threshold;
            }
            else if (heap[i].counter == 0) {
                heap[i].id = id;
                heap[i].interval = interval;
                heap[i].counter = CalmSSParams.promotion_threshold;
                return heap[i].counter >= CalmSSParams.report_threshold;
            }
            else if (min_cnt > heap[i].counter) {
                min_pos = i;
                min_cnt = heap[i].counter;
            }
        }
        // update smallest node
        heap[min_pos].id = id;
        heap[min_pos].interval = interval;
        heap[min_pos].counter += CalmSSParams.promotion_threshold;

        return heap[min_pos].counter >= CalmSSParams.report_threshold;
    }
}

public class CalmSpaceSaving implements java.io.Serializable {
    public CircularArray circualr_array = new CircularArray();
    public LRU_queue lru_queue = new LRU_queue();
    public Min_Heap min_heap = new Min_Heap();

    public PeriodicBatch insert(int id, long timestamp) {
        long result_interval = -1;
        long interval = circualr_array.insert(id, timestamp);
        if (interval != -1) { 
            // find item in the circularArray and get interval
            if (lru_queue.insert(id, interval)) {
                if (min_heap.insert(id, interval)) {
                    result_interval = interval;
		}
            }
        }
        PeriodicBatch batch = new PeriodicBatch(id, result_interval);
	return batch;
    }
}
